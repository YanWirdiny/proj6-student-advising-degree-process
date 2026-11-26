#include "course_code.h"
#include <cctype>
#include <map>

using namespace std;

// Spelled-out subjects -> catalog prefix.
static const map<string, string> kSubjectAliases = {
    {"MATHEMATICS", "MATH"},
    {"PHYSICS",     "PHYSIC"},
    {"ENGLISH",     "ENGL"},
};

string normalizeCode(const string& raw) {
    string compact;
    for (char c : raw) {
        unsigned char u = static_cast<unsigned char>(c);
        if (!isspace(u)) compact += static_cast<char>(toupper(u));
    }

    size_t firstDigit = compact.find_first_of("0123456789");
    if (firstDigit == string::npos || firstDigit == 0) return "";

    string subject = compact.substr(0, firstDigit);
    auto alias = kSubjectAliases.find(subject);
    if (alias != kSubjectAliases.end()) subject = alias->second;

    return subject + compact.substr(firstDigit);
}

string subjectOf(const string& code) {
    size_t firstDigit = code.find_first_of("0123456789");
    return firstDigit == string::npos ? code : code.substr(0, firstDigit);
}
