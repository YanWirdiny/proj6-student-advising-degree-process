#include <gtest/gtest.h>
#include <thread>
#include "app.h"

using namespace std;

// Loading the real data once is enough for every test here.
static const AppData& realApp() {
    static const AppData app = loadAppData(PROJECT_DIR);
    return app;
}

// ---- loading ----

TEST(AppDataTest, LoadsAllRealFiles) {
    const AppData& app = realApp();
    EXPECT_EQ(app.catalog.size(), 524u);
    EXPECT_FALSE(app.genEd.empty());
    EXPECT_EQ(app.distributionRules.size(), 16u);
    // Only the known gap: BC290 is named by a requirement but not in the catalog.
    ASSERT_EQ(app.dataErrors.size(), 1u);
    EXPECT_NE(app.dataErrors[0].find("BC290"), string::npos);
}

TEST(AppDataTest, MissingFolderReportsErrors) {
    AppData app = loadAppData("/no/such/folder");
    EXPECT_EQ(app.catalog.size(), 0u);
    EXPECT_FALSE(app.dataErrors.empty());
}

// ---- response bodies ----

TEST(HealthJsonTest, MatchesContract) {
    json body = healthJson(realApp());
    EXPECT_EQ(body["status"], "ok");
    EXPECT_EQ(body["courses"], 524);
    ASSERT_TRUE(body["dataErrors"].is_array());
    EXPECT_EQ(body["dataErrors"].size(), 1u);
}

TEST(ErrorJsonTest, MatchesContract) {
    EXPECT_EQ(errorJson("unknown course CS999"), json({ { "error", "unknown course CS999" } }));
}

// ---- over real HTTP ----

class ServerTest : public testing::Test {
protected:
    void SetUp() override {
        registerRoutes(server_, realApp());
        port_ = server_.bind_to_any_port("127.0.0.1");
        ASSERT_GT(port_, 0);
        thread_ = thread([this] { server_.listen_after_bind(); });
        server_.wait_until_ready();
    }
    void TearDown() override {
        server_.stop();
        if (thread_.joinable()) thread_.join();
    }
    httplib::Client client() { return httplib::Client("127.0.0.1", port_); }

    httplib::Server server_;
    int             port_ = 0;
    thread          thread_;
};

TEST_F(ServerTest, HealthOverHttp) {
    auto res = client().Get("/api/health");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->get_header_value("Content-Type"), "application/json");
    EXPECT_EQ(json::parse(res->body)["courses"], 524);
}

TEST_F(ServerTest, UnknownApiRouteIsJson404) {
    auto res = client().Get("/api/nope");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 404);
    EXPECT_EQ(json::parse(res->body)["error"], "no such endpoint: GET /api/nope");
}

// ---- course JSON ----

TEST(CourseJsonTest, MatchesContract) {
    const Course* cs310 = realApp().catalog.find("CS310");
    ASSERT_NE(cs310, nullptr);
    json body = courseJson(*cs310);
    EXPECT_EQ(body["code"], "CS310");
    EXPECT_EQ(body["title"], "Advanced Data Structures and Algorithms");
    EXPECT_EQ(body["credits"], 3);
    EXPECT_EQ(body["kind"], "CS");
    EXPECT_EQ(body["prereqs"], json({ "CS210", "CS220", "CS240" }));
    EXPECT_TRUE(body["coreqs"].is_array());
    EXPECT_TRUE(body["note"].is_string());
}

TEST(CourseJsonTest, UnknownCreditsIsNull) {
    // PHIL100 only comes from the scraped gen-ed file: no credits known.
    json body = courseJson(*realApp().catalog.find("PHIL100"));
    EXPECT_TRUE(body["credits"].is_null());
    EXPECT_EQ(body["kind"], "DISTRIBUTION");
    EXPECT_EQ(body["area"], "HU");
}

TEST(CatalogGroupTest, DerivedFromCourse) {
    const Courses& c = realApp().catalog;
    EXPECT_EQ(catalogGroup(*c.find("CS110")), "cs");
    EXPECT_EQ(catalogGroup(*c.find("MATH140")), "math_science");
    EXPECT_EQ(catalogGroup(*c.find("PHYSIC113")), "math_science");
    EXPECT_EQ(catalogGroup(*c.find("ENGL101")), "gen_ed");
    EXPECT_EQ(catalogGroup(*c.find("PHIL100")), "gen_ed");
}

// ---- search parameters ----

static httplib::Request requestWith(initializer_list<pair<string, string>> params) {
    httplib::Request req;
    for (const auto& [k, v] : params) req.params.emplace(k, v);
    return req;
}

TEST(SearchParamsTest, Defaults) {
    SearchParams p;
    EXPECT_EQ(parseSearchParams(requestWith({}), p), "");
    EXPECT_EQ(p.q, "");
    EXPECT_EQ(p.limit, 20);
}

TEST(SearchParamsTest, ReadsAndNormalizes) {
    SearchParams p;
    EXPECT_EQ(parseSearchParams(requestWith({ { "q", "cs 4" }, { "subject", "math" }, { "group", "cs" },
                                              { "limit", "5" }, { "term", "2027SP" }, { "open", "1" } }), p), "");
    EXPECT_EQ(p.q, "cs 4");
    EXPECT_EQ(p.subject, "MATH");
    EXPECT_EQ(p.group, "cs");
    EXPECT_EQ(p.limit, 5);
}

TEST(SearchParamsTest, LimitCappedAt100) {
    SearchParams p;
    parseSearchParams(requestWith({ { "limit", "500" } }), p);
    EXPECT_EQ(p.limit, 100);
}

TEST(SearchParamsTest, RejectsBadValues) {
    SearchParams p;
    EXPECT_NE(parseSearchParams(requestWith({ { "limit", "abc" } }), p), "");
    EXPECT_NE(parseSearchParams(requestWith({ { "limit", "0" } }), p), "");
    EXPECT_NE(parseSearchParams(requestWith({ { "limit", "5x" } }), p), "");
    EXPECT_NE(parseSearchParams(requestWith({ { "group", "major" } }), p), "");
}

// ---- search results ----

static vector<string> resultCodes(const json& body) {
    vector<string> out;
    for (const json& r : body["results"]) out.push_back(r["course"]["code"]);
    return out;
}

TEST(SearchJsonTest, CodePrefixInRelevanceOrder) {
    SearchParams p;
    p.q = "cs 44";
    json body = searchJson(realApp(), p);
    EXPECT_EQ(resultCodes(body), (vector<string>{ "CS442", "CS443", "CS444", "CS446", "CS449" }));
    EXPECT_EQ(body["total"], 5);
    EXPECT_TRUE(body["results"][0]["satisfies"].is_null());
}

TEST(SearchJsonTest, LimitKeepsTotal) {
    SearchParams p;
    p.q = "cs";
    p.limit = 3;
    json body = searchJson(realApp(), p);
    EXPECT_EQ(body["results"].size(), 3u);
    EXPECT_EQ(body["total"], 46);   // 43 CS-major courses + CS341L + CS105, CS271L (distribution courses)
}

TEST(SearchJsonTest, FiltersBySubjectAndGroup) {
    SearchParams p;
    p.q = "calculus";
    p.subject = "MATH";
    // "Survey of Calculus" matches; "Precalculus" does not (word starts only).
    EXPECT_EQ(resultCodes(searchJson(realApp(), p)), (vector<string>{ "MATH135", "MATH140", "MATH141" }));

    SearchParams g;
    g.group = "math_science";
    g.limit = 100;
    for (const string& code : resultCodes(searchJson(realApp(), g)))
        EXPECT_TRUE(code.rfind("MATH", 0) == 0 || code.rfind("PHYSIC", 0) == 0) << code;
}

TEST(SearchJsonTest, ElectiveGroupEmptyUntilAdvisor) {
    SearchParams p;
    p.group = "elective";
    EXPECT_EQ(searchJson(realApp(), p)["total"], 0);
}

// ---- over real HTTP ----

TEST_F(ServerTest, SearchOverHttp) {
    auto res = client().Get("/api/courses?q=cs%2044&limit=2");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    json body = json::parse(res->body);
    EXPECT_EQ(body["total"], 5);
    EXPECT_EQ(body["results"].size(), 2u);
}

TEST_F(ServerTest, SearchBadParamIs400) {
    auto res = client().Get("/api/courses?limit=nope");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);
    EXPECT_TRUE(json::parse(res->body).contains("error"));
}

TEST_F(ServerTest, CourseByCodeAnySpelling) {
    auto res = client().Get("/api/courses/cs%20310");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(json::parse(res->body)["course"]["code"], "CS310");
}

TEST_F(ServerTest, UnknownCourseIs404) {
    auto res = client().Get("/api/courses/CS999");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 404);
    EXPECT_EQ(json::parse(res->body)["error"], "unknown course CS999");
}
