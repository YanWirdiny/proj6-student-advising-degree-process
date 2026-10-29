#include "extractor_storage.h"
#include <regex>
#include <fstream>

// Course references as the CS and Gen Ed pages write them: "CS110", "CS285L", "Math140",
// "Physics113", "English 101". Group 1 is the subject as written, group 2 the number.
static const std::regex kCoursePattern(R"(\b(CS|Math|MATH|Physics|PHYSIC|English|ENGL) ?(\d{3}[A-Za-z0-9]*))");

// Spelled-out subject -> catalog prefix.
static std::string normalizeSubject(const std::string& subject) {
    if (subject == "Math")    return "MATH";
    if (subject == "Physics") return "PHYSIC";
    if (subject == "English") return "ENGL";
    return subject;
}

// Subjects whose courses count only toward a distribution area for the CS major.
std::string defaultDistArea(const std::string& subject) {
    if (subject == "MATH")   return "MT";
    if (subject == "PHYSIC") return "NS";
    return "";
}

CourseCategory classify(const string& subject, const string& distArea) {
    if (subject == "CS")   return CourseCategory::CS;
    if (subject == "ENGL") return CourseCategory::GEN_ED;
    if (!distArea.empty()) return CourseCategory::DISTRIBUTION;
    return CourseCategory::GEN_ED;
}

std::vector<Course> CourseExtractor::extract(const std::string& text) const {
    std::vector<Course> results;
    auto begin = std::sregex_iterator(text.begin(), text.end(), kCoursePattern);
    auto end   = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        std::string subject = normalizeSubject((*it)[1]);
        std::string area    = defaultDistArea(subject);
        results.push_back({ subject + (*it)[2].str(), classify(subject, area), area });
    }
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
    const std::pair<CourseCategory, const char*> kSections[] = {
        { CourseCategory::CS,           "cs_courses" },
        { CourseCategory::GEN_ED,       "gen_ed_courses" },
        { CourseCategory::DISTRIBUTION, "distribution_courses" },
    };
    std::ofstream out(filepath_);
    out << "{\n";
    for (size_t s = 0; s < 3; ++s) {
        std::vector<const Course*> section;
        for (const auto& c : courses_)
            if (c.category == kSections[s].first) section.push_back(&c);

        out << "  \"" << kSections[s].second << "\": [";
        for (size_t i = 0; i < section.size(); ++i) {
            out << (i ? ",\n    " : "\n    ");
            if (section[i]->distArea.empty())
                out << "\"" << section[i]->code << "\"";
            else
                out << "{ \"code\": \"" << section[i]->code << "\", \"area\": \"" << section[i]->distArea << "\" }";
        }
        out << (section.empty() ? "]" : "\n  ]") << (s + 1 < 3 ? "," : "") << "\n";
    }
    out << "}\n";
}
