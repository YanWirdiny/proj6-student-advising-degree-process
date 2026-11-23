#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std ;

enum class SeedCategory {
// data of seeds to figure out
    CS_COURSE,
    CATALOG,
    GEN_ED,
    DISTRIBUTION,
    DISTRIBUTION_COURSES   // paginated course search filtered by one distribution area
};

struct Seed {
    string url;
    SeedCategory category ;
    string area = "";      // distribution code ("AR", "HU", ...) for DISTRIBUTION_COURSES
};

vector<Seed> getSeeds();

