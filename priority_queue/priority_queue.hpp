#pragma once

#include <functional>
#include <memory>

template<class _T, class _Priority>
class PriorityQueueNode {
public:

    PriorityQueueNode() = default;

    PriorityQueueNode(const _T &key, const _Priority &priority):
        key_(key),
        priority_(priority)
    { }

    virtual ~PriorityQueueNode() { }

    const _Priority & getPriority() const { return priority_; }
    void setPriority(const _Priority &priority) { priority_ = priority; }

    void setKey(const _T &key) { key_ = key; } 
    const _T & getKey() const { return key_; }

    void swap(PriorityQueueNode<_T, _Priority> &node) {
        std::swap(key_, node.key_);
        std::swap(priority_, node.priority_);
    }

private:
    _T key_;
    _Priority priority_;
};

template<class _T, class _Priority>
class IPriorityQueueNodePtr {
public:
    virtual const PriorityQueueNode<_T, _Priority> & getNode() const = 0;
    virtual bool isValid() const = 0;
};

template <class Key, class Value> using PQNodePtr = std::shared_ptr<IPriorityQueueNodePtr<Key, Value>>;

template<class _T, class _Priority, class _Comp = std::less<_Priority>,
    typename _NodePtr = std::shared_ptr<IPriorityQueueNodePtr<_T, _Priority>>>
class IPriorityQueue {
public:
    virtual _NodePtr insert(const _T &key, const _Priority &priority) = 0;
    virtual const PriorityQueueNode<_T, _Priority> & getTop() const = 0;
    virtual void extractTop() = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
    virtual void updatePriority(_NodePtr pointer, const _Priority &newPriority) = 0;
};

