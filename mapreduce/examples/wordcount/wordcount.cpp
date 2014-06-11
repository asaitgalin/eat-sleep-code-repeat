#include <cstdlib>
#include <string>
#include <fstream>
#include <boost/tokenizer.hpp>

#include <mapreduce/mapreduce.hpp>

/* Count word frequencies in file using MapReduce framework */

class WordCountMapper: public MapReduce::Mapper {
public:
    virtual void operator() (const std::string &key, const std::string &value) {
        boost::tokenizer<> tokenizer(value); // used to split sentence into words 
        for (auto it = tokenizer.begin(); it != tokenizer.end(); ++it) {
            emitIntermediate(*it, "1");
        }
    }
    static std::string getName() {
        return "WordCountMapper";
    }
};

REGISTER_MAPPER(WordCountMapper)

class WordCountReducer: public MapReduce::Reducer {
public:
    virtual void operator() (const std::string &key, const ValueVector &values) {
        size_t totalCount = 0;
        for (const auto & it : values) {
           totalCount += std::atoi(it.c_str()); 
        }
        emit(key, std::to_string(totalCount));
    }
    static std::string getName() {
        return "WordCountReducer";
    }
};

REGISTER_REDUCER(WordCountReducer)

void readInputText(const std::string &fileName, std::vector<std::pair<std::string, std::string>> &output) {
    std::fstream file(fileName, std::ios_base::in); 
    std::string line;
    size_t counter = 0;
    output.clear();
    while (std::getline(file, line)) {
        output.push_back(std::make_pair(std::to_string(counter), line));
        ++counter;
    }
}

void writeOutput(const MapReduce::RecordVector &results) {
    std::fstream file("output.txt", std::ios_base::out);
    for (const auto &it : results) {
        file << it.getKey() << " " << it.getValue() << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Wrong arguments. Usage: wc text.txt" << std::endl;
        exit(-1);
    }
    std::vector<std::pair<std::string, std::string>> sentences;
    readInputText(argv[1], sentences);
    // Configure
    MapReduce::Specification specification;
    specification.setDataset(MapReduce::makeDatasetFromContainer(sentences.begin(), sentences.end())); 
    specification.setMapper(WordCountMapper::getName());
    specification.setReducer(WordCountReducer::getName());
    specification.setMapperCount(4);
    specification.setReducerCount(2);
    // Run
    MapReduce::RecordVector results;
    MapReduce::RunComputation(specification, results);
    writeOutput(results);
    return 0;
}

