#pragma once
#include <string>
#include <vector>
#include <set>
#include "types.h"
using namespace std;

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
