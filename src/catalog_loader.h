#pragma once
#include <string>
#include <vector>
#include "courses.h"
#include "types.h"
using namespace std;

// Turns the JSON files the scraper wrote into backend objects. The only backend code that
// knows about JSON. Every function appends a readable message to `errors` for each problem
// (missing file, malformed entry, conflicting course) and keeps going with the rest.

// cs_courses.json + gen_ed_requirements.json -> one catalog.
//   cs_courses               -> category CS
//   gen_ed_courses           -> category GEN_ED
//   distribution_courses     -> category DISTRIBUTION with its area
//   distribution_areas[].courses (gen-ed file) -> DISTRIBUTION with area and title
// A course listed in several places is merged (see Courses::merge).
// Returns false when either file cannot be read or parsed.
bool loadCourses(const string& csCoursesPath, const string& genEdPath,
                 Courses& out, vector<string>& errors);

// gen_ed_requirements.json "gen_ed" -> requirements, course codes normalized.
vector<GenEdRequirement> loadGenEd(const string& genEdPath, vector<string>& errors);

// gen_ed_requirements.json "distribution_rules" -> how many courses each program needs.
vector<DistributionRule> loadDistributionRules(const string& genEdPath, vector<string>& errors);

// Reports every course a requirement names that the catalog does not contain.
// Returns true when all links resolve.
bool checkLinks(const Courses& catalog, const vector<GenEdRequirement>& genEd,
                vector<string>& errors);
