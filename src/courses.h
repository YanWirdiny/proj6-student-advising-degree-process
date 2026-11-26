#pragma once
#include <map>
#include <string>
#include <vector>
#include "types.h"
using namespace std;

// The course catalog: every known course, looked up by code.
// Rule it guarantees: each code is stored normalized ("cs 110" -> "CS110") and only once.
class Courses {
public:
    // Stores a new course. Returns false and stores nothing when the code is malformed
    // or the course is already in the catalog.
    bool add(Course course);

    // Same course seen in another source: adds it if new, otherwise fills the fields the
    // stored copy is missing (name, distArea, credits, prereqs, coreqs, note) and keeps the stronger
    // category (CS > DISTRIBUTION > GEN_ED). Returns false and changes nothing when the
    // code is malformed or a field both copies have disagrees.
    bool merge(Course course);

    // nullptr when the course is not in the catalog. Accepts any spelling normalizeCode does.
    const Course* find(const string& code) const;
    bool          contains(const string& code) const { return find(code) != nullptr; }

    size_t                     size() const { return byCode_.size(); }
    const map<string, Course>& all()  const { return byCode_; }

    // Courses matching `query`, most relevant first; ties ordered by code. Case and spaces in
    // codes don't matter. Relevance:
    //   1. the query is exactly a code          "cs 310"    -> CS310
    //   2. a code starts with the query         "cs 4", "math"
    //   3. every word of the query starts a word of the title
    //                                           "data struct" -> CS210, CS310 ("cs" won't match "Physics")
    // An empty or blank query returns every course.
    vector<Course> search(const string& query) const;

    // Courses of one category, or carrying one distribution area, ordered by code.
    vector<Course> inCategory(CourseCategory category) const;
    vector<Course> inArea(const string& area) const;

private:
    map<string, Course> byCode_;   // key = normalized code
};
