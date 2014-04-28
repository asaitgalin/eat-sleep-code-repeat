#include "search_utils.h"

const char *abbreviationsFile = "abbr.txt";

/* TextSplitter */

void TextSplitter::getSentences(std::ostream &out, size_t &count) {
    std::string buffer;
    std::string lastWord; // Last read word
    size_t wordCount = 0; // Count words in each sentence
    char c;
    count = 0;
    while ((c = stream_.get()) != EOF) {
        // If we reached sentence delimiter (.?!) we should:
        // 1. Skip other delimiters (situations like '??!')
        // 2. Check if last word was abbreviation (then do not split by dot)
        // 3. Check for closing quote (do not move closing quote to next sentence)
        if (isSentenceDelimiter(c)) { 
            // Skip abbreviations
            if (c == '.' && isAbbreviation(lastWord)) {
                lastWord.clear();
                ++wordCount;
                buffer.push_back(c);
                continue;
            }
            if (!lastWord.empty()) { ++wordCount; }
            while (isSentenceDelimiter(c)) {
                buffer.push_back(c);
                c = stream_.get();
            }
            while (c == ' ') { c = stream_.get(); }  
            if (c == '"') {
                buffer.push_back(c);
                c = stream_.get();
            }
            if (wordCount >= MIN_SENTENCE_LEN) {
                out << removeStartingSpaces(buffer) << std::endl;
                wordCount = 0;
                ++count;
            }
            buffer.clear(); // clear sentence buffer
            lastWord.clear(); // clear word buffer
        } 
        // If we reached '\n' we will merge lines by space
        if (isCRLF(c)) {
            while (isCRLF(c)) { c = stream_.get(); }
            if (!buffer.empty()) {
                buffer.push_back(' ');
            }
            if (!lastWord.empty()) {
                ++wordCount;
                lastWord.clear();
            }
        }
        // Clear last word if we found word delimiter
        if (isWordDelimiter(c)) {
            if (!lastWord.empty()) {
                ++wordCount;
                lastWord.clear();
            }
        }
        if (c != EOF) {
            buffer.push_back(c);
        }
        if (!isWordDelimiter(c)) {
            lastWord.push_back(c);
        }
    }
}

void TextSplitter::readAbbreviations() {
    std::fstream f(abbreviationsFile);
    if (f) { // if no file - oke, we will do without it
        std::string s;
        while (std::getline(f, s)) {
            if (!s.empty()) {
                abbreviations_.insert(s);
            }
        }
    }
}

