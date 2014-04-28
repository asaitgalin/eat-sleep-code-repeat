#ifndef _SEARCH_ENGINE_H_
#define _SEARCH_ENGINE_H_

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <map>
#include <stdexcept> 
#include <fstream>
#include <cmath>
#include <set>

#include <boost/tokenizer.hpp>
#include "search_stemmer.h"
#include "search_utils.h"

const size_t DISPLAY_LIMIT = 10; // max sentence count to be displayed (-1 to display all)
                
class SearchTrie {
// Simple trie. Used to store words with info
public:
    typedef unsigned char _uChar;
   
    /* Subclass trie Node */
    
    class Node {
        typedef std::map<size_t, size_t>::const_iterator _Iterator;
    public:
        
        ~Node() {
            for (std::map<_uChar, Node *>::const_iterator it = children_.begin(); 
                it != children_.end(); ++it) {
                if (it->second != NULL) {
                    delete it->second;
                }
            }
        }

        Node *getChild(_uChar letter, bool createNew = false) {
            if (children_.find(letter) == children_.end()) {
                if (!createNew) {
                    return NULL;
                }
                children_.insert(std::make_pair(letter, new Node()));
            }
            return children_[letter];
        }
      
        void incrementTermFreq(size_t sentenceId) {
            std::map<size_t, size_t>::iterator it = sentences_.find(sentenceId);
            if (it == sentences_.end()) {
                sentences_[sentenceId] = 1;
            } else {
                it->second++;
            }
        }

        size_t getTermFreq(size_t sentenceId) const {
            std::map<size_t, size_t>::const_iterator it = sentences_.find(sentenceId);
            if (it == sentences_.end()) {
                return 0;
            } else {
                return it->second;
            }
        }

        size_t getSentencesCount() const { return sentences_.size(); }
        _Iterator sentenceIterBegin() const { return sentences_.begin(); }
        _Iterator sentenceIterEnd() const { return sentences_.end(); }
        
    private:
        std::map<_uChar, Node *> children_;
        std::map<size_t, size_t> sentences_; // Document -> term frequency (can be zero)
    };

    /* SearchTrie */

    SearchTrie():
        root_(new Node),
        size_(0)
    { }
   
    ~SearchTrie() {
        delete root_;
    }

    void addWord(const std::string &s, size_t sentenceId);
    Node *findWord(const std::string &s) const;
    size_t getSize() const { return size_; }

private:
    Node *root_;
    size_t size_; // Behaves like total word count (not only unique count)
};

class SearchIndexer {
// Indexer. Splits input file to sentences, prepares index file and fills Trie with words
public:
   
    SearchIndexer(const std::string &fileName):
        inputStream_(fileName.c_str(), std::ios_base::in)
    { }

    void prepareIndex();
    std::string getSentenceById(size_t id);
 
    size_t getSentenceWordCount(size_t id) const {
        std::map<size_t, size_t>::const_iterator it = wordCount_.find(id);
        if (it == wordCount_.end()) {
            throw std::runtime_error("indexer: failed to get sentence word count. Document not found");
        }
        return it->second;
    }
      
    // Total sentences count
    size_t getSentenceCount() const { return sentenceCount_; }
    // Average word count in each sentence (for BM25)
    float getAvgWordCount() const { return averageWordCount_; }
    const SearchTrie &getTrie() const { return wordsTrie_; } 

private:
    std::fstream inputStream_; // Input file
    size_t sentenceCount_; // Total sentences count
    float averageWordCount_; 

    SearchTrie wordsTrie_;    
    std::map<size_t, size_t> wordCount_; // Word count in each sentence
    std::fstream indexStream_; // Index file
    std::vector<std::streampos> offsets_; // Offset for each sentence in index file

    void splitSourceText();
};

class SentencesComparerByScore {
// Compares sentences by their score from scores map
public:
 
    SentencesComparerByScore(const std::map<size_t, float> &scores):
        scores_(scores)
    { }

    bool operator() (size_t i, size_t j) const {
        std::map<size_t, float>::const_iterator pi = scores_.find(i);
        std::map<size_t, float>::const_iterator pj = scores_.find(j);
        if (pi == scores_.end() || pj == scores_.end()) {
            throw std::runtime_error("sentences comparer: document was not found in scores map");
        }
        return *pi < *pj;
    }

private:
    const std::map<size_t, float > &scores_;
};

class SearchEngine {
public:

    SearchEngine(const std::string &file, std::ostream &out, std::ostream &err):
        stemmer_(new SnowballStemmer),
        indexer_(file),
        outStream_(out),
        errStream_(err)
    { } 

    void init();
    void processRequest(const std::string &req);
    
private:
    CustomStemmer *stemmer_;
    SearchIndexer indexer_;
    std::ostream &outStream_;
    std::ostream &errStream_;
};

#endif /* _SEARCH_ENGINE_H_ */

