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

    std::vector<Seed> seeds = getSeeds();
    std::cout << "Crawling " << seeds.size() << " seeds...\n";

    for (const Seed& seed : seeds) {
        switch (seed.category) {
        case SeedCategory::CS_COURSE: {
            std::string html    = fetcher.fetch(seed.url);
            auto        courses = extractor.extract(parser.extractText(html));
            storage.add(courses);
            std::cout << "[CS]           " << courses.size() << " courses  <- " << seed.url << "\n";
            break;
        }
        case SeedCategory::GEN_ED: {
            std::string html    = fetcher.fetch(seed.url);
            auto        courses = extractor.extract(parser.extractText(html));   // ENGL101, ENGL102
            auto        reqs    = genEdExtractor.extract(parser.extractBlocks(html));
            storage.add(courses);
            reqStorage.addGenEd(reqs);
            std::cout << "[GEN_ED]       " << courses.size() << " courses, " << reqs.size()
                      << " requirements  <- " << seed.url << "\n";
            break;
        }
        case SeedCategory::DISTRIBUTION: {
            auto areas = distExtractor.extract(parser.extractBlocks(fetcher.fetch(seed.url)));
            reqStorage.addAreas(areas);
            std::cout << "[DISTRIBUTION] " << areas.size() << " areas  <- " << seed.url << "\n";
            break;
        }
        case SeedCategory::DISTRIBUTION_COURSES: {
            // Follow &page=N until a page comes back with no courses (cap guards against a loop).
            size_t total = 0;
            int    page  = 1;
            for (; page <= 50; ++page) {
                std::string html    = fetcher.fetch(seed.url + "&page=" + std::to_string(page));
                auto        courses = distCourseExtractor.extract(parser.extractBlocks(html), seed.area);
                if (courses.empty()) break;
                reqStorage.addDistributionCourses(courses);
                total += courses.size();
            }
            std::cout << "[" << seed.area << "]           " << total << " courses over "
                      << page - 1 << " page(s)  <- " << seed.url << "\n";
            break;
        }
        default:
            std::cout << "[SKIPPED]      " << seed.url << "\n";
            break;   // CATALOG: blocks non-browser clients, not crawled yet
        }
    }

    storage.save();
    reqStorage.save();
    std::cout << "Saved cs_courses.json and gen_ed_requirements.json\n";
    return 0;
}
