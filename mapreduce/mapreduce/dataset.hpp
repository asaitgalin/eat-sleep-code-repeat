#pragma once

#include <string>
#include <iterator>
#include <stdexcept>
#include <memory>

namespace MapReduce {

/* Contains classes for managing MapReduce input */

template <class T>
class CustomDataset {
public:
    virtual size_t getSize() const = 0;
    virtual T get(const size_t index) const = 0;
    virtual ~CustomDataset() { }
};

using Dataset = CustomDataset<std::pair<const std::string, std::string>>;

template <class T, class Iterator>
class CustomContainerDataset: public CustomDataset<T> {
public:
    CustomContainerDataset(Iterator begin, Iterator end):
        begin_(begin),
        end_(end),
        size_(std::distance(begin, end))
    { }
    
    CustomContainerDataset(Iterator begin, Iterator end, size_t size):
        begin_(begin),
        end_(end),
        size_(size)
    { }

    virtual size_t getSize() const { return size_; }

    virtual T get(const size_t index) const {
        if (index >= size_) {
            throw std::runtime_error("Index out of bounds (ContainerDataset::operator[])");
        }
        return *std::next(begin_, index);
    }

private:
    Iterator begin_;
    Iterator end_;
    size_t size_;
};

template <class It> using ContainerDataset = 
    CustomContainerDataset<std::pair<const std::string, std::string>, It>;

template <class It>
std::shared_ptr<Dataset> makeDatasetFromContainer(It begin, It end, size_t size) {
    return std::make_shared<ContainerDataset<It>>(begin, end, size);
}

template <class It>
std::shared_ptr<Dataset> makeDatasetFromContainer(It begin, It end) {
    return std::make_shared<ContainerDataset<It>>(begin, end);
}

} // namespace MapReduce

