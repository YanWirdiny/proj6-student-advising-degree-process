#pragma once
#include <string>
#include <vector>

// One text-bearing element of a page (heading, paragraph or list item).
struct HtmlBlock {
    std::string tag;    // "h2", "h3", "p", "li", ...
    std::string text;   // entity-decoded, whitespace-collapsed
};

class HtmlParser {
public:
    std::string extractText(const std::string& html) const;

    // Keeps document structure: returns headings/paragraphs/list items in order,
    // limited to the <main> region when present. <br> inside an element starts a new block.
    std::vector<HtmlBlock> extractBlocks(const std::string& html) const;
};
