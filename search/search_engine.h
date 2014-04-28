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

const size_t DISPLAY_LIMIT = 10; // max sentence count to be displayed
                
class SearchTrie {
public:
    typedef unsigned char _uChar;
   
    /* Subclass trie Node */
    
    class Node {
        typedef std::map<size_t, size_t>::const_iterator _iterator;
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
        _iterator sentenceIterBegin() const { return sentences_.begin(); }
        _iterator sentenceIterEnd() const { return sentences_.end(); }
        
    private:
        std::map<_uChar, Node *> children_;
        std::map<size_t, size_t> sentences_; // document -> term frequency (can be zero)
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
    size_t size_; // behaves like total word count (not only unique count)
};

class SearchIndexer {
public:
   
    SearchIndexer(const std::string &fileName):
        inputStream_(fileName.c_str(), std::ios_base::in)
    { }

    void prepareIndex();
    std::string getSentenceById(size_t id);

    size_t getSentenceWordCount(size_t id) const {
        std::map<size_t, size_t>::const_iterator it = wordCount_.find(id);
        if (it == wordCount_.end()) {
            throw std::runtime_error("indexer: failed to get sentence word count");
        }
        return it->second;
    }
       
    size_t getSentenceCount() const { return sentenceCount_; }
    float getAvgWordCount() const { return averageWordCount_; }
    const SearchTrie &getTrie() const { return wordsTrie_; } 

private:
    std::fstream inputStream_;
    size_t sentenceCount_; // total sentences count
    float averageWordCount_; 

    SearchTrie wordsTrie_;    
    std::map<size_t, size_t> wordCount_; // word count in each sentence
    std::fstream indexStream_;
    std::vector<std::streampos> offsets_; // offset for each sentence in index file

    void splitSourceText();
};

class SentencesComparerByScore {
public:
 
    SentencesComparerByScore(const std::map<size_t, float > &scores):
        scores_(scores)
    { }

    bool operator() (size_t i, size_t j) const {
        std::map<size_t, float>::const_iterator pi = scores_.find(i);
        std::map<size_t, float>::const_iterator pj = scores_.find(j);
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

