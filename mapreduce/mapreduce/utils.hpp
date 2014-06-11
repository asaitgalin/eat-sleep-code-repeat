#pragma once

#include <algorithm>
#include <stdexcept>

namespace MapReduce {

void divideByBlocks(size_t dataSize, size_t threadCount, size_t &blockSize, 
                    size_t &numThreads, size_t minPerThread = 10) {
    if (threadCount == 0) {
        throw std::runtime_error("divideByBlocks: incorrent threadCount parameter");
    }
    if (dataSize == 0) {
        blockSize = 0;
        numThreads = 0;
        return;
    }
    size_t maxThreads = (dataSize + minPerThread - 1) / minPerThread;
    numThreads = std::min(threadCount, maxThreads);
    blockSize = dataSize / numThreads;
}

} // namespace MapReduce

