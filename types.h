#pragma once
#include <string>
#include <utility>
#include <vector>

// Bucket a course falls into for the CS major.
enum class CourseCategory {
    CS,             // any CS-prefixed course
    GEN_ED,         // English and any other non-CS course without a distribution area
    DISTRIBUTION    // course carrying an AR/HU/MT/NS/SB/WC/WL designation (Math -> MT, Physics -> NS)
};

struct Course {
    std::string    code;       // e.g. "CS110", "MATH140", "ENGL101"
    CourseCategory category;
    std::string    distArea;   // distribution code when category is DISTRIBUTION, else empty
};

// One item of the general education program, e.g. "Two courses in writing and composition".
struct GenEdRequirement {
    std::string              phase;     // "first_year", "middle" or "capstone"
    std::string              text;      // requirement as written on the page
    std::vector<std::string> courses;   // course codes named in the text, e.g. "ENGL101"
    std::vector<std::string> areas;     // distribution codes named in the text, e.g. "AR"
};

// A course approved for one distribution area, from the courses.umb.edu search.
struct DistributionCourse {
    std::string code;    // e.g. "PHIL100"
    std::string title;   // e.g. "Introduction to Philosophy"
    std::string area;    // e.g. "HU"
};

// A distribution area as described on the distribution requirement page.
struct DistributionArea {
    std::string              name;      // e.g. "Arts and Humanities"
    std::vector<std::string> codes;     // e.g. {"AR", "HU"}
    std::vector<std::string> outcomes;  // learning goals listed under the area
};

// How many distribution courses one program needs from a pool of areas.
struct DistributionRule {
    std::string                              program;      // e.g. "BS (CLASM)"
    std::vector<std::string>                 areas;        // codes that count toward the rule
    int                                      minCourses;
    std::vector<std::pair<std::string, int>> minPerArea;   // e.g. {"NS", 1}
    std::string                              note;         // constraints not modelled above
};
