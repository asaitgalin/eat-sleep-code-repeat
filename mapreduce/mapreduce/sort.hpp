#pragma once

#include <functional>
#include <iterator>

#include <pool.hpp>

namespace MapReduce {

template <class ForwardIt, class Compare>
static void insertionSort(ForwardIt first, ForwardIt last, Compare cmp) {
    if (std::distance(first, last) < 2) {
        return;
    }
    for (auto pi = first + 1; pi != last; ++pi) {
        for (auto pj = pi; pj != first && cmp(*pj, *(pj - 1)); --pj) {
            std::swap(*pj, *(pj - 1));
        }
    }
}

template <class ForwardIt, class Compare>
static void doQuickSort(ForwardIt first, ForwardIt last, ThreadPool &threadPool, Compare cmp) {
    using T = typename std::iterator_traits<ForwardIt>::value_type;
    while (true) {
        if (first == last) {
            return;
        }
        auto diff = std::distance(first, last);
        if (diff < 10) {
            insertionSort(first, last, cmp);
            return;
        }
        
        T pivot = *std::next(first, diff / 2);

        auto middle1 = std::partition(first, last, [pivot, cmp] (const T &item) { 
            return cmp(item, pivot); 
        });
        auto middle2 = std::partition(middle1, last, [pivot, cmp] (const T &item) { 
            return !cmp(pivot, item); 
        });
        
        threadPool.addTask(std::bind(doQuickSort<ForwardIt, Compare>, middle2, last, std::ref(threadPool), cmp));
        last = middle1; 
    } 
}

template <class ForwardIt, class Compare = 
          std::less<typename std::iterator_traits<ForwardIt>::value_type>>
void quickSort(ForwardIt first, ForwardIt last, size_t threadCount = std::thread::hardware_concurrency(), 
                                Compare cmp = Compare()) {
    ThreadPool threadPool(threadCount);
    doQuickSort(first, last, threadPool, cmp);
}

} // namespace MapReduce

