#include "parser.h"
#include <cctype>
#include <set>

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

// Decodes the handful of entities the UMB pages use and collapses whitespace.
static std::string cleanText(const std::string& raw) {
    static const std::pair<const char*, const char*> kEntities[] = {
        { "&nbsp;", " " }, { "&amp;", "&" }, { "&ldquo;", "\"" }, { "&rdquo;", "\"" },
        { "&lsquo;", "'" }, { "&rsquo;", "'" }, { "&quot;", "\"" }, { "&bull;", "" },
        { "&ndash;", "-" }, { "&mdash;", "-" }, { "&lt;", "<" }, { "&gt;", ">" },
        { "\xC2\xA0", " " },   // UTF-8 non-breaking space
    };
    std::string s = raw;
    for (const auto& e : kEntities) {
        std::string from = e.first, to = e.second;
        for (size_t pos = s.find(from); pos != std::string::npos; pos = s.find(from, pos + to.size()))
            s.replace(pos, from.size(), to);
    }
    std::string out;
    for (char c : s) {
        bool space = std::isspace(static_cast<unsigned char>(c));
        if (space && (out.empty() || out.back() == ' ')) continue;
        out += space ? ' ' : c;
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

std::vector<HtmlBlock> HtmlParser::extractBlocks(const std::string& html) const {
    static const std::set<std::string> kBlockTags = { "h1", "h2", "h3", "h4", "p", "li" };

    size_t start = html.find("<main");
    size_t stop  = html.find("</main>");
    if (start == std::string::npos) start = 0;
    if (stop  == std::string::npos) stop  = html.size();

    std::vector<HtmlBlock> blocks;
    std::string currentTag, buffer;
    auto flush = [&]() {
        std::string text = cleanText(buffer);
        if (!currentTag.empty() && !text.empty())
            blocks.push_back({ currentTag, text });
        buffer.clear();
    };

    size_t i = start;
    while (i < stop) {
        if (html.compare(i, 4, "<!--") == 0) {                 // comments can hide markup
            size_t end = html.find("-->", i);
            i = (end == std::string::npos) ? stop : end + 3;
            continue;
        }
        if (html[i] != '<') { buffer += html[i++]; continue; }

        size_t end = html.find('>', i);
        if (end == std::string::npos) break;
        std::string tag = html.substr(i + 1, end - i - 1);
        i = end + 1;

        bool closing = !tag.empty() && tag[0] == '/';
        std::string name;
        for (size_t k = closing ? 1 : 0; k < tag.size() && std::isalnum(static_cast<unsigned char>(tag[k])); ++k)
            name += static_cast<char>(std::tolower(static_cast<unsigned char>(tag[k])));

        if (name == "script" || name == "style") {
            if (!closing) {
                size_t close = html.find("</" + name, i);
                i = (close == std::string::npos) ? stop : close;
            }
            continue;
        }
        if (name == "br") { std::string keep = currentTag; flush(); currentTag = keep; continue; }
        if (kBlockTags.count(name)) {
            flush();
            currentTag = closing ? "" : name;
            continue;
        }
        buffer += ' ';                                          // inline tag boundary
    }
    flush();
    return blocks;
}
