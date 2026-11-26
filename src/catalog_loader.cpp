#include "catalog_loader.h"
#include "course_code.h"
#include <fstream>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Reads and parses one file. On failure records why and returns false.
static bool readJson(const string& path, json& out, vector<string>& errors) {
    ifstream in(path);
    if (!in) {
        errors.push_back("cannot open " + path);
        return false;
    }
    try {
        in >> out;
    } catch (const json::exception& e) {
        errors.push_back("cannot parse " + path + ": " + e.what());
        return false;
    }
    return true;
}

// The array under `key`, or an empty array when the key is missing or not an array.
static json arrayAt(const json& doc, const string& key, const string& path, vector<string>& errors) {
    if (!doc.contains(key)) {
        errors.push_back(path + ": missing \"" + key + "\"");
        return json::array();
    }
    if (!doc[key].is_array()) {
        errors.push_back(path + ": \"" + key + "\" is not a list");
        return json::array();
    }
    return doc[key];
}

static string stringField(const json& item, const string& key) {
    return item.contains(key) && item[key].is_string() ? item[key].get<string>() : "";
}

static vector<string> stringList(const json& item, const string& key) {
    vector<string> out;
    if (item.contains(key) && item[key].is_array())
        for (const json& s : item[key])
            if (s.is_string()) out.push_back(s.get<string>());
    return out;
}

static void mergeInto(Courses& out, const Course& course, const string& where, vector<string>& errors) {
    if (course.code.empty()) {
        errors.push_back(where + ": entry without a course code");
        return;
    }
    if (!out.merge(course))
        errors.push_back(where + ": bad or conflicting course \"" + course.code + "\"");
}

bool loadCourses(const string& csCoursesPath, const string& genEdPath,
                 Courses& out, vector<string>& errors) {
    json cs, ge;
    bool okCs = readJson(csCoursesPath, cs, errors);
    bool okGe = readJson(genEdPath, ge, errors);
    if (!okCs || !okGe) return false;

    // Plain code lists.
    const pair<const char*, CourseCategory> kCodeLists[] = {
        { "cs_courses",     CourseCategory::CS },
        { "gen_ed_courses", CourseCategory::GEN_ED },
    };
    for (const auto& [key, category] : kCodeLists) {
        for (const json& entry : arrayAt(cs, key, csCoursesPath, errors)) {
            Course c;
            c.code     = entry.is_string() ? entry.get<string>() : "";
            c.category = category;
            mergeInto(out, c, csCoursesPath + " " + key, errors);
        }
    }

    // { "code": ..., "area": ... }
    for (const json& entry : arrayAt(cs, "distribution_courses", csCoursesPath, errors)) {
        Course c;
        c.code     = stringField(entry, "code");
        c.category = CourseCategory::DISTRIBUTION;
        c.distArea = stringField(entry, "area");
        mergeInto(out, c, csCoursesPath + " distribution_courses", errors);
    }

    // { "name": ..., "courses": [ { "code", "title", "area" } ] }
    for (const json& area : arrayAt(ge, "distribution_areas", genEdPath, errors)) {
        if (!area.contains("courses") || !area["courses"].is_array()) continue;
        for (const json& entry : area["courses"]) {
            Course c;
            c.code     = stringField(entry, "code");
            c.name     = stringField(entry, "title");
            c.category = CourseCategory::DISTRIBUTION;
            c.distArea = stringField(entry, "area");
            mergeInto(out, c, genEdPath + " distribution_areas", errors);
        }
    }
    return true;
}

// "CS" / "GEN_ED" / "DISTRIBUTION" as written in course_details.json.
static bool parseCategory(const string& text, CourseCategory& out) {
    if (text == "CS")           { out = CourseCategory::CS;           return true; }
    if (text == "GEN_ED")       { out = CourseCategory::GEN_ED;       return true; }
    if (text == "DISTRIBUTION") { out = CourseCategory::DISTRIBUTION; return true; }
    return false;
}

bool loadCourseDetails(const string& detailsPath, Courses& out, vector<string>& errors) {
    json doc;
    if (!readJson(detailsPath, doc, errors)) return false;

    for (const json& entry : arrayAt(doc, "courses", detailsPath, errors)) {
        Course c;
        c.code     = stringField(entry, "code");
        c.name     = stringField(entry, "title");
        c.distArea = stringField(entry, "area");
        c.note     = stringField(entry, "note");
        c.prereqs  = stringList(entry, "prereqs");
        c.coreqs   = stringList(entry, "coreqs");

        string where = detailsPath + " " + (c.code.empty() ? string("courses") : c.code);
        if (!parseCategory(stringField(entry, "category"), c.category)) {
            errors.push_back(where + ": unknown category \"" + stringField(entry, "category") + "\"");
            continue;
        }
        if (entry.contains("credits")) {
            if (!entry["credits"].is_number() || entry["credits"].get<double>() < 0) {
                errors.push_back(where + ": credits must be a number >= 0");
                continue;
            }
            c.credits = entry["credits"].get<double>();
        }
        mergeInto(out, c, detailsPath, errors);
    }
    return true;
}

bool checkCourseReferences(const Courses& catalog, vector<string>& errors) {
    bool ok = true;
    for (const auto& [code, course] : catalog.all()) {
        for (const string& p : course.prereqs)
            if (!catalog.contains(p)) {
                errors.push_back(code + ": prerequisite " + p + " is not in the catalog");
                ok = false;
            }
        for (const string& p : course.coreqs)
            if (!catalog.contains(p)) {
                errors.push_back(code + ": co-requisite " + p + " is not in the catalog");
                ok = false;
            }
    }

    // Cycle check: depth-first search over prerequisites. A course reached again while it
    // is still on the current path ("visiting") closes a cycle.
    enum class Mark { NEW, VISITING, DONE };
    map<string, Mark> mark;
    function<bool(const string&)> visit = [&](const string& code) -> bool {
        Mark& m = mark[code];
        if (m == Mark::DONE)     return true;
        if (m == Mark::VISITING) return false;
        m = Mark::VISITING;
        const Course* course = catalog.find(code);
        if (course)
            for (const string& p : course->prereqs)
                if (!visit(p)) {
                    mark[code] = Mark::DONE;   // so the same cycle is reported only once
                    return false;
                }
        mark[code] = Mark::DONE;
        return true;
    };
    for (const auto& [code, course] : catalog.all())
        if (mark[code] == Mark::NEW && !visit(code)) {
            errors.push_back(code + ": prerequisites form a cycle");
            ok = false;
        }
    return ok;
}

vector<GenEdRequirement> loadGenEd(const string& genEdPath, vector<string>& errors) {
    vector<GenEdRequirement> out;
    json ge;
    if (!readJson(genEdPath, ge, errors)) return out;

    for (const json& entry : arrayAt(ge, "gen_ed", genEdPath, errors)) {
        GenEdRequirement req;
        req.phase = stringField(entry, "phase");
        req.text  = stringField(entry, "text");
        req.areas = stringList(entry, "areas");
        if (req.text.empty()) {
            errors.push_back(genEdPath + " gen_ed: requirement without text");
            continue;
        }
        for (const string& raw : stringList(entry, "courses")) {
            string code = normalizeCode(raw);
            if (code.empty()) errors.push_back(genEdPath + " gen_ed: bad course code \"" + raw + "\"");
            else              req.courses.push_back(code);
        }
        out.push_back(req);
    }
    return out;
}

vector<DistributionRule> loadDistributionRules(const string& genEdPath, vector<string>& errors) {
    vector<DistributionRule> out;
    json ge;
    if (!readJson(genEdPath, ge, errors)) return out;

    for (const json& entry : arrayAt(ge, "distribution_rules", genEdPath, errors)) {
        DistributionRule rule;
        rule.program = stringField(entry, "program");
        rule.areas   = stringList(entry, "areas");
        rule.note    = stringField(entry, "note");
        if (rule.program.empty() || !entry.contains("min_courses")
            || !entry["min_courses"].is_number_integer()) {
            errors.push_back(genEdPath + " distribution_rules: incomplete rule \"" + rule.program + "\"");
            continue;
        }
        rule.minCourses = entry["min_courses"].get<int>();
        // {"AR": 1, "HU": 1} -> {("AR", 1), ("HU", 1)}
        if (entry.contains("min_per_area") && entry["min_per_area"].is_object())
            for (const auto& [area, count] : entry["min_per_area"].items())
                if (count.is_number_integer()) rule.minPerArea.push_back({ area, count.get<int>() });
        out.push_back(rule);
    }
    return out;
}

bool checkLinks(const Courses& catalog, const vector<GenEdRequirement>& genEd,
                vector<string>& errors) {
    bool ok = true;
    for (const GenEdRequirement& req : genEd)
        for (const string& code : req.courses)
            if (!catalog.contains(code)) {
                errors.push_back("requirement \"" + req.text + "\" names unknown course " + code);
                ok = false;
            }
    return ok;
}
