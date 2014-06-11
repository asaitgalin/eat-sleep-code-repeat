#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <algorithm>
#include <future>
#include <boost/thread.hpp>

#include "base.hpp"
#include "registerer.hpp"
#include "specification.hpp"
#include "sort.hpp"
#include "utils.hpp"

namespace MapReduce {

/* MapReduce framework computation implementation */

class MapJob {
public:
    MapJob(const Specification &spec, size_t begin, size_t end):
        spec_(spec),
        begin_(begin),
        end_(end)
    { }

    std::shared_ptr<Mapper> operator()() const {
        std::shared_ptr<Mapper> m = createNewMapper(spec_.getMapper());        
        m->setUserData(spec_.getUserData());
        for (size_t i = begin_; i < end_; ++i) {
            std::pair<std::string, std::string> item = spec_.getDataset()->get(i);
            (*m)(item.first, item.second);
        }
        return m;
    }

private:
    const Specification &spec_;
    size_t begin_;
    size_t end_;
};

class ReduceJob {
    using ReducerInput = std::pair<std::string, std::vector<std::string>>;
public:
    ReduceJob(const Specification &spec, const std::vector<std::vector<ReducerInput>> &input,
              size_t index):
        spec_(spec),
        input_(input),
        index_(index)
    { }

    std::shared_ptr<Reducer> operator()() const {
        std::shared_ptr<Reducer> r = createNewReducer(spec_.getReducer());
        r->setUserData(spec_.getUserData());
        for (const auto & i : input_[index_]) {
            (*r)(i.first, i.second);
        }
        return r;
    }

private:
    const Specification &spec_;
    const std::vector<std::vector<ReducerInput>> &input_;
    size_t index_;
};

static void runMapTask(const Specification &spec, RecordVector &merged) {
    size_t dataSize = spec.getDataset()->getSize();
    size_t blockSize;
    size_t threadNum;
    divideByBlocks(dataSize, spec.getMapperCount(), blockSize, threadNum, 1); 
    
    std::vector<boost::thread> mappers;
    std::vector<std::future<std::shared_ptr<Mapper>>> intermediate;

    for (size_t i = 0; i < threadNum; ++i) {
        size_t begin = i * blockSize;
        size_t end = (i == threadNum - 1) ? dataSize : (begin + blockSize);
        std::packaged_task<std::shared_ptr<Mapper>()> task(MapJob(spec, begin, end));
        intermediate.push_back(task.get_future());
        mappers.emplace_back(std::move(task));
    }
    
    std::vector<std::shared_ptr<Mapper>> results;
    size_t totalSize = 0;
    for (auto & f : intermediate) {
        results.push_back(f.get());
        totalSize += results.back()->getSize();
    }
    
    merged.resize(totalSize);
    auto outputIterator = merged.begin();
    for (auto & m : results) {
        outputIterator = std::copy(m->cbegin(), m->cend(), outputIterator);
    }
    std::for_each(mappers.begin(), mappers.end(), std::mem_fn(&boost::thread::join));
}

static void runReducerTask(const Specification &spec, std::vector<std::vector<std::pair<std::string, std::vector<std::string>>>> &input, 
        RecordVector &output) {

    std::vector<boost::thread> reducers;
    std::vector<std::future<std::shared_ptr<Reducer>>> futures;

    for (size_t i = 0; i < spec.getReducerCount(); ++i) {
        std::packaged_task<std::shared_ptr<Reducer>()> task(ReduceJob(spec, input, i));
        futures.push_back(task.get_future());
        reducers.emplace_back(std::move(task));
    }
    
    std::vector<std::shared_ptr<Reducer>> results;
    size_t totalSize = 0;
    for (auto & f : futures) {
        results.push_back(f.get());
        totalSize += results.back()->getSize();
    }
    
    output.resize(totalSize);
    auto outputIterator = output.begin();
    for (auto & r : results) {
        outputIterator = std::copy(r->cbegin(), r->cend(), outputIterator);
    }
    std::for_each(reducers.begin(), reducers.end(), std::mem_fn(&boost::thread::join));
}

static bool isSpecificationReady(const Specification &spec) {
    return !spec.getMapper().empty() && !spec.getReducer().empty() &&
        spec.getDataset();
}

void RunComputation(const Specification &spec, RecordVector &out) {
    if (!isSpecificationReady(spec)) {
        throw std::invalid_argument("Invalid specification. Fill all necessary fields. (MapReduce::RunComputation)");
    }
    RecordVector mergedVector;
    runMapTask(spec, mergedVector);

    std::shared_ptr<KeyComparer> comparer = spec.getKeyComparer();
   
    quickSort(mergedVector.begin(), mergedVector.end(), spec.getSorterCount(), [comparer] (const Record &a, const Record &b) {
            return (*comparer)(a.getKey(), b.getKey());
    });

    using reducerInput = std::pair<std::string, std::vector<std::string>>;
    std::vector<std::vector<reducerInput>> reducerTasks(spec.getReducerCount());
    
    std::shared_ptr<Partitioner> partitioner = spec.getPartitioner();

    size_t i = 0;
    while (i < mergedVector.size()) {
        reducerInput p;
        p.first = mergedVector[i].getKey();

        while (i < mergedVector.size() && mergedVector[i].getKey() == p.first) {
            p.second.push_back(mergedVector[i].getValue());
            ++i;
        }
        
        size_t reducerIndex = partitioner->getReducer(p.first, spec.getReducerCount());
        reducerTasks[reducerIndex].push_back(p);
    }

    runReducerTask(spec, reducerTasks, out);
}

} // namespace MapReduce

