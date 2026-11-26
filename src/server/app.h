#pragma once
#include <string>
#include <vector>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "courses.h"
#include "types.h"
using namespace std;
using json = nlohmann::json;

// Everything the server loads once at startup. Read-only while serving.
struct AppData {
    Courses                  catalog;
    vector<GenEdRequirement> genEd;
    vector<DistributionRule> distributionRules;
    vector<string>           dataErrors;   // every problem found while loading (see doc/data-contract.md)
};

// Loads the data files from `dataDir` (the folder that holds cs_courses.json,
// gen_ed_requirements.json and course_details.json) in the order the contract defines.
AppData loadAppData(const string& dataDir);

// Response bodies, kept as plain functions so tests can check them without HTTP.
json healthJson(const AppData& app);
json errorJson(const string& message);

// A catalog course in the API's shape (doc/data-contract.md, Course). Unknown credits -> null.
json courseJson(const Course& course);

// Requirement group of a course for search filters. Until the Advisor exists this is derived
// from the course alone: CS -> "cs", MATH/PHYSIC -> "math_science", anything else -> "gen_ed".
// "elective" depends on the student's plan and is never returned here.
string catalogGroup(const Course& course);

// Query parameters of GET /api/courses after validation.
struct SearchParams {
    string q;
    string group;     // "" = any
    string subject;   // "" = any, compared case-insensitively
    int    limit = 20;
};

// Reads and validates the parameters. Returns "" when valid, otherwise the error message
// for a 400 response. `term` and `open` are accepted and ignored (no offering data yet).
string parseSearchParams(const httplib::Request& req, SearchParams& out);

// { "total": N, "results": [ { "course": Course, "satisfies": null }, ... ] }
json searchJson(const AppData& app, const SearchParams& params);

// Attaches every /api route to `server`. `app` must outlive the server.
void registerRoutes(httplib::Server& server, const AppData& app);
