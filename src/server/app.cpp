#include "app.h"
#include <algorithm>
#include <cctype>
#include <set>
#include "catalog_loader.h"
#include "course_code.h"

using namespace std;

AppData loadAppData(const string& dataDir) {
    AppData app;
    string cs      = dataDir + "/cs_courses.json";
    string genEd   = dataDir + "/gen_ed_requirements.json";
    string details = dataDir + "/course_details.json";

    loadCourses(cs, genEd, app.catalog, app.dataErrors);
    loadCourseDetails(details, app.catalog, app.dataErrors);
    checkCourseReferences(app.catalog, app.dataErrors);
    app.genEd             = loadGenEd(genEd, app.dataErrors);
    app.distributionRules = loadDistributionRules(genEd, app.dataErrors);
    checkLinks(app.catalog, app.genEd, app.dataErrors);
    return app;
}

json healthJson(const AppData& app) {
    return {
        { "status",     "ok" },
        { "courses",    app.catalog.size() },
        { "dataErrors", app.dataErrors },
    };
}

json errorJson(const string& message) {
    return { { "error", message } };
}

static string categoryName(CourseCategory category) {
    switch (category) {
        case CourseCategory::CS:           return "CS";
        case CourseCategory::GEN_ED:       return "GEN_ED";
        case CourseCategory::DISTRIBUTION: return "DISTRIBUTION";
    }
    return "GEN_ED";
}

json courseJson(const Course& course) {
    return {
        { "code",    course.code },
        { "title",   course.name },
        { "credits", course.credits > 0 ? json(course.credits) : json(nullptr) },
        { "kind",    categoryName(course.category) },
        { "area",    course.distArea },
        { "prereqs", course.prereqs },
        { "coreqs",  course.coreqs },
        { "note",    course.note },
    };
}

string catalogGroup(const Course& course) {
    if (course.category == CourseCategory::CS) return "cs";
    string subject = subjectOf(course.code);
    if (subject == "MATH" || subject == "PHYSIC") return "math_science";
    return "gen_ed";
}

static string upper(string s) {
    for (char& c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

string parseSearchParams(const httplib::Request& req, SearchParams& out) {
    out = SearchParams{};
    out.q       = req.get_param_value("q");
    out.group   = req.get_param_value("group");
    out.subject = upper(req.get_param_value("subject"));

    static const set<string> kGroups = { "cs", "math_science", "gen_ed", "elective" };
    if (!out.group.empty() && !kGroups.count(out.group))
        return "group must be one of cs, math_science, gen_ed, elective";

    if (req.has_param("limit")) {
        const string text = req.get_param_value("limit");
        size_t used = 0;
        int limit = 0;
        try { limit = stoi(text, &used); } catch (const exception&) { used = 0; }
        if (used == 0 || used != text.size() || limit < 1)
            return "limit must be a whole number of at least 1";
        out.limit = min(limit, 100);
    }
    return "";
}

json searchJson(const AppData& app, const SearchParams& params) {
    json results = json::array();
    int  total   = 0;
    for (const Course& course : app.catalog.search(params.q)) {
        if (!params.subject.empty() && subjectOf(course.code) != params.subject) continue;
        if (!params.group.empty()   && catalogGroup(course) != params.group)     continue;
        ++total;
        if (static_cast<int>(results.size()) < params.limit)
            results.push_back({ { "course", courseJson(course) }, { "satisfies", nullptr } });
    }
    return { { "total", total }, { "results", results } };
}

static void sendJson(httplib::Response& res, const json& body, int status = 200) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

void registerRoutes(httplib::Server& server, const AppData& app) {
    server.Get("/api/health", [&app](const httplib::Request&, httplib::Response& res) {
        sendJson(res, healthJson(app));
    });

    server.Get("/api/courses", [&app](const httplib::Request& req, httplib::Response& res) {
        SearchParams params;
        string problem = parseSearchParams(req, params);
        if (!problem.empty()) return sendJson(res, errorJson(problem), 400);
        sendJson(res, searchJson(app, params));
    });

    // The code may be written any way normalizeCode accepts: /api/courses/cs%20310 works.
    server.Get(R"(/api/courses/([^/]+))", [&app](const httplib::Request& req, httplib::Response& res) {
        const string code   = req.matches[1];
        const Course* course = app.catalog.find(code);
        if (!course) return sendJson(res, errorJson("unknown course " + code), 404);
        sendJson(res, { { "course", courseJson(*course) }, { "satisfies", nullptr } });
    });

    // Any /api path without a route gets the contract's JSON error instead of an HTML page.
    // Routes that answer 404 themselves ("unknown course") already have a body: keep it.
    server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        if (req.path.rfind("/api/", 0) == 0 && res.status == 404 && res.body.empty())
            sendJson(res, errorJson("no such endpoint: " + req.method + " " + req.path), 404);
    });
}
