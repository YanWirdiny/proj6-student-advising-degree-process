#include <gtest/gtest.h>
#include "student.h"

using namespace std;

static CourseRecord done(const string& code, const string& term, const string& grade, double credits = 3) {
    return { code, term, grade, credits, CourseStatus::COMPLETED, CourseSource::UMB };
}

static CourseRecord taking(const string& code, const string& term, double credits = 3) {
    return { code, term, "", credits, CourseStatus::IN_PROGRESS, CourseSource::UMB };
}

static Student makeStudent() {
    return Student({ "Test Student", "00000000", "BS Computer Science", "2024-2025", "Fall 2024", 0 });
}

// ---- grade helpers ----

TEST(GradeTest, EarnsCredit) {
    EXPECT_TRUE(earnsCredit("A"));
    EXPECT_TRUE(earnsCredit("D-"));
    EXPECT_TRUE(earnsCredit("T"));
    EXPECT_TRUE(earnsCredit("P"));
    EXPECT_FALSE(earnsCredit("F"));
    EXPECT_FALSE(earnsCredit("W"));
    EXPECT_FALSE(earnsCredit("I"));
    EXPECT_FALSE(earnsCredit(""));
}

TEST(GradeTest, MeetsMinimum) {
    EXPECT_TRUE(meetsMinimum("C", "C-"));
    EXPECT_TRUE(meetsMinimum("C-", "C-"));
    EXPECT_FALSE(meetsMinimum("D+", "C-"));
    EXPECT_TRUE(meetsMinimum("D", ""));
    EXPECT_TRUE(meetsMinimum("T", "C"));
    EXPECT_FALSE(meetsMinimum("F", ""));
    EXPECT_FALSE(meetsMinimum("A", "Z"));
}

// ---- addRecord invariants ----

TEST(StudentTest, AddNormalizesCode) {
    Student s = makeStudent();
    ASSERT_TRUE(s.addRecord(done("cs 110", "Fall 2024", "A")));
    EXPECT_EQ(s.records().front().code, "CS110");
}

TEST(StudentTest, RejectsInconsistentRecords) {
    Student s = makeStudent();
    EXPECT_FALSE(s.addRecord(done("", "Fall 2024", "A")));                  // no code
    EXPECT_FALSE(s.addRecord(done("CS110", "Fall 2024", "")));              // completed, no grade
    CourseRecord graded = taking("CS210", "Spring 2025");
    graded.grade = "A";
    EXPECT_FALSE(s.addRecord(graded));                                      // in progress with grade
    EXPECT_FALSE(s.addRecord(done("CS110", "Fall 2024", "A", -3)));         // negative credits
    EXPECT_TRUE(s.records().empty());
}

TEST(StudentTest, RejectsDuplicateInSameTerm) {
    Student s = makeStudent();
    EXPECT_TRUE(s.addRecord(done("CS110", "Fall 2024", "F")));
    EXPECT_FALSE(s.addRecord(done("CS 110", "Fall 2024", "A")));
    EXPECT_TRUE(s.addRecord(done("CS110", "Spring 2025", "B")));            // a retake is fine
}

// ---- queries ----

TEST(StudentTest, HasPassedRespectsGrade) {
    Student s = makeStudent();
    s.addRecord(done("CS110", "Fall 2024", "D+"));
    EXPECT_TRUE(s.hasPassed("CS110"));
    EXPECT_FALSE(s.hasPassed("CS110", "C-"));
    EXPECT_FALSE(s.hasPassed("CS210"));
}

TEST(StudentTest, RetakeCountsBestAttempt) {
    Student s = makeStudent();
    s.addRecord(done("CS110", "Fall 2024", "F", 4));
    s.addRecord(done("CS110", "Spring 2025", "B", 4));
    EXPECT_TRUE(s.hasPassed("CS110", "C-"));
    EXPECT_DOUBLE_EQ(s.creditsCompleted(), 4);
}

TEST(StudentTest, InProgressIsTakingNotPassed) {
    Student s = makeStudent();
    s.addRecord(taking("CS210", "Fall 2025", 4));
    EXPECT_TRUE(s.isTaking("cs 210"));
    EXPECT_FALSE(s.hasPassed("CS210"));
    EXPECT_DOUBLE_EQ(s.creditsCompleted(), 0);
    EXPECT_DOUBLE_EQ(s.creditsInProgress(), 4);
}

TEST(StudentTest, TransferDetection) {
    Student s = makeStudent();
    s.addRecord(done("CS110", "Fall 2024", "A"));
    EXPECT_FALSE(s.isTransfer());
    s.addRecord({ "MATH140", "", "T", 4, CourseStatus::COMPLETED, CourseSource::TRANSFER });
    EXPECT_TRUE(s.isTransfer());
    EXPECT_TRUE(s.hasPassed("MATH140", "C"));
}

TEST(StudentTest, StandingFromCredits) {
    Student s = makeStudent();
    EXPECT_EQ(s.standing(), ClassStanding::FRESHMAN);
    for (int i = 0; i < 10; ++i)
        s.addRecord(done("CS" + to_string(100 + i), "Fall 2024", "B"));    // 30 credits
    EXPECT_EQ(s.standing(), ClassStanding::SOPHOMORE);
    for (int i = 10; i < 20; ++i)
        s.addRecord(done("CS" + to_string(100 + i), "Fall 2024", "B"));    // 60 credits
    EXPECT_EQ(s.standing(), ClassStanding::JUNIOR);
}
