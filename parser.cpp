#include "parser.h"

std::string HtmlParser::extractText(const std::string& html) const {
    std::string text;
    text.reserve(html.size());
    bool inTag = false;
    for (char c : html) {
        if      (c == '<') { inTag = true;  text += ' '; }
        else if (c == '>') { inTag = false; text += ' '; }
        else if (!inTag)   { text += c; }
    }
    return text;
}
