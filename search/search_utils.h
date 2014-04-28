#ifndef _SEARCH_UTILS_H_
#define _SEARCH_UTILS_H_

#include <fstream>
#include <string>
#include <cstdio>
#include <ctime>
#include <set>

class TextSplitter {
public:

    TextSplitter(std::istream &input):
        stream_(input)
    { 
        readAbbreviations();
    }

    void getSentences(std::ostream &out, size_t &count);

private:
    enum {
        MIN_SENTENCE_LEN = 2  // minimal sentence len in words
    };
    std::istream &stream_;
    std::set<std::string> abbreviations_;    

    static std::string removeStartingSpaces(const std::string &s) {
        size_t off = 0;
        const char *ptr = s.c_str();
        while (ptr[off] == ' ') { ++off; }
        return s.substr(off, s.length() - off);
    }

    static bool isSentenceDelimiter(char c) {
        return c == '.' || c == '!' || c == '?';
    }
    
    static bool isWordDelimiter(char c) {
        return c == ' ' || c == ',' || c == ';' || c == '\t'; 
    }

    static bool isCRLF(char c) {
        return c == '\n' || c == '\r';
    }
    
    bool isAbbreviation(const std::string &s) {
        return abbreviations_.find(s) != abbreviations_.end();
    }

    void readAbbreviations();
};

class Timer { 
public:
    
    Timer(bool start = true) {
        if (start) {
            start_ = std::clock();
        }
    }

    double elapsed() const {
        return double(std::clock() - start_) / CLOCKS_PER_SEC;
    }

    void restart() { start_ = std::clock(); }

private:
    std::clock_t start_;
};

#endif /* _SEARCH_UTILS_H_ */
