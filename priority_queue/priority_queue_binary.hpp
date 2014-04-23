#pragma once

#include <vector>
#include <stdexcept>
#include <map>
#include <unordered_map>

#include "priority_queue.hpp"

template<class _T, class _Priority, class _Comp = std::less<_Priority>>
class PriorityQueueBinary: public IPriorityQueue<_T, _Priority, _Comp> {
private:

    class PriorityQueueBinaryPtr: public IPriorityQueueNodePtr<_T, _Priority> {
    public:
        
        PriorityQueueBinaryPtr(const PriorityQueueBinary<_T, _Priority, _Comp> *ptr, size_t id):
            id_(id),
            isHeapDestroyed_(ptr->heapDestroyed_),
            parent_(ptr)
        { }

        const PriorityQueueBinary<_T, _Priority, _Comp> *getParent() const {
            return parent_;
        }

        // IPriorityQueueNodePtr implementation
        virtual const PriorityQueueNode<_T, _Priority> & getNode() const {
            if (!isValid()) {
                throw std::runtime_error("binary heap node pointer is invalid");
            }
            size_t realIndex = parent_->adr_.find(id_)->second;
            return parent_->data_[realIndex];
        }

        virtual bool isValid() const {
            return (*isHeapDestroyed_) == false &&
                parent_->adr_.find(id_) != parent_->adr_.end() &&
                parent_->adr_.find(id_)->second < parent_->data_.size();
        }

        size_t getId() const { return id_; }

    private:
        size_t id_;
        std::shared_ptr<bool> isHeapDestroyed_;
        const PriorityQueueBinary<_T, _Priority, _Comp> *parent_;
    };

    typedef PriorityQueueNode<_T, _Priority> _NodeType;
    typedef IPriorityQueueNodePtr<_T, _Priority> _BasePtr;
    typedef PriorityQueueBinaryPtr _BinaryPtr;

public:

    PriorityQueueBinary(const _Comp &compare = _Comp()): 
        base_(0),
        comparer_(compare),
        heapDestroyed_(std::make_shared<bool>(false))
    { }

    PriorityQueueBinary(const std::vector<_NodeType> &items, 
                        const _Comp &compare = _Comp()):
        base_(0),
        data_(items),
        comparer_(compare),
        heapDestroyed_(std::make_shared<bool>(false))
    {
        buildHeap();
    }

    PriorityQueueBinary(const std::vector<std::pair<_T, _Priority>> &items, 
                        const _Comp &compare = _Comp());
    PriorityQueueBinary(const PriorityQueueBinary &pq);

    ~PriorityQueueBinary() {
        (*heapDestroyed_) = true;
    }

    PriorityQueueBinary & operator= (const PriorityQueueBinary &rhs);
    bool operator== (const PriorityQueueBinary &rhs) const = delete;

    // IPriorityQueue implementation
    virtual std::shared_ptr<_BasePtr> insert(const _T &data, const _Priority &priority);
    virtual const _NodeType & getTop() const { return data_[0]; }
    virtual void extractTop(); 
    
    virtual void clear() {
        data_.clear();
        base_ = 0;
    }
  
    virtual size_t size() const { return data_.size(); }
    virtual bool empty() const { return data_.empty(); }
    virtual void updatePriority(std::shared_ptr<_BasePtr> pointer, const _Priority &newPriority);

#ifdef DEBUG
    void dump(std::ostream &o) {
        for (size_t i = 0; i < data_.size(); ++i) {
            o << "Key: " << data_[i].getKey() << ", " << "Prior: " << data_[i].getPriority() << std::endl;
        }
    }
#endif

private:
    size_t base_;
    std::vector<_NodeType> data_;
    _Comp comparer_;
    std::vector<size_t> positions_;
    std::unordered_map<size_t, size_t> adr_; // adr_: node_number -> node_index_in_vector
    std::shared_ptr<bool> heapDestroyed_;

    friend class PriorityQueueBinaryPtr;

    void swapNodes(size_t i, size_t j) {
        std::swap(data_[i], data_[j]);
        std::swap(adr_[positions_[i]], adr_[positions_[j]]); 
        std::swap(positions_[i], positions_[j]); 
    }
   
    void buildHeap();
    void siftUp(size_t i);
    void heapify(size_t i);
};

#include "priority_queue_binary_impl.hpp"

