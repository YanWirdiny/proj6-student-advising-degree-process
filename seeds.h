#pragma once
#include <iostream>
#include <vector>
using namespace std ;

enum class SeedCategory {
// data of seeds to figure out
    CS_COURSE,
    CATALOG,
    GEN_ED, 
    DISTRIBUTION
    

};

struct Seed {
    string url;
    SeedCategory category ;
};

std::vector<Seed> getSeeds();

