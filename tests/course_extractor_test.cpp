#include "extractor_storage.h"
#include <iostream>
#include <vector>

int main() {
    const CourseExtractor extractor;
    int failures = 0;
    auto check = [&](const char* name, const std::string& input,
                     const std::vector<std::string>& expected) {
        std::vector<std::string> actual;
        for (const auto& course : extractor.extract(input))
            actual.push_back(course.code);
        if (actual != expected) {
            std::cerr << "FAIL: " << name << '\n';
            ++failures;
        }
    };

    check("compact codes", "CS101, CS341L; CS450",
          {"CS101", "CS341L", "CS450"});
    check("single space", "Take CS 341 before CS 450.",
          {"CS341", "CS450"});
    check("HTML layout whitespace", "CS  101 CS\t220 CS\n341",
          {"CS101", "CS220", "CS341"});
    check("suffixes", "CS 341L CS 450A1", {"CS341L", "CS450A1"});
    check("mixed formats preserve order", "CS341 CS 341 CS450",
          {"CS341", "CS341", "CS450"});
    check("punctuation", "(CS 101), [CS220].", {"CS101", "CS220"});
    check("reject embedded prefixes", "XCS 341 _CS 220 1CS101", {});
    check("reject incomplete codes", "CS 12 CS ABC MATH 341", {});
    check("empty input", "", {});
    check("unrelated text", "No courses are listed.", {});
    // Explicit return checks remain active in Release builds (unlike assert).
    return failures == 0 ? 0 : 1;
}
