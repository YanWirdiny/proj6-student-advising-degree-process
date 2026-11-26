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
