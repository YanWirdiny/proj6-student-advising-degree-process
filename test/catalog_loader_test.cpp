#include <gtest/gtest.h>
#include <fstream>
#include "catalog_loader.h"

using namespace std;

// Writes `content` to a fresh file in the test temp directory and returns its path.
static string writeFile(const string& name, const string& content) {
    string path = testing::TempDir() + name;
    ofstream(path) << content;
    return path;
}

static const string kRealCs    = string(PROJECT_DIR) + "/cs_courses.json";
static const string kRealGenEd = string(PROJECT_DIR) + "/gen_ed_requirements.json";
static const string kRealDetails = string(PROJECT_DIR) + "/course_details.json";

// Small files shaped like the real ones, with the overlaps the real data has.
static const char* kCsJson = R"json({
  "cs_courses": ["CS110", "cs 210", "CS285L"],
  "gen_ed_courses": ["ENGL101", "ENGL102"],
  "distribution_courses": [ { "code": "MATH140", "area": "MT" } ]
})json";

static const char* kGenEdJson = R"json({
  "gen_ed": [
    { "phase": "first_year", "text": "Writing", "courses": ["English 101", "ENGL102"], "areas": [] },
    { "phase": "middle", "text": "Arts and Humanities", "courses": [], "areas": ["AR", "HU"] }
  ],
  "distribution_areas": [
    { "name": "Natural Sciences and Mathematics", "codes": ["NS", "MT"], "outcomes": [],
      "courses": [ { "code": "MATH140", "title": "Calculus I", "area": "MT" },
                   { "code": "CS110", "title": "Introduction to Computing", "area": "MT" } ] },
    { "name": "Arts and Humanities", "codes": ["AR", "HU"], "outcomes": [],
      "courses": [ { "code": "PHIL100", "title": "Introduction to Philosophy", "area": "HU" } ] }
  ],
  "distribution_rules": [
    { "program": "BS (CSM)", "areas": ["AR", "HU", "SB"], "min_courses": 3,
      "min_per_area": { "AR": 1, "HU": 1 }, "note": "" }
  ]
})json";

// ==================== loadCourses ====================

TEST(LoadCoursesTest, LoadsEveryList) {
    string cs = writeFile("cs.json", kCsJson), ge = writeFile("ge.json", kGenEdJson);
    Courses catalog;
    vector<string> errors;
    ASSERT_TRUE(loadCourses(cs, ge, catalog, errors));
    EXPECT_TRUE(errors.empty()) << errors[0];

    // CS110 CS210 CS285L ENGL101 ENGL102 MATH140 PHIL100
    EXPECT_EQ(catalog.size(), 7u);
    EXPECT_EQ(catalog.find("CS210")->category,  CourseCategory::CS);       // "cs 210" normalized
    EXPECT_EQ(catalog.find("CS285L")->category, CourseCategory::CS);
    EXPECT_EQ(catalog.find("ENGL101")->category, CourseCategory::GEN_ED);
    EXPECT_EQ(catalog.find("PHIL100")->category, CourseCategory::DISTRIBUTION);
    EXPECT_EQ(catalog.find("PHIL100")->distArea, "HU");
    EXPECT_EQ(catalog.find("PHIL100")->name, "Introduction to Philosophy");
}

TEST(LoadCoursesTest, MergesCoursesListedInBothFiles) {
    string cs = writeFile("cs.json", kCsJson), ge = writeFile("ge.json", kGenEdJson);
    Courses catalog;
    vector<string> errors;
    loadCourses(cs, ge, catalog, errors);

    const Course* math140 = catalog.find("MATH140");
    EXPECT_EQ(math140->name, "Calculus I");          // title from the gen-ed file
    EXPECT_EQ(math140->distArea, "MT");

    const Course* cs110 = catalog.find("CS110");
    EXPECT_EQ(cs110->category, CourseCategory::CS);  // stays CS
    EXPECT_EQ(cs110->distArea, "MT");                // but also counts toward MT
}

TEST(LoadCoursesTest, ReportsBadAndConflictingEntries) {
    string cs = writeFile("cs_bad.json", R"json({
      "cs_courses": ["CS110", "not a code", 42],
      "gen_ed_courses": [],
      "distribution_courses": [ { "code": "MATH140", "area": "MT" }, { "area": "NS" } ]
    })json");
    string ge = writeFile("ge_bad.json", R"json({
      "distribution_areas": [ { "courses": [ { "code": "MATH140", "title": "Calculus I", "area": "NS" } ] } ]
    })json");
    Courses catalog;
    vector<string> errors;
    EXPECT_TRUE(loadCourses(cs, ge, catalog, errors));   // files readable, so it still loads

    EXPECT_EQ(catalog.size(), 2u);                       // CS110, MATH140
    EXPECT_EQ(catalog.find("MATH140")->distArea, "MT");  // conflict did not overwrite
    EXPECT_EQ(errors.size(), 4u);                        // "not a code", 42, no code, MT vs NS
}

TEST(LoadCoursesTest, MissingFileFails) {
    string ge = writeFile("ge.json", kGenEdJson);
    Courses catalog;
    vector<string> errors;
    EXPECT_FALSE(loadCourses("/no/such/file.json", ge, catalog, errors));
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_NE(errors[0].find("cannot open"), string::npos);
    EXPECT_EQ(catalog.size(), 0u);
}

TEST(LoadCoursesTest, InvalidJsonFails) {
    string cs = writeFile("broken.json", "{ \"cs_courses\": [ \"CS110\", ");
    string ge = writeFile("ge.json", kGenEdJson);
    Courses catalog;
    vector<string> errors;
    EXPECT_FALSE(loadCourses(cs, ge, catalog, errors));
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_NE(errors[0].find("cannot parse"), string::npos);
}

TEST(LoadCoursesTest, RealFiles) {
    Courses catalog;
    vector<string> errors;
    ASSERT_TRUE(loadCourses(kRealCs, kRealGenEd, catalog, errors));
    EXPECT_TRUE(errors.empty()) << errors.size() << " errors, first: " << errors[0];

    EXPECT_GT(catalog.size(), 500u);
    EXPECT_EQ(catalog.find("CS110")->category, CourseCategory::CS);
    EXPECT_EQ(catalog.find("CS110")->distArea, "MT");
    EXPECT_EQ(catalog.find("MATH140")->name, "Calculus I");
    EXPECT_EQ(catalog.find("PHIL100")->distArea, "HU");
    EXPECT_EQ(catalog.inCategory(CourseCategory::CS).size(), 43u);
}

// ==================== loadGenEd ====================

TEST(LoadGenEdTest, LoadsRequirementsWithNormalizedCodes) {
    string ge = writeFile("ge.json", kGenEdJson);
    vector<string> errors;
    vector<GenEdRequirement> reqs = loadGenEd(ge, errors);
    EXPECT_TRUE(errors.empty());
    ASSERT_EQ(reqs.size(), 2u);

    EXPECT_EQ(reqs[0].phase, "first_year");
    EXPECT_EQ(reqs[0].text, "Writing");
    EXPECT_EQ(reqs[0].courses, (vector<string>{ "ENGL101", "ENGL102" }));   // "English 101" normalized
    EXPECT_TRUE(reqs[0].areas.empty());

    EXPECT_TRUE(reqs[1].courses.empty());
    EXPECT_EQ(reqs[1].areas, (vector<string>{ "AR", "HU" }));
}

TEST(LoadGenEdTest, SkipsBadEntries) {
    string ge = writeFile("ge_bad.json", R"json({ "gen_ed": [
      { "phase": "middle", "courses": ["ENGL101"] },
      { "phase": "middle", "text": "Writing", "courses": ["ENGL101", "???"] }
    ] })json");
    vector<string> errors;
    vector<GenEdRequirement> reqs = loadGenEd(ge, errors);
    ASSERT_EQ(reqs.size(), 1u);                                       // the one without text dropped
    EXPECT_EQ(reqs[0].courses, (vector<string>{ "ENGL101" }));        // bad code dropped
    EXPECT_EQ(errors.size(), 2u);
}

TEST(LoadGenEdTest, MissingFileGivesEmptyList) {
    vector<string> errors;
    EXPECT_TRUE(loadGenEd("/no/such/file.json", errors).empty());
    EXPECT_EQ(errors.size(), 1u);
}

TEST(LoadGenEdTest, RealFile) {
    vector<string> errors;
    vector<GenEdRequirement> reqs = loadGenEd(kRealGenEd, errors);
    EXPECT_TRUE(errors.empty());
    ASSERT_FALSE(reqs.empty());
    EXPECT_EQ(reqs[0].courses, (vector<string>{ "ENGL101", "ENGL102" }));
}

// ==================== loadDistributionRules ====================

TEST(LoadDistributionRulesTest, LoadsRule) {
    string ge = writeFile("ge.json", kGenEdJson);
    vector<string> errors;
    vector<DistributionRule> rules = loadDistributionRules(ge, errors);
    EXPECT_TRUE(errors.empty());
    ASSERT_EQ(rules.size(), 1u);

    EXPECT_EQ(rules[0].program, "BS (CSM)");
    EXPECT_EQ(rules[0].areas, (vector<string>{ "AR", "HU", "SB" }));
    EXPECT_EQ(rules[0].minCourses, 3);
    EXPECT_EQ(rules[0].minPerArea, (vector<pair<string, int>>{ { "AR", 1 }, { "HU", 1 } }));
}

TEST(LoadDistributionRulesTest, SkipsIncompleteRule) {
    string ge = writeFile("rules_bad.json", R"json({ "distribution_rules": [
      { "program": "BA", "areas": ["AR"] },
      { "areas": ["AR"], "min_courses": 2 },
      { "program": "BS", "areas": ["NS"], "min_courses": 2 }
    ] })json");
    vector<string> errors;
    vector<DistributionRule> rules = loadDistributionRules(ge, errors);
    ASSERT_EQ(rules.size(), 1u);
    EXPECT_EQ(rules[0].program, "BS");
    EXPECT_TRUE(rules[0].minPerArea.empty());
    EXPECT_EQ(errors.size(), 2u);
}

TEST(LoadDistributionRulesTest, RealFile) {
    vector<string> errors;
    vector<DistributionRule> rules = loadDistributionRules(kRealGenEd, errors);
    EXPECT_TRUE(errors.empty());
    EXPECT_EQ(rules.size(), 16u);
}

// ==================== checkLinks ====================

TEST(CheckLinksTest, AllCoursesFound) {
    string cs = writeFile("cs.json", kCsJson), ge = writeFile("ge.json", kGenEdJson);
    Courses catalog;
    vector<string> errors;
    loadCourses(cs, ge, catalog, errors);
    EXPECT_TRUE(checkLinks(catalog, loadGenEd(ge, errors), errors));
    EXPECT_TRUE(errors.empty());
}

TEST(CheckLinksTest, ReportsUnknownCourse) {
    Courses catalog;
    GenEdRequirement req;
    req.text    = "Writing";
    req.courses = { "ENGL101" };
    vector<string> errors;
    EXPECT_FALSE(checkLinks(catalog, { req }, errors));
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_NE(errors[0].find("ENGL101"), string::npos);
}

TEST(CheckLinksTest, RealFilesLinkUpExceptBC290) {
    // Known data gap: the writing proficiency requirement allows BC290 for College of
    // Management students, but the scraper only collects CS, gen-ed and distribution courses.
    // If this starts failing, the data changed: either BC290 was added or a new course is missing.
    Courses catalog;
    vector<string> errors;
    loadCourses(kRealCs, kRealGenEd, catalog, errors);
    ASSERT_TRUE(errors.empty());
    EXPECT_FALSE(checkLinks(catalog, loadGenEd(kRealGenEd, errors), errors));
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_NE(errors[0].find("BC290"), string::npos);
}

// ==================== loadCourseDetails ====================

static const char* kDetailsJson = R"json({
  "_meta": { "source": "test" },
  "courses": [
    { "code": "CS110", "category": "CS", "area": "MT", "title": "Introduction to Computing",
      "credits": 4, "prereqs": [], "coreqs": [], "note": "" },
    { "code": "CS210", "category": "CS", "area": "", "title": "Intermediate Computing",
      "credits": 4, "prereqs": ["cs 110"], "coreqs": [], "note": "CS110 or CS/IT 115L" },
    { "code": "MATH140", "category": "DISTRIBUTION", "area": "", "title": "Calculus I",
      "credits": 4, "prereqs": [], "coreqs": [], "note": "" },
    { "code": "CS341", "category": "CS", "area": "", "title": "Computer Architecture",
      "credits": 3, "prereqs": [], "coreqs": ["CS341L"], "note": "" },
    { "code": "CS341L", "category": "CS", "area": "", "title": "Computer Architecture Lab",
      "credits": 1, "prereqs": [], "coreqs": ["CS341"], "note": "" }
  ]
})json";

TEST(LoadCourseDetailsTest, FillsDetailsOnScrapedCourses) {
    string cs = writeFile("cs.json", kCsJson), ge = writeFile("ge.json", kGenEdJson);
    string details = writeFile("details.json", kDetailsJson);
    Courses catalog;
    vector<string> errors;
    loadCourses(cs, ge, catalog, errors);
    ASSERT_TRUE(loadCourseDetails(details, catalog, errors));
    EXPECT_TRUE(errors.empty()) << errors[0];

    const Course* cs210 = catalog.find("CS210");
    EXPECT_EQ(cs210->name, "Intermediate Computing");
    EXPECT_EQ(cs210->credits, 4);
    EXPECT_EQ(cs210->prereqs, (vector<string>{ "CS110" }));   // "cs 110" normalized
    EXPECT_EQ(cs210->note, "CS110 or CS/IT 115L");
}

TEST(LoadCourseDetailsTest, EmptyAreaKeepsScrapedArea) {
    // An empty "area" in the details file means "unknown", not "remove the area".
    string cs = writeFile("cs.json", kCsJson), ge = writeFile("ge.json", kGenEdJson);
    string details = writeFile("details.json", kDetailsJson);
    Courses catalog;
    vector<string> errors;
    loadCourses(cs, ge, catalog, errors);
    loadCourseDetails(details, catalog, errors);
    EXPECT_EQ(catalog.find("MATH140")->distArea, "MT");
    EXPECT_EQ(catalog.find("MATH140")->credits, 4);
}

TEST(LoadCourseDetailsTest, AddsCoursesOnlyListedThere) {
    string details = writeFile("details.json", kDetailsJson);
    Courses catalog;
    vector<string> errors;
    loadCourseDetails(details, catalog, errors);
    ASSERT_TRUE(catalog.contains("CS341L"));
    EXPECT_EQ(catalog.find("CS341L")->credits, 1);
    EXPECT_EQ(catalog.find("CS341")->coreqs, (vector<string>{ "CS341L" }));
}

TEST(LoadCourseDetailsTest, ReportsBadEntries) {
    string details = writeFile("details_bad.json", R"json({ "courses": [
      { "code": "CS110", "category": "MAJOR", "credits": 4 },
      { "code": "CS210", "category": "CS", "credits": "four" },
      { "code": "CS220", "category": "CS", "credits": -3 },
      { "code": "CS240", "category": "CS", "credits": 3 }
    ] })json");
    Courses catalog;
    vector<string> errors;
    EXPECT_TRUE(loadCourseDetails(details, catalog, errors));
    EXPECT_EQ(catalog.size(), 1u);          // only CS240 is valid
    EXPECT_EQ(errors.size(), 3u);
}

TEST(LoadCourseDetailsTest, ConflictWithScrapedDataIsReported) {
    string cs = writeFile("cs.json", kCsJson), ge = writeFile("ge.json", kGenEdJson);
    string details = writeFile("details_conflict.json", R"json({ "courses": [
      { "code": "MATH140", "category": "DISTRIBUTION", "area": "NS", "title": "Calculus I" }
    ] })json");
    Courses catalog;
    vector<string> errors;
    loadCourses(cs, ge, catalog, errors);
    loadCourseDetails(details, catalog, errors);
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_NE(errors[0].find("MATH140"), string::npos);
    EXPECT_EQ(catalog.find("MATH140")->distArea, "MT");   // scraped value kept
}

TEST(LoadCourseDetailsTest, MissingFileFails) {
    Courses catalog;
    vector<string> errors;
    EXPECT_FALSE(loadCourseDetails("/no/such/file.json", catalog, errors));
    EXPECT_EQ(errors.size(), 1u);
}

TEST(LoadCourseDetailsTest, RealFiles) {
    Courses catalog;
    vector<string> errors;
    loadCourses(kRealCs, kRealGenEd, catalog, errors);
    ASSERT_TRUE(loadCourseDetails(kRealDetails, catalog, errors));
    EXPECT_TRUE(errors.empty()) << errors.size() << " errors, first: " << errors[0];

    EXPECT_EQ(catalog.size(), 524u);   // 523 scraped + CS341L, only in the details file
    const Course* cs310 = catalog.find("CS310");
    EXPECT_EQ(cs310->name, "Advanced Data Structures and Algorithms");
    EXPECT_EQ(cs310->credits, 3);
    EXPECT_EQ(cs310->prereqs, (vector<string>{ "CS210", "CS220", "CS240" }));
    EXPECT_EQ(catalog.find("CS341")->coreqs, (vector<string>{ "CS341L" }));
    EXPECT_EQ(catalog.find("MATH345")->distArea, "MT");    // kept from the scraped file
    EXPECT_EQ(catalog.find("PHYSIC181")->distArea, "NS");
}

// ==================== checkCourseReferences ====================

static Course withPrereqs(const string& code, vector<string> prereqs, vector<string> coreqs = {}) {
    Course c;
    c.code     = code;
    c.category = CourseCategory::CS;
    c.prereqs  = prereqs;
    c.coreqs   = coreqs;
    return c;
}

TEST(CheckCourseReferencesTest, CleanCatalogPasses) {
    Courses catalog;
    catalog.add(withPrereqs("CS110", {}));
    catalog.add(withPrereqs("CS210", { "CS110" }));
    catalog.add(withPrereqs("CS341", {}, { "CS341L" }));
    catalog.add(withPrereqs("CS341L", {}, { "CS341" }));   // coreqs may point at each other
    vector<string> errors;
    EXPECT_TRUE(checkCourseReferences(catalog, errors));
    EXPECT_TRUE(errors.empty());
}

TEST(CheckCourseReferencesTest, ReportsDanglingPrereqAndCoreq) {
    Courses catalog;
    catalog.add(withPrereqs("CS644", { "CS641" }));
    catalog.add(withPrereqs("CS341", {}, { "CS341L" }));
    vector<string> errors;
    EXPECT_FALSE(checkCourseReferences(catalog, errors));
    ASSERT_EQ(errors.size(), 2u);
    // errors follow catalog order (sorted by code): CS341 before CS644
    EXPECT_NE(errors[0].find("CS341L"), string::npos);
    EXPECT_NE(errors[1].find("CS641"), string::npos);
}

TEST(CheckCourseReferencesTest, ReportsPrereqCycleOnce) {
    // CS210 -> CS310 -> CS410 -> CS210
    Courses catalog;
    catalog.add(withPrereqs("CS210", { "CS410" }));
    catalog.add(withPrereqs("CS310", { "CS210" }));
    catalog.add(withPrereqs("CS410", { "CS310" }));
    catalog.add(withPrereqs("CS110", {}));
    vector<string> errors;
    EXPECT_FALSE(checkCourseReferences(catalog, errors));
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_NE(errors[0].find("cycle"), string::npos);
}

TEST(CheckCourseReferencesTest, RealFilesAreClean) {
    Courses catalog;
    vector<string> errors;
    loadCourses(kRealCs, kRealGenEd, catalog, errors);
    loadCourseDetails(kRealDetails, catalog, errors);
    EXPECT_TRUE(checkCourseReferences(catalog, errors));
    EXPECT_TRUE(errors.empty()) << errors[0];
}
