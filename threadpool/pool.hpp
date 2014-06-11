#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <future>
#include <algorithm>
#include <type_traits>
#include <atomic>

class FunctionWrapper {
private:
    typedef std::function<void()> _Proc;
public:
 
    template <class Fn, class Ret>
    static _Proc makeProc(std::future<Ret> &result, Fn task) {
        std::shared_ptr<std::promise<Ret>> promise = std::make_shared<std::promise<Ret>>();
        result = promise->get_future();
        return [task, promise] {
            try {
                promise->set_value(task());
            } catch (const std::exception &ex) {
                promise->set_exception(std::current_exception());
            }
        }; 
    }

    template <class Fn>
    static _Proc makeProc(std::future<void> &result, Fn task) {
        std::shared_ptr<std::promise<void>> promise = std::make_shared<std::promise<void>>();
        result = promise->get_future();
        return [task, promise] {
            try {
                task();
                promise->set_value();
            } catch (const std::exception &ex) {
                promise->set_exception(std::current_exception());
            }
        };
    }

};

template <class T> using FutureVector = std::vector<std::future<T>>;

class ThreadPool {
private:
    typedef std::function<void()> _TaskType;
public:
    
    ThreadPool(size_t threadsCount = std::thread::hardware_concurrency()):
        closingFlag_(false)  
    {
        for (size_t i = 0; i < threadsCount; ++i) {
            threads_.emplace_back(std::bind(&ThreadPool::threadProc, this));
        }
    }

    ThreadPool(const ThreadPool &rhs) = delete;
    ThreadPool &operator= (const ThreadPool &rhs) = delete;

    ~ThreadPool() {
        setClosingFlag();
        std::for_each(threads_.begin(), threads_.end(), 
                std::mem_fn(&std::thread::join));
    }

    template <class Fn>
    std::future<typename std::result_of<Fn()>::type> addTask(Fn task) {
        std::future<typename std::result_of<Fn()>::type> result;
        std::unique_lock<std::mutex> lock(conditionMutex_);
        queue_.push(FunctionWrapper::makeProc(result, task));
        condition_.notify_one();
        return result;
    }
    
    size_t getThreadCount() const {
        return threads_.size();
    }

    template <class T>
    static void waitAll(const FutureVector<T> &fv) {
        std::for_each(fv.begin(), fv.end(), std::mem_fn(&std::future<T>::wait));
    }

private:

    void threadProc() {
        _TaskType task;
        while (extractTop(task)) {
            task();
        }
    }

    bool extractTop(_TaskType &out) {
        std::unique_lock<std::mutex> lock(conditionMutex_);
        condition_.wait(lock, [this]() -> bool {
            return !queue_.empty() || closingFlag_;
        });
        if (!queue_.empty()) {
            out = queue_.front();
            queue_.pop();
            return true;
        }
        return false;
    }

    void setClosingFlag() {
        std::unique_lock<std::mutex> lock(conditionMutex_);
        closingFlag_ = true;
        condition_.notify_all();
    }

    std::vector<std::thread> threads_;
    std::queue<_TaskType> queue_;
    std::mutex conditionMutex_;
    std::condition_variable condition_;
    bool closingFlag_;
};

