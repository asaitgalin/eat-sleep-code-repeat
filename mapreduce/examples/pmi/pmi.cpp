#include <cstdlib>
#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <mapreduce/mapreduce.hpp>

struct InputData {
    std::unordered_map<std::string, size_t> *counts;
    size_t totalSentenceCount;
};

std::string transformToLower(const std::string &s) {
    std::string result(s.length(), '\0');
    std::transform(s.cbegin(), s.cend(), result.begin(), ::tolower);
    return result;
}

// Counting words count for p(x) and p(y)
class WCMapper: public MapReduce::Mapper {
public:
    virtual void operator() (const std::string &key, const std::string &value) {
        boost::tokenizer<> tok(value);
        for (auto it = tok.begin(); it != tok.end(); ++it) {
            emitIntermediate(transformToLower(*it), "1");
        }
    }

    static std::string getName() {
        return "WCMapper";
    }
};

REGISTER_MAPPER(WCMapper)

class WCReducer: public MapReduce::Reducer {
public:
    virtual void operator() (const std::string &key, const ValueVector &values) {
        size_t occurences = 0;
        for (size_t i = 0; i < values.size(); ++i) {
            occurences += atoi(values[i].c_str());
        }
        emit(key, std::to_string(occurences));
    }

    static std::string getName() {
        return "WCReducer";
    }
};

REGISTER_REDUCER(WCReducer)

// Counting word pairs PMI
class PMIMapper: public MapReduce::Mapper {
public:
    virtual void operator() (const std::string &key, const std::string &value) {
        boost::tokenizer<> tok(value);
        std::vector<std::string> words;
        for (auto it = tok.begin(); it != tok.end(); ++it) {
            words.push_back(*it);
        }
        for (size_t i = 0; i < words.size(); ++i) {
            if (i + 1 < words.size()) {
                emitIntermediate(transformToLower(words[i]) + " " + transformToLower(words[i + 1]), "1");
            }
        }
    }
    
    static std::string getName() {
        return "PMIMapper";
    }
};

REGISTER_MAPPER(PMIMapper)

class PMIReducer: public MapReduce::Reducer {
public:
    virtual void operator() (const std::string &key, const ValueVector &values) {
        size_t occurences = 0;
        for (size_t i = 0; i < values.size(); ++i) {
            occurences += atoi(values[i].c_str());
        }
        InputData *data = (InputData *)getUserData();
        std::stringstream ss(key); 
        std::string first, second;
        ss >> first; ss >> second;
        double jointProb = (float) occurences / data->totalSentenceCount;
        double firstProb = (float)((*data->counts)[first]) / data->totalSentenceCount;
        double secondProb = (float)((*data->counts)[second]) / data->totalSentenceCount;
        double pmi = std::log10(jointProb / (firstProb * secondProb));
        double npmi = pmi / (-log10(jointProb));
        emit(key, std::to_string(npmi));
    }

    static std::string getName() {
        return "PMIReducer";
    }
};

REGISTER_REDUCER(PMIReducer)

size_t readSentences(const std::string &fileName, std::vector<std::pair<std::string, std::string>> &out) {
    std::fstream f(fileName, std::ios_base::in);
    if (!f) {
        std::cerr << "Failed to open input file.\n" << std::endl;
        exit(-1);
    }
    std::string line;
    size_t counter = 0;
    while (std::getline(f, line)) { 
        if (!line.empty()) {
            out.emplace_back(boost::lexical_cast<std::string>(counter), line);
        }
        ++counter;
    }
    return counter;
}

void writeOutput(const MapReduce::RecordVector &vector) {
    std::fstream f("out.txt", std::ios_base::out);
    for (auto &it : vector) {
        f << it.getKey() << " -> " << it.getValue() << std::endl;
    }
}

void convertToUnorderedSet(const MapReduce::RecordVector &results, std::unordered_map<std::string, size_t> &out) {
    for (const auto &it : results) {
        out[it.getKey()] = boost::lexical_cast<size_t>(it.getValue());
    }
}
        
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Wrong arguments. Usage: pmi text.txt" << std::endl;
        exit(-1);
    }
    std::vector<std::pair<std::string, std::string>> sentences;
    size_t count = readSentences(argv[1], sentences);
    // Count word counts
    MapReduce::Specification specification;
    MapReduce::RecordVector results;
    specification.setDataset(MapReduce::makeDatasetFromContainer(sentences.begin(), sentences.end())); 
    specification.setMapperCount(2);
    specification.setReducerCount(2);

    specification.setMapper(WCMapper::getName());
    specification.setReducer(WCReducer::getName());
    MapReduce::RunComputation(specification, results);
    // Count NPMI
    std::unordered_map<std::string, size_t> countsMap;
    convertToUnorderedSet(results, countsMap);
    InputData data;
    data.counts = &countsMap;
    data.totalSentenceCount = count;

    specification.setMapper(PMIMapper::getName());
    specification.setReducer(PMIReducer::getName());
    specification.setUserData(&data);
    MapReduce::RunComputation(specification, results);
    std::sort(results.begin(), results.end(), [] (const MapReduce::Record &a, const MapReduce::Record &b) {
        return a.getValue() > b.getValue();
    });
    writeOutput(results);
    return 0;
}

