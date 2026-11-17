add_test([=[SeedsTest.FixedSeedsHaveExpectedCategories]=]  C:/Users/drawd/proj6-student-advising-degree-process/build/seeds_test.exe [==[--gtest_filter=SeedsTest.FixedSeedsHaveExpectedCategories]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SeedsTest.FixedSeedsHaveExpectedCategories]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:/Users/drawd/proj6-student-advising-degree-process/test/seeds_test.cpp:7]==]
    WORKING_DIRECTORY [==[C:/Users/drawd/proj6-student-advising-degree-process/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SeedsTest.DistributionCourseSeedsCoverEveryAreaAndTerm]=]  C:/Users/drawd/proj6-student-advising-degree-process/build/seeds_test.exe [==[--gtest_filter=SeedsTest.DistributionCourseSeedsCoverEveryAreaAndTerm]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SeedsTest.DistributionCourseSeedsCoverEveryAreaAndTerm]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[C:/Users/drawd/proj6-student-advising-degree-process/test/seeds_test.cpp:23]==]
    WORKING_DIRECTORY [==[C:/Users/drawd/proj6-student-advising-degree-process/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(seeds_test_TESTS [==[SeedsTest.FixedSeedsHaveExpectedCategories]==] [==[SeedsTest.DistributionCourseSeedsCoverEveryAreaAndTerm]==])
