#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "extractor_storage.h"

using namespace std;

static vector<string> codes(const string& input) {
    vector<string> out;
    for (const auto& course : CourseExtractor().extract(input))
        out.push_back(course.code);
    return out;
}

TEST(CourseExtractorTest, CompactCodes) {
    EXPECT_EQ(codes("CS101, CS341L; CS450"), (vector<string>{"CS101", "CS341L", "CS450"}));
}

TEST(CourseExtractorTest, SingleSpace) {
    EXPECT_EQ(codes("Take CS 341 before CS 450."), (vector<string>{"CS341", "CS450"}));
}

TEST(CourseExtractorTest, HtmlLayoutWhitespace) {
    EXPECT_EQ(codes("CS  101 CS\t220 CS\n341"), (vector<string>{"CS101", "CS220", "CS341"}));
}

TEST(CourseExtractorTest, Suffixes) {
    EXPECT_EQ(codes("CS 341L CS 450A1"), (vector<string>{"CS341L", "CS450A1"}));
}

TEST(CourseExtractorTest, MixedFormatsPreserveOrder) {
    EXPECT_EQ(codes("CS341 CS 341 CS450"), (vector<string>{"CS341", "CS341", "CS450"}));
}

TEST(CourseExtractorTest, Punctuation) {
    EXPECT_EQ(codes("(CS 101), [CS220]."), (vector<string>{"CS101", "CS220"}));
}

TEST(CourseExtractorTest, RejectEmbeddedPrefixes) {
    EXPECT_TRUE(codes("XCS 341 _CS 220 1CS101").empty());
}

TEST(CourseExtractorTest, RejectIncompleteOrUnknownCodes) {
    EXPECT_TRUE(codes("CS 12 CS ABC BIO 341").empty());
}

TEST(CourseExtractorTest, OtherSubjectsNormalized) {
    EXPECT_EQ(codes("Math 140, Physics\n113, English 101"),
              (vector<string>{"MATH140", "PHYSIC113", "ENGL101"}));
}

TEST(CourseExtractorTest, EmptyAndUnrelatedText) {
    EXPECT_TRUE(codes("").empty());
    EXPECT_TRUE(codes("No courses are listed.").empty());
}
