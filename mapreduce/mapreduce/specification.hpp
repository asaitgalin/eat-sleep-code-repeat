#pragma once

#include <memory>

#include "dataset.hpp"
#include "registerer.hpp"

namespace MapReduce {

/* MapReduce configuration */

class Specification {
public:
    Specification():
        mapperCount_(1),
        reducerCount_(1),
        sorterCount_(1),
        userData_(NULL)
    { }

    void setMapper(const std::string &name) {
        if (!isMapperRegistered(name)) {
            throw std::runtime_error("Mapper class not found (Specification::setMapper)");
        }
        mapper_ = name;
    }

    std::string getMapper() const { return mapper_; }
    
    void setReducer(const std::string &name) {
        if (!isReducerRegistered(name)) {
            throw std::runtime_error("Reducer class not found (Specification::setReducer)");
        }
        reducer_ = name;
    }

    std::string getReducer() const { return reducer_; }

    void setPartitioner(const std::string &name) {
        if (!isPartitionerRegistered(name)) {
            throw std::runtime_error("Partitioner class not found (Specification::setPartitioner)");
        }
        partitioner_ = MapReduce::getPartitioner(name);
    }
    
    std::shared_ptr<Partitioner> getPartitioner() const {
        if (!partitioner_) {
            return std::make_shared<DefaultPartitioner>();
        }
        return partitioner_;
    }

    void setKeyComparer(const std::string &name) {
        if (!isComparerRegistered(name)) {
            throw std::runtime_error("KeyComparer class not found (Specification::setPartitioner)");
        }
        comparer_ = MapReduce::getComparer(name);
    }
    
    std::shared_ptr<KeyComparer> getKeyComparer() const {
        if (!comparer_) {
            return std::make_shared<DefaultComparer>();
        }
        return comparer_;
    }

    void setDataset(std::shared_ptr<Dataset> data) { dataset_ = data; }

    std::shared_ptr<Dataset> getDataset() const { return dataset_; }

    void setMapperCount(size_t count) { 
        if (count == 0) {
            throw std::invalid_argument("Invalid count parameter (Specification::setMapperCount)");
        }
        mapperCount_ = count; 
    }

    size_t getMapperCount() const { return mapperCount_; }
    
    void setReducerCount(size_t count) {
        if (count == 0) {
            throw std::invalid_argument("Invalid count parameter (Specification::setReducerCount)");
        }
        reducerCount_ = count;
    }

    size_t getReducerCount() const { return reducerCount_; }

    void setSorterCount(size_t count) {
        if (count == 0) {
            throw std::invalid_argument("Invalid count parameter (Specification::setSorterCount)");
        }
        sorterCount_ = count;
    }

    size_t getSorterCount() const { return sorterCount_; }

    void setUserData(void *data) { userData_ = data; }
    void *getUserData() const { return userData_; }

private:
    std::string mapper_;
    std::string reducer_;
    std::shared_ptr<Partitioner> partitioner_;
    std::shared_ptr<Dataset> dataset_;
    std::shared_ptr<KeyComparer> comparer_;
    size_t mapperCount_;
    size_t reducerCount_;
    size_t sorterCount_;
    void *userData_;
};

} // namespace MapReduce

