#include "student.h"
#include "course_code.h"
#include <map>
#include <set>

using namespace std;

static const map<string, double> kGradePoints = {
    {"A",  4.0}, {"A-", 3.7},
    {"B+", 3.3}, {"B",  3.0}, {"B-", 2.7},
    {"C+", 2.3}, {"C",  2.0}, {"C-", 1.7},
    {"D+", 1.3}, {"D",  1.0}, {"D-", 0.7},
    {"F",  0.0},
};

// Earn credit but carry no grade points: transfer, pass, credit.
static const set<string> kCreditNoPoints = {"T", "P", "CR"};

bool earnsCredit(const string& grade) {
    if (kCreditNoPoints.count(grade)) return true;
    auto it = kGradePoints.find(grade);
    return it != kGradePoints.end() && it->second > 0;   // F, W, I, NR, AU -> no credit
}

bool meetsMinimum(const string& grade, const string& minGrade) {
    if (!earnsCredit(grade)) return false;
    // Assumption: a transferred or pass grade satisfies any minimum, since only
    // C-or-better work transfers in. Revisit if the catalog says otherwise.
    if (minGrade.empty() || kCreditNoPoints.count(grade)) return true;
    auto min = kGradePoints.find(minGrade);
    if (min == kGradePoints.end()) return false;   // unknown minimum: be strict
    return kGradePoints.at(grade) >= min->second;
}

Student::Student(const StudentInfo& info) : info_(info) {}

bool Student::addRecord(CourseRecord record) {
    record.code = normalizeCode(record.code);
    if (record.code.empty() || record.credits < 0) return false;

    bool hasGrade = !record.grade.empty();
    if ((record.status == CourseStatus::COMPLETED) != hasGrade) return false;

    for (const auto& r : records_)
        if (r.code == record.code && r.term == record.term) return false;

    records_.push_back(record);
    return true;
}

bool Student::hasPassed(const string& code, const string& minGrade) const {
    string key = normalizeCode(code);
    for (const auto& r : records_)
        if (r.code == key && r.status == CourseStatus::COMPLETED && meetsMinimum(r.grade, minGrade))
            return true;
    return false;
}

bool Student::isTaking(const string& code) const {
    string key = normalizeCode(code);
    for (const auto& r : records_)
        if (r.code == key && r.status == CourseStatus::IN_PROGRESS) return true;
    return false;
}

bool Student::isTransfer() const {
    for (const auto& r : records_)
        if (r.source == CourseSource::TRANSFER) return true;
    return false;
}

double Student::creditsCompleted() const {
    map<string, double> best;   // code -> credits of its best passing attempt
    for (const auto& r : records_)
        if (r.status == CourseStatus::COMPLETED && earnsCredit(r.grade))
            best[r.code] = max(best[r.code], r.credits);

    double total = 0;
    for (const auto& entry : best) total += entry.second;
    return total;
}

double Student::creditsInProgress() const {
    double total = 0;
    for (const auto& r : records_)
        if (r.status == CourseStatus::IN_PROGRESS) total += r.credits;
    return total;
}

ClassStanding Student::standing() const {
    double credits = creditsCompleted();
    if (credits >= 90) return ClassStanding::SENIOR;
    if (credits >= 60) return ClassStanding::JUNIOR;
    if (credits >= 30) return ClassStanding::SOPHOMORE;
    return ClassStanding::FRESHMAN;
}
