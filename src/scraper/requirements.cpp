#include "requirements.h"
#include <algorithm>
#include <fstream>
#include <regex>

// "English 101", "Math 125", "BC290" -> course codes. Spelled-out subjects map to catalog prefixes.
static const regex kCourseRef(R"(\b(English|Math|[A-Z]{2,4}) ?(\d{3}[A-Z]?)\b)");
// "(AR)" or "(AR and HU)" -> distribution codes.
static const regex kAreaRef(R"(\(([A-Z]{2})(?: and ([A-Z]{2}))?\))");

static vector<string> findCourses(const string& text) {
    vector<string> codes;
    for (auto it = sregex_iterator(text.begin(), text.end(), kCourseRef); it != sregex_iterator(); ++it) {
        string subject = (*it)[1];
        if      (subject == "English") subject = "ENGL";
        else if (subject == "Math")    subject = "MATH";
        codes.push_back(subject + (*it)[2].str());
    }
    return codes;
}

static vector<string> findAreas(const string& text) {
    vector<string> codes;
    for (auto it = sregex_iterator(text.begin(), text.end(), kAreaRef); it != sregex_iterator(); ++it)
        for (size_t g = 1; g < it->size(); ++g)
            if ((*it)[g].matched) codes.push_back((*it)[g]);
    return codes;
}

static bool startsWith(const string& s, const string& prefix) {
    return s.compare(0, prefix.size(), prefix) == 0;
}

vector<GenEdRequirement> GenEdExtractor::extract(const vector<HtmlBlock>& blocks) const {
    vector<GenEdRequirement> results;
    string phase;
    for (const auto& b : blocks) {
        if (b.tag == "h2" || b.tag == "h3") { phase.clear(); continue; }
        if (b.tag == "p") {
            if      (startsWith(b.text, "The first-year experience")) phase = "first_year";
            else if (startsWith(b.text, "The middle phase"))          phase = "middle";
            else if (startsWith(b.text, "In the last phase")) {
                // The capstone phase has no list; its first sentence is the requirement.
                results.push_back({ "capstone", b.text.substr(0, b.text.find('.') + 1), {}, {} });
                phase.clear();
            }
            continue;
        }
        if (b.tag == "li" && !phase.empty())
            results.push_back({ phase, b.text, findCourses(b.text), findAreas(b.text) });
    }
    return results;
}

vector<DistributionArea> DistributionExtractor::extract(const vector<HtmlBlock>& blocks) const {
    vector<DistributionArea> results;
    bool inArea = false;
    for (const auto& b : blocks) {
        if (b.tag == "h2" || b.tag == "h3") {
            vector<string> codes = findAreas(b.text);
            inArea = !codes.empty();
            if (inArea) results.push_back({ b.text.substr(0, b.text.find(" (")), codes, {} });
        }
        else if (b.tag == "p") inArea = false;
        else if (b.tag == "li" && inArea) results.back().outcomes.push_back(b.text);
    }
    return results;
}

// "PHIL 100 Introduction to Philosophy", "EDC U 212 ..." -> subject, number, title.
static const regex kCourseHeading(R"(^([A-Z]+(?: [A-Z]+)?) (\d+[A-Za-z]*) (.+)$)");

vector<DistributionCourse> DistributionCourseExtractor::extract(const vector<HtmlBlock>& blocks,
                                                                const string& area) const {
    vector<DistributionCourse> results;
    smatch m;
    for (const auto& b : blocks) {
        if (b.tag != "h2" || !regex_match(b.text, m, kCourseHeading)) continue;
        string subject = m[1];
        subject.erase(remove(subject.begin(), subject.end(), ' '), subject.end());
        results.push_back({ subject + m[2].str(), m[3], area });
    }
    return results;
}

vector<DistributionRule> getDistributionRules() {
    const string ba   = "BA (Liberal Arts, Science & Math, Education & Human Development, Public & Community Service)";
    const string bs   = "BS (Liberal Arts, Science & Math)";
    const string cm   = "College of Management";
    const string cnhs = "College of Nursing & Health Sciences";
    return {
        { ba,   { "AR", "HU" }, 3, { { "AR", 1 }, { "HU", 1 } }, "" },
        { ba,   { "MT", "NS" }, 3, { { "NS", 1 } },              "Any math course must be MATH125 or higher" },
        { ba,   { "SB" },       3, {},                           "" },
        { ba,   { "WC", "WL" }, 2, {},                           "Elementary language proficiency also required" },

        { bs,   { "AR", "HU" }, 2, { { "AR", 1 }, { "HU", 1 } }, "" },
        { bs,   { "MT", "NS" }, 4, { { "NS", 1 } },              "Any math course must be Calculus II or higher" },
        { bs,   { "SB" },       2, {},                           "" },
        { bs,   { "WC", "WL" }, 1, {},                           "A WL course must be at the 102 level or higher" },

        { cm,   { "AR", "HU" }, 2, {},                           "" },
        { cm,   { "MT", "NS" }, 2, {},                           "Any math course must be above MATH134" },
        { cm,   { "SB" },       2, {},                           "" },
        { cm,   { "WC", "WL" }, 1, {},                           "" },

        { cnhs, { "AR", "HU" }, 2, {},                           "" },
        { cnhs, { "NS" },       2, {},                           "" },
        { cnhs, { "SB" },       1, {},                           "" },
        { cnhs, { "WC", "WL" }, 2, {},                           "" },
    };
}

RequirementStorage::RequirementStorage(const string& filepath)
    : filepath_(filepath) {}

void RequirementStorage::addGenEd(const vector<GenEdRequirement>& reqs) {
    genEd_.insert(genEd_.end(), reqs.begin(), reqs.end());
}

void RequirementStorage::addAreas(const vector<DistributionArea>& areas) {
    areas_.insert(areas_.end(), areas.begin(), areas.end());
}

void RequirementStorage::addDistributionCourses(const vector<DistributionCourse>& courses) {
    for (const auto& c : courses)
        if (seenCourses_.insert(c.area + ":" + c.code).second)
            distCourses_.push_back(c);
}

static string quote(const string& s) {
    string out = "\"";
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else                out += c;
    }
    return out + "\"";
}

static string list(const vector<string>& items) {
    string out = "[";
    for (size_t i = 0; i < items.size(); ++i)
        out += (i ? ", " : "") + quote(items[i]);
    return out + "]";
}

void RequirementStorage::save() const {
    ofstream out(filepath_);
    out << "{\n  \"gen_ed\": [\n";
    for (size_t i = 0; i < genEd_.size(); ++i) {
        const auto& r = genEd_[i];
        out << "    { \"phase\": " << quote(r.phase) << ", \"text\": " << quote(r.text)
            << ", \"courses\": " << list(r.courses) << ", \"areas\": " << list(r.areas) << " }"
            << (i + 1 < genEd_.size() ? "," : "") << "\n";
    }
    out << "  ],\n  \"distribution_areas\": [\n";
    for (size_t i = 0; i < areas_.size(); ++i) {
        const auto& a = areas_[i];
        out << "    { \"name\": " << quote(a.name) << ", \"codes\": " << list(a.codes)
            << ", \"outcomes\": " << list(a.outcomes) << ", \"courses\": [";
        bool first = true;
        for (const auto& c : distCourses_) {
            if (find(a.codes.begin(), a.codes.end(), c.area) == a.codes.end()) continue;
            out << (first ? "\n" : ",\n") << "        { \"code\": " << quote(c.code) << ", \"title\": "
                << quote(c.title) << ", \"area\": " << quote(c.area) << " }";
            first = false;
        }
        out << (first ? "] }" : "\n      ] }") << (i + 1 < areas_.size() ? "," : "") << "\n";
    }
    out << "  ],\n  \"distribution_rules\": [\n";
    vector<DistributionRule> rules = getDistributionRules();
    for (size_t i = 0; i < rules.size(); ++i) {
        const auto& r = rules[i];
        out << "    { \"program\": " << quote(r.program) << ", \"areas\": " << list(r.areas)
            << ", \"min_courses\": " << r.minCourses << ", \"min_per_area\": {";
        for (size_t k = 0; k < r.minPerArea.size(); ++k)
            out << (k ? ", " : " ") << quote(r.minPerArea[k].first) << ": " << r.minPerArea[k].second
                << (k + 1 == r.minPerArea.size() ? " " : "");
        out << "}, \"note\": " << quote(r.note) << " }"
            << (i + 1 < rules.size() ? "," : "") << "\n";
    }
    out << "  ]\n}\n";
}
