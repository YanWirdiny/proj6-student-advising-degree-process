#include "app.h"
#include "catalog_loader.h"

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

static void sendJson(httplib::Response& res, const json& body, int status = 200) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

void registerRoutes(httplib::Server& server, const AppData& app) {
    server.Get("/api/health", [&app](const httplib::Request&, httplib::Response& res) {
        sendJson(res, healthJson(app));
    });

    // Any /api path without a route gets the contract's JSON error instead of an HTML page.
    server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        if (req.path.rfind("/api/", 0) == 0 && res.status == 404)
            sendJson(res, errorJson("no such endpoint: " + req.method + " " + req.path), 404);
    });
}
