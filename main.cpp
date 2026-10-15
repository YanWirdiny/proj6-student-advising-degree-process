#include "seeds.h"
#include "fetcher.h"
#include "parser.h"
#include "extractor_storage.h"

int main() {
    Fetcher        fetcher;
    HtmlParser     parser;
    CourseExtractor extractor;
    CourseStorage  storage("cs_courses.json");

    for (const Seed& seed : getSeeds()) {
        if (seed.category != SeedCategory::CS_COURSE) continue;

        std::string html    = fetcher.fetch(seed.url);
        std::string text    = parser.extractText(html);
        auto        courses = extractor.extract(text);
        storage.add(courses);
    }

    storage.save();
    return 0;
}
