// priority_queue.hpp
// Andrey Saitgalin, 2014
#pragma once

#include <functional>
#include <memory>

// Минимальный класс для элемента кучи (ключ + приоритет)
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

// Интерфейс для произвольного абстрактного указателя на элемент в очереди
// Необходим для операции updateKey
template<class _T, class _Priority, class _Id = size_t>
class IPriorityQueueNodePtr {
public:
    virtual const PriorityQueueNode<_T, _Priority> & getNode() const = 0;
    virtual bool isValid() const = 0;
    // Возвращает абстрактный уникальный идентификатор. Так как положения 
    // элементов в структуре данных могут поменяться, по идентификатору всегда 
    // должна быть возможность восстановить элемент
    virtual _Id getId() const = 0;
    // Возвращает абстрактный указатель на кучу, его создавшую. Не освобождать!
    virtual const void *getParentPtr() const = 0; 
};

template <class Key, class Value> using PQNodePtr = std::shared_ptr<IPriorityQueueNodePtr<Key, Value>>;

// Произвольная очередь с приоритетом
// Компаратор работает также, как в куче из STL (...the element popped is the last according 
// to strict weak ordering criterion...)
// По умолчанию в вершине находится максимальный элемент
template<class _T, class _Priority, class _Comp = std::less<_Priority>>
class IPriorityQueue {
public:
    virtual PQNodePtr<_T, _Priority> insert(const _T &key, const _Priority &priority) = 0;
    virtual const PriorityQueueNode<_T, _Priority> & getTop() const = 0;
    virtual void extractTop() = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
    virtual void updatePriority(PQNodePtr<_T, _Priority> pointer, const _Priority &newPriority) = 0;
};

