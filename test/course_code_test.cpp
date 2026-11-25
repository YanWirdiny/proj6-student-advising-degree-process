#include <gtest/gtest.h>
#include "course_code.h"

using namespace std;

TEST(CourseCodeTest, AlreadyCanonical) {
    EXPECT_EQ(normalizeCode("CS110"), "CS110");
    EXPECT_EQ(normalizeCode("CS285L"), "CS285L");
}

TEST(CourseCodeTest, StripsWhitespace) {
    EXPECT_EQ(normalizeCode("CS 110"), "CS110");
    EXPECT_EQ(normalizeCode("  CS\t 110 \n"), "CS110");
    EXPECT_EQ(normalizeCode("EDC U 212"), "EDCU212");
}

TEST(CourseCodeTest, UpperCases) {
    EXPECT_EQ(normalizeCode("cs110"), "CS110");
    EXPECT_EQ(normalizeCode("Math 140"), "MATH140");
    EXPECT_EQ(normalizeCode("cs 285l"), "CS285L");
}

TEST(CourseCodeTest, SpelledOutSubjects) {
    EXPECT_EQ(normalizeCode("Physics113"), "PHYSIC113");
    EXPECT_EQ(normalizeCode("English  101"), "ENGL101");
    EXPECT_EQ(normalizeCode("Mathematics 140"), "MATH140");
}

TEST(CourseCodeTest, RejectsMalformed) {
    EXPECT_EQ(normalizeCode(""), "");
    EXPECT_EQ(normalizeCode("   "), "");
    EXPECT_EQ(normalizeCode("CS"), "");
    EXPECT_EQ(normalizeCode("110"), "");
}
