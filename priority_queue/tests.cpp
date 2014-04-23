#define BOOST_TEST_MODULE HeapsTest
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <random>
#include <algorithm>
#include <chrono>
#include <queue>
#include <stdexcept>

#define DEBUG
#include "priority_queue_binary.hpp"
#include "priority_queue_binomial.hpp"

std::default_random_engine generator;

template <typename Heap>
void correctnessTest() {
    const size_t ItemsCount = 1000000;
    const int ItemRange = 1000000;

    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> uniform(-ItemRange, ItemRange);
   
    Heap heap;

    for (size_t i = 0; i < ItemsCount; ++i) {
        heap.insert(uniform(generator), uniform(generator));
    }
    
    // Извлекаем элементы
    int prevPriority;
    bool prevPrioritySet = false;
   
    while (!heap.empty()) {
        PriorityQueueNode<int, int> node = heap.getTop();
        heap.extractTop();
        if (!prevPrioritySet) {
            prevPrioritySet = true;
            prevPriority = node.getPriority();
        } else {
            BOOST_CHECK(prevPriority >= node.getPriority());
            prevPriority = node.getPriority();
        }
    }
}

BOOST_AUTO_TEST_CASE(BinaryHeapCorrectnessTest) {
    correctnessTest<PriorityQueueBinary<int, int>>();  
}

BOOST_AUTO_TEST_CASE(BinomialHeapCorrectnessTest) {
    correctnessTest<PriorityQueueBinomial<int, int>>();
}

BOOST_AUTO_TEST_CASE(PointersFromDifferentQueuesMustFailTest) {
    PriorityQueueBinary<int, int> binary;
    PriorityQueueBinomial<int, int> binomial;
    PQNodePtr<int, int> binaryPtr = binary.insert(5, 5);
    PQNodePtr<int, int> binomialPtr = binomial.insert(3, 3);
    BOOST_CHECK_THROW(binary.updatePriority(binomialPtr, 1), std::runtime_error);
    BOOST_CHECK_THROW(binomial.updatePriority(binaryPtr, 1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(BinomialPointersFromDifferentInstancesMustFailTest) {
    PriorityQueueBinomial<int, int> binomial1;
    PriorityQueueBinomial<int, int> binomial2;
    PQNodePtr<int, int> binomialPtr1 = binomial1.insert(1, 2);
    PQNodePtr<int, int> binomialPtr2 = binomial2.insert(2, 3);
    BOOST_CHECK_THROW(binomial2.updatePriority(binomialPtr1, 1), std::runtime_error);
    BOOST_CHECK_THROW(binomial1.updatePriority(binomialPtr2, 1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(BinaryPointersFromDifferentInstancesMustFailTest) {
    PriorityQueueBinary<int, int> binary1;
    PriorityQueueBinary<int, int> binary2;
    PQNodePtr<int, int> binaryPtr1 = binary1.insert(1, 2);
    PQNodePtr<int, int> binaryPtr2 = binary2.insert(2, 3);
    BOOST_CHECK_THROW(binary2.updatePriority(binaryPtr1, 1), std::runtime_error);
    BOOST_CHECK_THROW(binary1.updatePriority(binaryPtr2, 1), std::runtime_error);
}

template <typename Heap>
void decreasePriorityTest() {
    Heap heap;
    // Добавляем элементы в очередь
    heap.insert(10, 11);
    heap.insert(12, 8);
    heap.insert(32, 10);
    PQNodePtr<int, int> ptr1 = heap.insert(23, 9);
    PQNodePtr<int, int> ptr2 = heap.insert(24, 7); 
    PQNodePtr<int, int> ptr3 = heap.insert(15, 12);
    PQNodePtr<int, int> ptr4 = heap.insert(16, 42);
    PQNodePtr<int, int> ptr5 = heap.insert(100, 24);
   
    heap.updatePriority(ptr3, 1); // 15 12 --> 15 1
    BOOST_CHECK_EQUAL(heap.getTop().getKey(), 15);
    BOOST_CHECK_EQUAL(heap.getTop().getPriority(), 1);
    heap.extractTop();
    
    heap.updatePriority(ptr1, 5); // 23 9 --> 23 5
    BOOST_CHECK_EQUAL(heap.getTop().getKey(), 23);
    BOOST_CHECK_EQUAL(heap.getTop().getPriority(), 5);
    heap.extractTop();
   
    heap.updatePriority(ptr4, 41);

    BOOST_CHECK_EQUAL(heap.getTop().getKey(), 24);
    BOOST_CHECK_EQUAL(heap.getTop().getPriority(), 7);

    // Проверяем сломанные и не сломанные указатели
    BOOST_CHECK(ptr1->isValid() == false);
    
    BOOST_CHECK(ptr2->isValid() == true);
    BOOST_CHECK_EQUAL(ptr2->getNode().getKey(), 24);
    BOOST_CHECK_EQUAL(ptr2->getNode().getPriority(), 7);

    BOOST_CHECK(ptr3->isValid() == false);

    BOOST_CHECK(ptr4->isValid() == true);
    BOOST_CHECK_EQUAL(ptr4->getNode().getKey(), 16);
    BOOST_CHECK_EQUAL(ptr4->getNode().getPriority(), 41);

    BOOST_CHECK(ptr5->isValid() == true);
    BOOST_CHECK_EQUAL(ptr5->getNode().getKey(), 100);
    BOOST_CHECK_EQUAL(ptr5->getNode().getPriority(), 24);
}

BOOST_AUTO_TEST_CASE(BinaryHeapDecreasePriorityTest) {
    decreasePriorityTest<PriorityQueueBinary<int, int, std::greater<int>>>();
}

BOOST_AUTO_TEST_CASE(BinomialHeapDecreasePriorityTest) {
    decreasePriorityTest<PriorityQueueBinomial<int, int, std::greater<int>>>();
}

template <class Heap> 
void sizeTest() {
    Heap heap;
    heap.insert(3, 3);
    BOOST_CHECK_EQUAL(heap.size(), 1);
    heap.insert(4, 4);
    BOOST_CHECK_EQUAL(heap.size(), 2);
    heap.insert(5, 5);
    BOOST_CHECK_EQUAL(heap.size(), 3);
    heap.extractTop();
    BOOST_CHECK_EQUAL(heap.size(), 2);
    heap.clear();
    BOOST_CHECK(heap.empty());
    BOOST_CHECK_EQUAL(heap.size(), 0);
}

BOOST_AUTO_TEST_CASE(BinaryHeapSizeTest) {
    sizeTest<PriorityQueueBinary<int, int>>();
}

BOOST_AUTO_TEST_CASE(BinomialHeapSizeTest) {
    sizeTest<PriorityQueueBinomial<int, int>>();
}

BOOST_AUTO_TEST_CASE(BinomialHeapCopyTest) {
    PriorityQueueBinomial<int, int> q;
    for (int i = 0; i < 10; ++i) {
        q.insert(i * 5, i + 1);
    }
    PriorityQueueBinomial<int, int> q2(q);
    size_t count = 0;
    size_t i = 10;
    while (!q2.empty()) {
        BOOST_CHECK_EQUAL(q2.getTop().getPriority(), i); 
        q2.extractTop();
        --i;
        ++count;
    }
    BOOST_CHECK(count == q.size());
    // Оригинальная очередь
    i = 10;
    while (!q.empty()) {
        BOOST_CHECK_EQUAL(q.getTop().getPriority(), i); 
        q.extractTop();
        --i;
    }
    BOOST_CHECK(q.empty());
}

