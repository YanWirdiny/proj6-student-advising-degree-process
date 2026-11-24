#pragma once
#include <string>
#include <vector>
using namespace std;
enum class CourseStatus  { COMPLETED, IN_PROGRESS };
enum class CourseSource  { UMB, TRANSFER, AP };
enum class ClassStanding { FRESHMAN, SOPHOMORE, JUNIOR, SENIOR };

// One enrollment as the audit lists it. Plain data: the rules about the course itself
// (prerequisites, distribution area, ...) live in the catalog and are looked up by code.
struct CourseRecord {
    string  code;                         
    string  term;
    string  grade;                         
    double       credits = 0;                  
    CourseStatus status  = CourseStatus::COMPLETED;
    CourseSource source  = CourseSource::UMB;
};

// Who the student is, as printed in the audit header. Plain data, set once.
struct StudentInfo {
    string name;
    string schoolId;
    string major;          
    string catalogYear;       
    string entryTerm;          
    double      auditCredits = 0;  
};

// A student's course history. The record list is the single source of truth:
// credits and standing are always derived from it, never stored separately.
class Student {
public:
    explicit Student(const StudentInfo& info);

    const StudentInfo&               info()    const { return info_; }
    const vector<CourseRecord>& records() const { return records_; }

    // Normalizes the code and stores the record. Returns false and stores nothing when
    // the code is malformed, credits are negative, the grade does not match the status
    // (completed needs a grade, in progress must not have one), or the same course is
    // already recorded for the same term.
    bool addRecord(CourseRecord record);

    // Completed with a credit-earning grade at or above minGrade ("" = any passing grade).
    bool hasPassed(const string& code, const string& minGrade = "") const;
    bool isTaking(const string& code) const;
    bool isTransfer() const;

    // Each passed course counts once, even when repeated.
    double        creditsCompleted()  const;
    double        creditsInProgress() const;
    ClassStanding standing()          const;

private:
    StudentInfo               info_;
    vector<CourseRecord> records_;
};

// Grade helpers, kept public for tests and for prerequisite checks later.
bool earnsCredit(const string& grade);
bool meetsMinimum(const string& grade, const string& minGrade);
