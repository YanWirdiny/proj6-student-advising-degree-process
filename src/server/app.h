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

// Attaches every /api route to `server`. `app` must outlive the server.
void registerRoutes(httplib::Server& server, const AppData& app);
