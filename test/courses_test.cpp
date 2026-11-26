#include <gtest/gtest.h>
#include "courses.h"

using namespace std;

static Course csCourse(const string& code, const string& name = "") {
    Course c;
    c.code     = code;
    c.category = CourseCategory::CS;
    c.name     = name;
    return c;
}

static Course distCourse(const string& code, const string& area, const string& name = "") {
    Course c;
    c.code     = code;
    c.category = CourseCategory::DISTRIBUTION;
    c.distArea = area;
    c.name     = name;
    return c;
}

// ---- add ----

TEST(CoursesAddTest, StoresNormalizedCode) {
    Courses catalog;
    EXPECT_TRUE(catalog.add(csCourse("cs 110", "Introduction to Computing")));
    ASSERT_NE(catalog.find("CS110"), nullptr);
    EXPECT_EQ(catalog.find("CS110")->code, "CS110");
    EXPECT_EQ(catalog.find("CS110")->name, "Introduction to Computing");
}

TEST(CoursesAddTest, FindAcceptsAnySpelling) {
    Courses catalog;
    catalog.add(distCourse("MATH140", "MT"));
    EXPECT_TRUE(catalog.contains("Math 140"));
    EXPECT_TRUE(catalog.contains("math140"));
    EXPECT_FALSE(catalog.contains("MATH141"));
}

TEST(CoursesAddTest, RejectsDuplicate) {
    Courses catalog;
    EXPECT_TRUE(catalog.add(csCourse("CS110")));
    EXPECT_FALSE(catalog.add(csCourse("cs 110", "Other name")));
    EXPECT_EQ(catalog.size(), 1u);
    EXPECT_EQ(catalog.find("CS110")->name, "");
}

TEST(CoursesAddTest, RejectsMalformedCode) {
    Courses catalog;
    EXPECT_FALSE(catalog.add(csCourse("")));
    EXPECT_FALSE(catalog.add(csCourse("110")));
    EXPECT_FALSE(catalog.add(csCourse("CS")));
    EXPECT_EQ(catalog.size(), 0u);
}

TEST(CoursesAddTest, NormalizesPrereqs) {
    Courses catalog;
    Course c = csCourse("CS310");
    c.prereqs = { "cs 210", "CS220" };
    catalog.add(c);
    EXPECT_EQ(catalog.find("CS310")->prereqs, (vector<string>{ "CS210", "CS220" }));
}

// ---- merge ----

TEST(CoursesMergeTest, AddsWhenNew) {
    Courses catalog;
    EXPECT_TRUE(catalog.merge(distCourse("PHIL100", "HU", "Introduction to Philosophy")));
    EXPECT_EQ(catalog.size(), 1u);
}

TEST(CoursesMergeTest, FillsMissingName) {
    // cs_courses.json lists MATH140 without a title; gen_ed_requirements.json has it.
    Courses catalog;
    catalog.add(distCourse("MATH140", "MT"));
    EXPECT_TRUE(catalog.merge(distCourse("MATH140", "MT", "Calculus I")));
    EXPECT_EQ(catalog.find("MATH140")->name, "Calculus I");
    EXPECT_EQ(catalog.size(), 1u);
}

TEST(CoursesMergeTest, CsCourseKeepsCategoryAndGainsArea) {
    // CS110 is a CS course and also listed under the MT distribution area.
    Courses catalog;
    catalog.add(csCourse("CS110"));
    EXPECT_TRUE(catalog.merge(distCourse("CS110", "MT", "Introduction to Computing")));
    const Course* cs110 = catalog.find("CS110");
    EXPECT_EQ(cs110->category, CourseCategory::CS);
    EXPECT_EQ(cs110->distArea, "MT");
    EXPECT_EQ(cs110->name, "Introduction to Computing");
}

TEST(CoursesMergeTest, StrongerCategoryWinsWhicheverComesFirst) {
    Courses catalog;
    catalog.add(distCourse("CS110", "MT"));
    EXPECT_TRUE(catalog.merge(csCourse("CS110")));
    EXPECT_EQ(catalog.find("CS110")->category, CourseCategory::CS);
}

TEST(CoursesMergeTest, ConflictChangesNothing) {
    Courses catalog;
    catalog.add(distCourse("MATH140", "MT"));
    EXPECT_FALSE(catalog.merge(distCourse("MATH140", "NS", "Calculus I")));
    EXPECT_EQ(catalog.find("MATH140")->distArea, "MT");
    EXPECT_EQ(catalog.find("MATH140")->name, "");   // name not filled either
}

TEST(CoursesMergeTest, ConflictingNameOrCredits) {
    Courses catalog;
    Course a = csCourse("CS110", "Introduction to Computing");
    a.credits = 4;
    catalog.add(a);
    EXPECT_FALSE(catalog.merge(csCourse("CS110", "Intro to Programming")));
    Course b = csCourse("CS110");
    b.credits = 3;
    EXPECT_FALSE(catalog.merge(b));
    EXPECT_EQ(catalog.find("CS110")->credits, 4);
}

TEST(CoursesMergeTest, RejectsMalformedCode) {
    Courses catalog;
    EXPECT_FALSE(catalog.merge(csCourse("not a course")));
    EXPECT_EQ(catalog.size(), 0u);
}

// ---- queries ----

TEST(CoursesQueryTest, InCategoryAndInArea) {
    Courses catalog;
    catalog.add(csCourse("CS210"));
    catalog.add(csCourse("CS110"));
    catalog.merge(distCourse("CS110", "MT"));
    catalog.add(distCourse("MATH140", "MT"));
    catalog.add(distCourse("PHIL100", "HU"));

    vector<Course> cs = catalog.inCategory(CourseCategory::CS);
    ASSERT_EQ(cs.size(), 2u);
    EXPECT_EQ(cs[0].code, "CS110");   // ordered by code
    EXPECT_EQ(cs[1].code, "CS210");

    vector<Course> mt = catalog.inArea("MT");
    ASSERT_EQ(mt.size(), 2u);
    EXPECT_EQ(mt[0].code, "CS110");
    EXPECT_EQ(mt[1].code, "MATH140");

    EXPECT_TRUE(catalog.inArea("SB").empty());
}

TEST(CoursesQueryTest, FindMissingReturnsNull) {
    Courses catalog;
    EXPECT_EQ(catalog.find("CS110"), nullptr);
    EXPECT_EQ(catalog.find(""), nullptr);
}

// ---- search ----

static Courses searchCatalog() {
    Courses catalog;
    catalog.add(csCourse("CS210", "Intermediate Computing with Data Structures"));
    catalog.add(csCourse("CS310", "Advanced Data Structures and Algorithms"));
    catalog.add(csCourse("CS410", "Introduction to Software Engineering"));
    catalog.add(csCourse("CS411", "Competitive Programming"));
    catalog.add(csCourse("CS31"));                       // odd code, to test prefix vs exact
    catalog.add(distCourse("MATH140", "MT", "Calculus I"));
    catalog.add(distCourse("MATH141", "MT", "Calculus II"));
    return catalog;
}

static vector<string> codes(const vector<Course>& courses) {
    vector<string> out;
    for (const Course& c : courses) out.push_back(c.code);
    return out;
}

TEST(CoursesSearchTest, ExactCodeFirstThenPrefix) {
    // "cs 31" is exactly CS31 and also a prefix of CS310: the exact match comes first.
    EXPECT_EQ(codes(searchCatalog().search("cs 31")), (vector<string>{ "CS31", "CS310" }));
}

TEST(CoursesSearchTest, CodePrefixAnySpelling) {
    vector<string> expected{ "CS410", "CS411" };
    EXPECT_EQ(codes(searchCatalog().search("cs 4")), expected);
    EXPECT_EQ(codes(searchCatalog().search("CS4")), expected);
    EXPECT_EQ(codes(searchCatalog().search("  cs4  ")), expected);
}

TEST(CoursesSearchTest, SubjectAlone) {
    EXPECT_EQ(codes(searchCatalog().search("math")), (vector<string>{ "MATH140", "MATH141" }));
}

TEST(CoursesSearchTest, TitleWordsAllMustMatch) {
    EXPECT_EQ(codes(searchCatalog().search("data struct")), (vector<string>{ "CS210", "CS310" }));
    EXPECT_EQ(codes(searchCatalog().search("advanced data")), (vector<string>{ "CS310" }));
    EXPECT_EQ(codes(searchCatalog().search("CALCULUS")), (vector<string>{ "MATH140", "MATH141" }));
}

TEST(CoursesSearchTest, CodeMatchesRankAboveTitleMatches) {
    Courses catalog = searchCatalog();
    catalog.add(csCourse("CS101", "Math for Computing"));   // "math" in the title only
    EXPECT_EQ(codes(catalog.search("math")), (vector<string>{ "MATH140", "MATH141", "CS101" }));
}

TEST(CoursesSearchTest, EmptyQueryReturnsAllInCodeOrder) {
    Courses catalog = searchCatalog();
    EXPECT_EQ(catalog.search("").size(), catalog.size());
    EXPECT_EQ(catalog.search("   ").size(), catalog.size());
    EXPECT_EQ(catalog.search("").front().code, "CS210");
}

TEST(CoursesSearchTest, NoMatch) {
    EXPECT_TRUE(searchCatalog().search("quantum").empty());
    EXPECT_TRUE(searchCatalog().search("cs 9").empty());
}
