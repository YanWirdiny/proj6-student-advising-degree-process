#include "extractor_storage.h"
#include <regex>
#include <fstream>

// Pattern: "CS" followed by exactly 3 digits, then any trailing alphanumeric chars.
// Matches tokens like CS101, CS341L, CS450 — stops at any non-alphanumeric character.
static const std::regex kCoursePattern(R"(\bCS\d{3}[A-Za-z0-9]*)");

std::vector<Course> CourseExtractor::extract(const std::string& text) const {
    std::vector<Course> results;
    auto begin = std::sregex_iterator(text.begin(), text.end(), kCoursePattern);
    auto end   = std::sregex_iterator();
    for (auto it = begin; it != end; ++it)
        results.push_back({ it->str() });
    return results;
}

CourseStorage::CourseStorage(const std::string& filepath)
    : filepath_(filepath) {}

void CourseStorage::add(const std::vector<Course>& courses) {
    for (const auto& c : courses)
        if (seen_.insert(c.code).second)
            courses_.push_back(c);
}

void CourseStorage::save() const {
    std::ofstream out(filepath_);
    out << "{\n  \"cs_courses\": [\n";
    for (size_t i = 0; i < courses_.size(); ++i) {
        out << "    \"" << courses_[i].code << "\"";
        if (i + 1 < courses_.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
}
