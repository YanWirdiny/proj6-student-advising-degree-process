#include "seeds.h"
#include "fetcher.h"
#include "parser.h"
#include "extractor_storage.h"
#include "requirements.h"

int main() {
    Fetcher               fetcher;
    HtmlParser            parser;
    CourseExtractor       extractor;
    GenEdExtractor        genEdExtractor;
    DistributionExtractor distExtractor;
    DistributionCourseExtractor distCourseExtractor;
    CourseStorage         storage("cs_courses.json");
    RequirementStorage    reqStorage("gen_ed_requirements.json");

    for (const Seed& seed : getSeeds()) {
        switch (seed.category) {
        case SeedCategory::CS_COURSE: {
            std::string html = fetcher.fetch(seed.url);
            storage.add(extractor.extract(parser.extractText(html)));
            break;
        }
        case SeedCategory::GEN_ED: {
            std::string html = fetcher.fetch(seed.url);
            storage.add(extractor.extract(parser.extractText(html)));   // ENGL101, ENGL102
            reqStorage.addGenEd(genEdExtractor.extract(parser.extractBlocks(html)));
            break;
        }
        case SeedCategory::DISTRIBUTION:
            reqStorage.addAreas(distExtractor.extract(parser.extractBlocks(fetcher.fetch(seed.url))));
            break;
        case SeedCategory::DISTRIBUTION_COURSES:
            // Follow &page=N until a page comes back with no courses (cap guards against a loop).
            for (int page = 1; page <= 50; ++page) {
                std::string html    = fetcher.fetch(seed.url + "&page=" + std::to_string(page));
                auto        courses = distCourseExtractor.extract(parser.extractBlocks(html), seed.area);
                if (courses.empty()) break;
                reqStorage.addDistributionCourses(courses);
            }
            break;
        default:
            break;   // CATALOG: blocks non-browser clients, not crawled yet
        }
    }

    storage.save();
    reqStorage.save();
    return 0;
}
