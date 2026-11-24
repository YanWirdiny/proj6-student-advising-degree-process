#pragma once
#include <string>
#include <vector>

enum class CourseStatus  { COMPLETED, IN_PROGRESS };
enum class CourseSource  { UMB, TRANSFER, AP };
enum class ClassStanding { FRESHMAN, SOPHOMORE, JUNIOR, SENIOR };

// One enrollment as the audit lists it. Plain data: the rules about the course itself
// (prerequisites, distribution area, ...) live in the catalog and are looked up by code.
struct CourseRecord {
    std::string  code;                          // normalized on add, e.g. "CS110"
    std::string  term;                          // e.g. "Fall 2024"; may be empty for transfer/AP
    std::string  grade;                         // "B+", "T", "P", ...; empty while in progress
    double       credits = 0;                   // as awarded on the audit
    CourseStatus status  = CourseStatus::COMPLETED;
    CourseSource source  = CourseSource::UMB;
};

// Who the student is, as printed in the audit header. Plain data, set once.
struct StudentInfo {
    std::string name;
    std::string schoolId;
    std::string program;           // e.g. "BS Computer Science"
    std::string catalogYear;       // requirements the student is held to, e.g. "2023-2024"
    std::string entryTerm;         // e.g. "Fall 2023"
    double      auditCredits = 0;  // total as printed, to cross-check creditsCompleted()
};

// A student's course history. The record list is the single source of truth:
// credits and standing are always derived from it, never stored separately.
class Student {
public:
    explicit Student(const StudentInfo& info);

    const StudentInfo&               info()    const { return info_; }
    const std::vector<CourseRecord>& records() const { return records_; }

    // Normalizes the code and stores the record. Returns false and stores nothing when
    // the code is malformed, credits are negative, the grade does not match the status
    // (completed needs a grade, in progress must not have one), or the same course is
    // already recorded for the same term.
    bool addRecord(CourseRecord record);

    // Completed with a credit-earning grade at or above minGrade ("" = any passing grade).
    bool hasPassed(const std::string& code, const std::string& minGrade = "") const;
    bool isTaking(const std::string& code) const;
    bool isTransfer() const;

    // Each passed course counts once, even when repeated.
    double        creditsCompleted()  const;
    double        creditsInProgress() const;
    ClassStanding standing()          const;

private:
    StudentInfo               info_;
    std::vector<CourseRecord> records_;
};

// Grade helpers, kept public for tests and for prerequisite checks later.
bool earnsCredit(const std::string& grade);
bool meetsMinimum(const std::string& grade, const std::string& minGrade);
