#include "search_engine.h"

const char *indexFileName = "index.dat";

/* SearchTrie */

void SearchTrie::addWord(const std::string &s, size_t sentenceId) {
    Node *current = root_;
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        _uChar letter = *it;
        current = current->getChild(letter, true);
    }
    current->incrementTermFreq(sentenceId);
    ++size_;
}

SearchTrie::Node * SearchTrie::findWord(const std::string &s) const {
    Node *current = root_;
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        _uChar letter = *it;
        if (!current->getChild(letter)) {
            return NULL; // maybe add some autocompletion?
        } else {
            current = current->getChild(letter);
        }
    }
    return current;
}

/* SearchIndexer */

void SearchIndexer::prepareIndex() { 
    splitSourceText();
    size_t i = 0;
    std::string line;
    offsets_.push_back(0);
    CustomStemmer *stemmer = new SnowballStemmer; 
    while (std::getline(indexStream_, line)) {
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        boost::tokenizer<> tokenizer(line);
        std::pair<std::map<size_t, size_t>::iterator, bool> p = wordCount_.insert(std::make_pair(i, 0));
        for (boost::tokenizer<>::iterator it = tokenizer.begin(); it != tokenizer.end(); ++it) {
            std::string stem = stemmer->getStem(*it);
            if (stem.length() > 1) {
                wordsTrie_.addWord(stem, i);
                p.first->second++;
            }
        }
        ++i;
        offsets_.push_back(indexStream_.tellg());
    }
    averageWordCount_ = (float)wordsTrie_.getSize() / (float)sentenceCount_;
    delete stemmer;
}

std::string SearchIndexer::getSentenceById(size_t id) {
    std::streampos pos = offsets_.at(id);
    indexStream_.clear();
    indexStream_.seekg(pos, indexStream_.beg);
    std::string line;
    std::getline(indexStream_, line);
    return line;
}

void SearchIndexer::splitSourceText() {
    TextSplitter splitter(inputStream_);
    indexStream_.open(indexFileName, std::ios_base::out | std::ios_base::in | std::ios_base::trunc);
    if (!indexStream_) {
        std::string msg = "indexer: failed to open " + std::string(indexFileName) + " for writing";
        throw std::runtime_error(msg);    
    }
    splitter.getSentences(indexStream_, sentenceCount_);
    indexStream_.seekg(0, indexStream_.beg); 
    inputStream_.close();
}

/* SearchEngine */

void SearchEngine::init() {
    outStream_ << "Analyzing text..." << std::endl;
    Timer timer;
    try {
        indexer_.prepareIndex();
    } catch (const std::runtime_error &e) {
        errStream_ << "engine: failed to analyze text: " << e.what() << std::endl;
        std::terminate();
    }
    outStream_ << "Done. Processing time: " << timer.elapsed() << std::endl;
    outStream_ << "Now you can write requests" << std::endl;
}

void SearchEngine::processRequest(const std::string &req) {
    boost::tokenizer<> tok(req);

    std::vector<const SearchTrie::Node *> stats; // frequencies for each word
    std::map<size_t, size_t> sentenceToCount; // stores how much words from query has sentence 
    std::set<size_t> sentences; // all sentences that contain some of query words

    for (boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it) {
        std::string s = *it;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        const SearchTrie::Node *node = indexer_.getTrie().findWord(stemmer_->getStem(s));
        stats.push_back(node);
        if (node) {
            for (std::map<size_t, size_t>::const_iterator docIter = node->sentenceIterBegin(); 
                    docIter != node->sentenceIterEnd(); ++docIter) {
                    sentences.insert(docIter->first);
                    if (sentenceToCount.find(docIter->first) == sentenceToCount.end()) {
                        sentenceToCount[docIter->first] = 1;
                    } else {
                        sentenceToCount[docIter->first] += 1;
                    }
            }
        }
    }
   
    // Documents with highest number of words from query will be shown before others
    std::map<size_t, std::vector<size_t> > countToSentences; // words count from query to document
    for (std::map<size_t, size_t>::const_iterator it = sentenceToCount.begin(); it != sentenceToCount.end(); ++it) {
        countToSentences[it->second].push_back(it->first);
    }

    // Documents scores calculation (Okapi BM25)
    std::map<size_t, float> sentenceToScore;
    for (std::set<size_t, size_t>::const_iterator it = sentences.begin(); it != sentences.end(); ++it) {
        float score = 0.0f;
        for (size_t i = 0; i < stats.size(); ++i) {
            if (stats[i]) { // if any document contain word
                size_t sentenceWC = indexer_.getSentenceWordCount(*it);
                float idf = std::log((float)(indexer_.getSentenceCount()) / (float)stats[i]->getSentencesCount());
                float tf = (float)stats[i]->getTermFreq(*it) / (float)sentenceWC; 
                score += (idf * 2.5f * tf) / (tf + 1.5f * (0.25 + 0.75 * ((float)sentenceWC / indexer_.getAvgWordCount())));
            }
        }
        sentenceToScore[*it] = score;
    }

    if (countToSentences.empty()) {
        outStream_ << "No sentences match your query" << std::endl;
        return;
    }
   
    size_t count = 0;
    SentencesComparerByScore comparer(sentenceToScore);
    // Printing results
    for (std::map<size_t, std::vector<size_t> >::reverse_iterator it = countToSentences.rbegin(); 
        it != countToSentences.rend(); ++it) {
            std::sort(it->second.begin(), it->second.end(), comparer); 

            for (size_t i = 0; i < it->second.size(); ++i, ++count) {
                if (count == DISPLAY_LIMIT) {
                    break;
                }
                outStream_ << count + 1 << ". " << indexer_.getSentenceById(it->second[i]) << std::endl;
            }
           
    }
}

