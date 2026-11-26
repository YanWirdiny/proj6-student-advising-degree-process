#include "courses.h"
#include "course_code.h"

using namespace std;

// Higher rank wins when two sources disagree on the category of the same course:
// a CS course that also counts toward a distribution area is still a CS course.
static int categoryRank(CourseCategory category) {
    switch (category) {
        case CourseCategory::CS:           return 2;
        case CourseCategory::DISTRIBUTION: return 1;
        case CourseCategory::GEN_ED:       return 0;
    }
    return 0;
}

// Two values clash only when both are known and different; an empty value is "unknown".
static bool clashes(const string& a, const string& b) {
    return !a.empty() && !b.empty() && a != b;
}

// Same rule for lists: an empty list is "unknown".
static bool clashes(const vector<string>& a, const vector<string>& b) {
    return !a.empty() && !b.empty() && a != b;
}

static void normalizeAll(vector<string>& codes) {
    for (string& code : codes) code = normalizeCode(code);
}

bool Courses::add(Course course) {
    course.code = normalizeCode(course.code);
    if (course.code.empty())          return false;
    if (byCode_.count(course.code))   return false;
    normalizeAll(course.prereqs);
    normalizeAll(course.coreqs);
    byCode_[course.code] = course;
    return true;
}

bool Courses::merge(Course course) {
    course.code = normalizeCode(course.code);
    if (course.code.empty()) return false;

    auto it = byCode_.find(course.code);
    if (it == byCode_.end()) return add(course);

    Course& stored = it->second;
    normalizeAll(course.prereqs);
    normalizeAll(course.coreqs);

    // Check every field first so a conflict leaves the stored course untouched.
    if (clashes(stored.name, course.name))                             return false;
    if (clashes(stored.distArea, course.distArea))                     return false;
    if (stored.credits != 0 && course.credits != 0
        && stored.credits != course.credits)                          return false;
    if (clashes(stored.prereqs, course.prereqs))                       return false;
    if (clashes(stored.coreqs, course.coreqs))                         return false;
    if (clashes(stored.note, course.note))                             return false;

    if (stored.name.empty())     stored.name     = course.name;
    if (stored.distArea.empty()) stored.distArea = course.distArea;
    if (stored.credits == 0)     stored.credits  = course.credits;
    if (stored.prereqs.empty())  stored.prereqs  = course.prereqs;
    if (stored.coreqs.empty())   stored.coreqs   = course.coreqs;
    if (stored.note.empty())     stored.note     = course.note;
    if (categoryRank(course.category) > categoryRank(stored.category)) stored.category = course.category;
    return true;
}

const Course* Courses::find(const string& code) const {
    auto it = byCode_.find(normalizeCode(code));
    return it == byCode_.end() ? nullptr : &it->second;
}

vector<Course> Courses::inCategory(CourseCategory category) const {
    vector<Course> out;
    for (const auto& [code, course] : byCode_)
        if (course.category == category) out.push_back(course);
    return out;
}

vector<Course> Courses::inArea(const string& area) const {
    vector<Course> out;
    for (const auto& [code, course] : byCode_)
        if (course.distArea == area) out.push_back(course);
    return out;
}
