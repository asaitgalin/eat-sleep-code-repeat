#pragma once

#include <memory>
#include <stdexcept>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <random>

#include "priority_queue.hpp"

template<class _T, class _Priority, class _Comp = std::less<_Priority>>
class PriorityQueueBinomial: public IPriorityQueue<_T, _Priority, _Comp> {
private:

    class BinomialNode;
    typedef std::shared_ptr<BinomialNode> _NodePtr;
    typedef std::weak_ptr<BinomialNode> _NodeWPtr;

    class BinomialNode: public PriorityQueueNode<_T, _Priority> {    
    public:
        
        BinomialNode(const _T &data, const _Priority &priority):
            PriorityQueueNode<_T, _Priority>(data, priority),
            degree_(0)
        { }

        _NodePtr getParent() const { return parent_.lock(); }
        _NodePtr getSibling() const { return sibling_; }
        _NodePtr getChild() const { return child_; }
        size_t getDegree() const  { return degree_; }
        size_t getId() const { return id_; }

        void setParent(_NodePtr newParent) { parent_ = newParent; }
        void setSibling(_NodePtr newSibling) { sibling_ = newSibling; }
        void setChild(_NodePtr newChild) { child_ = newChild; }
        void setDegree(size_t newDegree) { degree_ = newDegree; }
        void setId(size_t newId) { id_ = newId; }

    private:
        _NodeWPtr parent_;
        _NodePtr sibling_;
        _NodePtr child_;
        size_t degree_;
        size_t id_;
    };

    class PriorityQueueBinomialPtr: public IPriorityQueueNodePtr<_T, _Priority> {
    public:

        PriorityQueueBinomialPtr(size_t id, const PriorityQueueBinomial<_T, _Priority, _Comp> *ptr):
            id_(id),
            parent_(ptr),
            isHeapDestroyed_(ptr->heapDestroyed_)
        { }

        const PriorityQueueBinomial<_T, _Priority, _Comp> *getParent() const {
            return parent_;
        }

        size_t getId() const { return id_; }
        void setId(size_t newId) { id_ = newId; }
        
        // IPriorityQueueNodePtr implementation
        virtual const PriorityQueueNode<_T, _Priority> & getNode() const {
            if (!isValid()) {
                throw std::runtime_error("binomial heap node pointer is expired");
            }
            return *parent_->nodePtrs_.find(id_)->second.lock();
        }

        virtual bool isValid() const {
            return (*isHeapDestroyed_) == false && 
                parent_->nodePtrs_.find(id_) != parent_->nodePtrs_.end();
        }

    private:
        size_t id_;
        const PriorityQueueBinomial<_T, _Priority, _Comp> *parent_; 
        std::shared_ptr<bool> isHeapDestroyed_;
    };

    typedef PriorityQueueNode<_T, _Priority> _NodeType;
    typedef IPriorityQueueNodePtr<_T, _Priority> _BasePtr;
    typedef PriorityQueueBinomialPtr _BinomialPtr;

public:
    
    PriorityQueueBinomial(const _Comp &compare = _Comp()):
        size_(0),
        base_(0),
        comparer_(compare),
        heapDestroyed_(std::make_shared<bool>(false))
    { }

    ~PriorityQueueBinomial() {
        (*heapDestroyed_) = true;
    }

    PriorityQueueBinomial(const PriorityQueueBinomial &heap);
    PriorityQueueBinomial & operator= (const PriorityQueueBinomial &rhs);

    bool operator== (const PriorityQueueBinomial &rhs) const = delete;

    // IPriorityQueue implementation
    virtual std::shared_ptr<_BasePtr> insert(const _T &data, const _Priority &priority);
    virtual const _NodeType & getTop() const;
    virtual void extractTop();
    
    virtual void clear() {
        head_.reset();
        nodePtrs_.clear();
        size_ = 0;
        base_ = 0;
    }
    virtual size_t size() const { return size_; }
    virtual bool empty() const { return size_ == 0; }
    virtual void updatePriority(std::shared_ptr<_BasePtr> pointer, const _Priority &newPriority); 

private:
    size_t size_;
    size_t base_;
    _Comp comparer_;
    std::shared_ptr<BinomialNode> head_;
    std::unordered_map<size_t, _NodeWPtr> nodePtrs_;
    std::shared_ptr<bool> heapDestroyed_;
   
    friend class PriorityQueueBinomialPtr;

    void binomialHeapLink(_NodePtr y, _NodePtr z) {
        y->setParent(z);
        y->setSibling(z->getChild());
        z->setChild(y);
        z->setDegree(z->getDegree() + 1);      
    }

    _NodePtr binomialHeapMerge(_NodePtr first, _NodePtr second);
    void binomialHeapUnionWithThis(_NodePtr heapHead);
    _NodePtr copyNode(_NodePtr node, _NodePtr parent);
};

#include "priority_queue_binomial_impl.hpp"

