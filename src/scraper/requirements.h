#pragma once
#include <set>
#include <string>
#include <vector>
#include "parser.h"
#include "types.h"
using namespace std;

// Pulls the first-year / middle / capstone requirement items off the Gen Ed page.
class GenEdExtractor {
public:
    vector<GenEdRequirement> extract(const vector<HtmlBlock>& blocks) const;
};

// Pulls the area headings (e.g. "Arts and Humanities (AR and HU)") and their outcomes
// off the Distribution Requirement page.
class DistributionExtractor {
public:
    vector<DistributionArea> extract(const vector<HtmlBlock>& blocks) const;
};

// Pulls course headings ("PHIL 100 Introduction to Philosophy") off one page of a
// courses.umb.edu search filtered by a distribution area. Empty result = past the last page.
class DistributionCourseExtractor {
public:
    vector<DistributionCourse> extract(const vector<HtmlBlock>& blocks, const string& area) const;
};

// Per-program course counts. Source is a PDF chart linked from the distribution page
// (SpecDistReq.pdf), so the table is maintained by hand rather than crawled.
vector<DistributionRule> getDistributionRules();

class RequirementStorage {
public:
    explicit RequirementStorage(const string& filepath);
    void addGenEd(const vector<GenEdRequirement>& reqs);
    void addAreas(const vector<DistributionArea>& areas);
    void addDistributionCourses(const vector<DistributionCourse>& courses);
    void save() const;
private:
    string                      filepath_;
    vector<GenEdRequirement>    genEd_;
    vector<DistributionArea>    areas_;
    set<string>                 seenCourses_;   // "AREA:CODE", a course can sit in several areas
    vector<DistributionCourse>  distCourses_;   // nested under their area on save
};
