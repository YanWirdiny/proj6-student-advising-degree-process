#include "seeds.h"

std::vector<Seed> getSeeds() {
    return {
        { "https://cs.umb.edu/cs_bs",                                                                                        SeedCategory::CS_COURSE    },
        { "https://catalog.umb.edu/content.php?catoid=68&navoid=13738",                                                      SeedCategory::CATALOG       },
        { "https://www.umb.edu/academics/seas/undergraduate-studies/general-education-requirements/",                        SeedCategory::GEN_ED        },
        { "https://www.umb.edu/academics/seas/undergraduate-studies/general-education-requirements/distribution-requirement/", SeedCategory::DISTRIBUTION },
        // GEN_ED_TRANSFER: URL not yet identified — transfer students have different Gen Ed rules
        // PREREQUISITES:   No known crawlable source; relationship rules are not published centrally
        // DISTRIBUTION per-major: department-defined, no single crawlable page exists
    };
}
