#include <cstdlib>
#include <string>
#include <fstream>
#include <boost/tokenizer.hpp>

#include <mapreduce/mapreduce.hpp>

/* Computing inverted index through MapReduce framework */

class InvertedIndexMapper: public MapReduce::Mapper {
public:
    virtual void operator() (const std::string &key, const std::string &value) {
        boost::tokenizer<> tokenizer(value); // used to split sentence into words 
        for (auto it = tokenizer.begin(); it != tokenizer.end(); ++it) {
            emitIntermediate(*it, key);
        }
    }
    static std::string getName() {
        return "InvertedIndexMapper";
    }
};

REGISTER_MAPPER(InvertedIndexMapper)

class InvertedIndexReducer: public MapReduce::Reducer {
public:
    virtual void operator() (const std::string &key, const ValueVector &values) {
        std::string value;
        for (size_t i = 0; i < values.size(); ++i) {
            value.append(values[i]);
            if (i != values.size() - 1) {
                value.append(", ");
            }
        }
        emit(key, value);
    }
    static std::string getName() {
        return "InvertedIndexReducer";
    }
};

REGISTER_REDUCER(InvertedIndexReducer)

void readInputText(const std::string &fileName, std::vector<std::pair<std::string, std::string>> &output) {
    std::fstream file(fileName, std::ios_base::in); 
    std::string line;
    size_t counter = 1;
    output.clear();
    while (std::getline(file, line)) {
        output.push_back(std::make_pair(std::to_string(counter), line));
        ++counter;
    }
}

void writeOutput(const MapReduce::RecordVector &results) {
    std::fstream file("output.txt", std::ios_base::out);
    for (const auto &it : results) {
        file << it.getKey() << " -> [" << it.getValue() << "]" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Wrong arguments. Usage: index text.txt" << std::endl;
        exit(-1);
    }
    std::vector<std::pair<std::string, std::string>> sentences;
    readInputText(argv[1], sentences);
    // Configure
    MapReduce::Specification specification;
    specification.setDataset(MapReduce::makeDatasetFromContainer(sentences.begin(), sentences.end())); 
    specification.setMapper(InvertedIndexMapper::getName());
    specification.setReducer(InvertedIndexReducer::getName());
    specification.setMapperCount(2);
    specification.setReducerCount(2);
    // Run
    MapReduce::RecordVector results;
    MapReduce::RunComputation(specification, results);
    writeOutput(results);
    return 0;
}

