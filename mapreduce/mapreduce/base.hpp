#pragma once

#include <vector>
#include <functional>

namespace MapReduce {

/* Base classes (record, mapper, reducer, partitioner) */

class Record {
public:
    Record() = default;
    Record(const std::string &key, const std::string &value):
        key_(key),
        value_(value)
    { } 

    const std::string &getKey() const { return key_; }
    void setKey(const std::string &key) { key_ = key; }
    
    const std::string &getValue() const { return value_; }
    void setValue(const std::string &value) { value_ = value; }

    std::pair<std::string, std::string> toPair() const {
        return std::make_pair(key_, value_);
    }

private:
    std::string key_;
    std::string value_;
};

using RecordVector = std::vector<Record>;

class MapJob;
class Mapper {
public:
    virtual void operator() (const std::string &key, const std::string &value)  = 0;
    virtual ~Mapper() { } 

    void emitIntermediate(const std::string &key, const std::string &value) {
        intermediate_.push_back(Record(key, value));
    }
    
    void *getUserData() const { return userData_; }
    size_t getSize() const { return intermediate_.size(); }

    using ConstIterator = typename RecordVector::const_iterator;
    
    ConstIterator cbegin() const { return intermediate_.cbegin(); }
    ConstIterator cend() const { return intermediate_.cend(); }
    
private:
    friend class MapJob;
    void setUserData(void *data) { userData_ = data; }
    void *userData_;
    RecordVector intermediate_;
};

class ReduceJob;
class Reducer {
public:
    typedef std::vector<std::string> ValueVector;
    virtual void operator() (const std::string &key, const ValueVector &values) = 0;
    virtual ~Reducer() { }
    
    void emit(const std::string &key, const std::string &value) {
        results_.push_back(Record(key, value));
    }
   
    void *getUserData() const { return userData_; }
    size_t getSize() const { return results_.size(); }

    using ConstIterator = typename RecordVector::const_iterator;
    
    ConstIterator cbegin() const { return results_.cbegin(); }
    ConstIterator cend() const { return results_.cend(); }

private:
    friend class ReduceJob;
    void setUserData(void *data) { userData_ = data; }
    void *userData_;
    RecordVector results_;
};

class Partitioner {
public:
    virtual size_t getReducer(const std::string &key, const size_t reducerCount) const = 0;
    virtual ~Partitioner() { } 
};

class DefaultPartitioner: public Partitioner {
public:
    virtual size_t getReducer(const std::string &key, const size_t reducerCount) const {
        std::hash<std::string> hash;
        return hash(key) % reducerCount;
    }
};

class KeyComparer {
public:
    virtual bool operator() (const std::string &key1, const std::string &key2) const = 0;
    virtual ~KeyComparer() { }
};

class DefaultComparer: public KeyComparer {
public:
    virtual bool operator() (const std::string &key1, const std::string &key2) const {
        return key1 < key2;
    }
};

} // namespace MapReduce

