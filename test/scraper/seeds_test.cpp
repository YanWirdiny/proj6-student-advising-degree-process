#include <gtest/gtest.h>
#include <map>
#include <set>
#include "seeds.h"

// The four fixed page seeds come first, in the order main.cpp processes them.
TEST(SeedsTest, FixedSeedsHaveExpectedCategories) {
    vector<Seed> seeds = getSeeds();
    ASSERT_GE(seeds.size(), 4u);

    EXPECT_EQ(seeds[0].category, SeedCategory::CS_COURSE);
    EXPECT_EQ(seeds[1].category, SeedCategory::CATALOG);
    EXPECT_EQ(seeds[2].category, SeedCategory::GEN_ED);
    EXPECT_EQ(seeds[3].category, SeedCategory::DISTRIBUTION);

    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(seeds[i].url.rfind("https://", 0), 0u) << seeds[i].url;
        EXPECT_TRUE(seeds[i].area.empty()) << seeds[i].url;
    }
}


