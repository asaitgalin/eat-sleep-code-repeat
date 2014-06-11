#pragma once

#include <memory>
#include <unordered_map>

namespace MapReduce {

/* Some kind of reflection for C++ */

class Mapper;
class Reducer;
class Partitioner;
class KeyComparer;

template <class T>
class ObjectFactory {
public:
    virtual std::shared_ptr<T> createNew() const = 0;
};

using MapperFactory = ObjectFactory<Mapper>;
using ReducerFactory = ObjectFactory<Reducer>;

static std::unordered_map<std::string, std::shared_ptr<MapperFactory>> &getMapperFactoryTable() {
    static std::unordered_map<std::string, std::shared_ptr<MapperFactory>> table;
    return table;
}

static std::unordered_map<std::string, std::shared_ptr<ReducerFactory>> &getReducerFactoryTable() {
    static std::unordered_map<std::string, std::shared_ptr<ReducerFactory>> table;
    return table;
}

static std::unordered_map<std::string, std::shared_ptr<Partitioner>> &getPartitionerTable() {
    static std::unordered_map<std::string, std::shared_ptr<Partitioner>> table;
    return table;
}

static std::unordered_map<std::string, std::shared_ptr<KeyComparer>> &getComparerTable() {
    static std::unordered_map<std::string, std::shared_ptr<KeyComparer>> table;
    return table;
}

#define REGISTER_CLASS(name, class_name, table_name) \
    struct name##Registerer {\
        name##Registerer() {\
            MapReduce::get##table_name##Table().insert(std::make_pair(#class_name, \
                std::make_shared<name>())); \
        } \
    }; \
    name##Registerer reg##name;

#define REGISTER_MAPPER(mapper_name) \
    class mapper_name##Factory: public MapReduce::MapperFactory {\
        virtual std::shared_ptr<MapReduce::Mapper> createNew() const {\
            return std::make_shared<mapper_name>(); \
        } \
    };\
    REGISTER_CLASS(mapper_name##Factory, mapper_name, MapperFactory);

#define REGISTER_REDUCER(reducer_name) \
    class reducer_name##Factory: public MapReduce::ReducerFactory {\
        virtual std::shared_ptr<MapReduce::Reducer> createNew() const {\
            return std::make_shared<reducer_name>(); \
        } \
    }; \
    REGISTER_CLASS(reducer_name##Factory, reducer_name, ReducerFactory)

#define REGISTER_PARTITIONER(partitioner_name) \
    REGISTER_CLASS(partitioner_name, partitioner_name, Partitioner)

#define REGISTER_COMPARER(comparer_name) \
    REGISTER_CLASS(comparer_name, comparer_name, Comparer)

std::shared_ptr<Mapper> createNewMapper(const std::string &name) {
    return getMapperFactoryTable()[name]->createNew();
}

std::shared_ptr<Reducer> createNewReducer(const std::string &name) {
    return getReducerFactoryTable()[name]->createNew();
}

std::shared_ptr<Partitioner> getPartitioner(const std::string &name) {
    return getPartitionerTable()[name];
}

std::shared_ptr<KeyComparer> getComparer(const std::string &name) {
    return getComparerTable()[name];
}

bool isMapperRegistered(const std::string &name) {
    return getMapperFactoryTable().find(name) != getMapperFactoryTable().end();
}

bool isReducerRegistered(const std::string &name) {
    return getReducerFactoryTable().find(name) != getReducerFactoryTable().end();
}

bool isPartitionerRegistered(const std::string &name) {
    return getPartitionerTable().find(name) != getPartitionerTable().end();
}

bool isComparerRegistered(const std::string &name) {
    return getComparerTable().find(name) != getComparerTable().end();
}

} // namespace MapReduce

