#include "seeds.h"

// courses.umb.edu filters by area name, not code. Values are URL-encoded.
static const pair<const char*, const char*> kGenEdFilters[] = {
    { "AR", "The%20Arts" },
    { "HU", "Humanities" },
    { "MT", "Mathematics%20and%20Technology" },
    { "NS", "Natural%20Sciences" },
    { "SB", "Social%20%26%20Behavioral%20Sciences" },
    { "WC", "World%20Cultures" },
    { "WL", "World%20Languages" },
};

// Search results only list courses offered in that term, so regular semesters are combined.
static const char* kTerms[] = { "fall-2026", "spring-2027" };

std::vector<Seed> getSeeds() {
    std::vector<Seed> seeds = {
        { "https://cs.umb.edu/cs_bs",                                                                                        SeedCategory::CS_COURSE    },
        { "https://catalog.umb.edu/content.php?catoid=68&navoid=13738",                                                      SeedCategory::CATALOG       },
        { "https://www.umb.edu/academics/seas/undergraduate-studies/general-education-requirements/",                        SeedCategory::GEN_ED        },
        { "https://www.umb.edu/academics/seas/undergraduate-studies/general-education-requirements/distribution-requirement/", SeedCategory::DISTRIBUTION },
        // GEN_ED_TRANSFER: URL not yet identified — transfer students have different Gen Ed rules
        // PREREQUISITES:   No known crawlable source; relationship rules are not published centrally
    };
    for (const char* term : kTerms)
        for (const auto& f : kGenEdFilters)
            seeds.push_back({ string("https://courses.umb.edu/search/") + term + "/?courseGenEd=" + f.second
                                  + "&courseStudyLevel=Undergraduate",
                              SeedCategory::DISTRIBUTION_COURSES, f.first });
    return seeds;
}
