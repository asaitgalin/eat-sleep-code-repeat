#include <vector>
#include <thread>

#include "pool.hpp"
#define BOOST_TEST_MODULE ThreadPoolTest
#include <boost/test/included/unit_test.hpp>

ThreadPool pool(3);

bool isValidArray(const std::vector<int> &numbers) {
    int k = 0;
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (numbers[i] != k) {
            return false;
        }
        k = (k + 1) % 5;
    }
    return true;
}

int sumFunction(int a, int b) {
    return a + b;
}

BOOST_AUTO_TEST_CASE(BasicFunctionality) {
    std::vector<int> numbers;
    FutureVector<void> results;
    std::mutex numbersMutex;
    for (size_t i = 0; i < 100; ++i) {
        results.push_back(pool.addTask([&numbers, &numbersMutex] { 
            std::lock_guard<std::mutex> lk(numbersMutex);
            for (int i = 0; i < 5; ++i) {
                numbers.push_back(i);
            }
        }));
    }
    ThreadPool::waitAll(results);        
    BOOST_CHECK_EQUAL(numbers.size(), 500);
    BOOST_CHECK(isValidArray(numbers));
}

BOOST_AUTO_TEST_CASE(NonVoidFunctionTest) {
    FutureVector<int> results;  
    for (int i = 1; i <= 100; ++i) {
        results.push_back(pool.addTask([i]() -> int {
            return i * 5;
        }));
    }
    std::future<int> sum = pool.addTask([&results]() -> int {
        int sum = 0;
        for (auto &x : results) {
            sum += x.get();
        }
        return sum;
    });
    BOOST_CHECK_EQUAL(sum.get(), 25250);
}

BOOST_AUTO_TEST_CASE(FunctionWithArgsTest) {
    FutureVector<int> results;
    for (int i = 0; i < 50; ++i) {
        results.push_back(pool.addTask(std::bind(&sumFunction, i, i + 1))); 
    }
    int sum = 0;
    for (auto &x : results) {
        int a = x.get();
        sum += a;
        
    }
    BOOST_CHECK_EQUAL(sum, 2500);
}

