#pragma once
#include <string>
#include <vector>
#include <set>
#include "types.h"
using namespace std;

// Distribution area implied by the subject alone: MATH -> MT, PHYSIC -> NS, otherwise empty.
string defaultDistArea(const string& subject);

// CS prefix -> CS; English -> Gen Ed; a distribution designation (AR, HU, MT, NS,
// SB, WC, WL) -> Distribution; anything left -> Gen Ed.
CourseCategory classify(const string& subject, const string& distArea = "");

class CourseExtractor {
public:
    vector<Course> extract(const std::string& text) const;
};

class CourseStorage {
public:
    explicit CourseStorage(const std::string& filepath);
    void add(const std::vector<Course>& courses);
    void save() const;
private:
    string            filepath_;
    set<std::string>  seen_;
    vector<Course>    courses_;
};
