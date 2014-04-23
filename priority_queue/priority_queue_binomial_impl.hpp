#define PQBinomial PriorityQueueBinomial<_T, _Priority, _Comp>

template <class _T, class _Priority, class _Comp>
PQBinomial::PriorityQueueBinomial(const PQBinomial &heap) {
    base_ = 0;
    head_ = copyNode(heap.head_, nullptr);
    size_ = heap.size_;
    comparer_ = heap.comparer_;
    heapDestroyed_ = std::make_shared<bool>(false);
}

template <class _T, class _Priority, class _Comp>
PQBinomial & PQBinomial::operator= (const PQBinomial &rhs) {
    if (&rhs != this) {
        base_ = 0;
        head_ = copyNode(rhs.head_, nullptr);
        size_ = rhs.size_;
        comparer_ = rhs.comparer_;
        heapDestroyed_ = std::make_shared<bool>(false);
    }
}

template <class _T, class _Priority, class _Comp>
std::shared_ptr<typename PQBinomial::_BasePtr> PQBinomial::insert(const _T &data, const _Priority &priority) {
    _NodePtr node = std::make_shared<BinomialNode>(data, priority);
    size_t id = base_++;
    nodePtrs_[id] = node;
    node->setId(id);
    binomialHeapUnionWithThis(node);
    ++size_;
    return std::make_shared<_BinomialPtr>(id, this);
}

template <class _T, class _Priority, class _Comp>
const PriorityQueueNode<_T, _Priority> & PQBinomial::getTop() const {
    _NodePtr x = head_;
    _NodePtr y = x;
    if (!x) {
        throw std::runtime_error("getTop error: binomial heap is empty");
    }
    while (x) {
        if (comparer_(y->getPriority(), x->getPriority())) {
            y = x;
        }
        x = x->getSibling();
    }
    return *y;
}

template <class _T, class _Priority, class _Comp>
void PQBinomial::extractTop() {           
    _NodePtr x = head_;
    _NodePtr min = x;
    _NodePtr minPrev;
    _NodePtr prev;
    if (!x) {
        throw std::runtime_error("getTop error: binomial heap is empty");
    }
    while (x) {
        if (comparer_(min->getPriority(), x->getPriority())) {
            min = x;
            minPrev = prev;
        }
        prev = x;
        x = x->getSibling();
    }
    // Extract minimum
    _NodePtr node = min->getChild();
    if (node) {
        _NodePtr next = node->getSibling();
        _NodePtr savedSibling;
        node->setSibling(_NodePtr());
        node->setParent(_NodePtr());
        while (next) {
            savedSibling = next->getSibling();
            next->setParent(_NodePtr());
            next->setSibling(node);
            node = next;
            next = savedSibling;
        }
    }
    // Free minimal node
    if (min == head_) {
        head_ = min->getSibling();
    } else {
        minPrev->setSibling(min->getSibling());
    }
    if (node) {
        binomialHeapUnionWithThis(node);
    }
    nodePtrs_.erase(min->getId());
    --size_;
}

template <class _T, class _Priority, class _Comp>
void PQBinomial::updatePriority(std::shared_ptr<_BasePtr> pointer, const _Priority &newPriority) {
    if (!pointer) {
        throw std::runtime_error("update key error: null pointer");
    }
    std::shared_ptr<_BinomialPtr> ptr = std::dynamic_pointer_cast<_BinomialPtr, _BasePtr>(pointer);
    if (!ptr || (ptr->getParent() != this) || !ptr->isValid()) {
        throw std::runtime_error("update key error: invalid pointer");
    }
    _NodePtr node = nodePtrs_[ptr->getId()].lock();
    if (comparer_(newPriority, node->getPriority())) {
        throw std::invalid_argument("update key error: invalid new priority");
    }
    node->setPriority(newPriority);
    _NodePtr p = node->getParent();
    _NodePtr y = node;
    while (p && comparer_(p->getPriority(), y->getPriority())) {
        y->swap(*p); // Swap node contents
        
        // Swap pointers in hash map
        std::swap(nodePtrs_[y->getId()], nodePtrs_[p->getId()]);
           
        size_t pIndex = p->getId();
        p->setId(y->getId());
        y->setId(pIndex);
        
        y = p;
        p = y->getParent();
    }
}

template <class _T, class _Priority, class _Comp>
typename PQBinomial::_NodePtr PQBinomial::binomialHeapMerge(_NodePtr first, _NodePtr second) {
    _NodePtr newHead;
    _NodePtr saved;
    if (!first && !second) {
        return nullptr;
    } else if (!first || !second) {
        return (first != nullptr) ? first : second;
    } else {
        if (first->getDegree() < second->getDegree()) {
            newHead = first;
            first = first->getSibling();
        } else {
            newHead = second;
            second = second->getSibling();
        }
    }
    saved = newHead;
    while (first && second) {
        if (first->getDegree() < second->getDegree()) {
            newHead->setSibling(first);
            newHead = newHead->getSibling();
            first = first->getSibling();
        } else {
            newHead->setSibling(second);
            newHead = newHead->getSibling();
            second = second->getSibling();
        }
    }
    while (first) {
        newHead->setSibling(first);
        newHead = newHead->getSibling();
        first = first->getSibling();
    }
    while (second) {
        newHead->setSibling(second);
        newHead = newHead->getSibling();
        second = second->getSibling();
    }
    return saved;
}

template <class _T, class _Priority, class _Comp>
void PQBinomial::binomialHeapUnionWithThis(_NodePtr heapHead) {
   _NodePtr head = binomialHeapMerge(head_, heapHead);
    if (!head) {
        return;
    }
    _NodePtr prevX;
    _NodePtr x = head;
    _NodePtr nextX = x->getSibling();
    while (nextX) {
        if (x->getDegree() != nextX->getDegree() ||
                (nextX->getSibling() != nullptr && nextX->getSibling()->getDegree() == x->getDegree())) {
            prevX = x;
            x = nextX;
        } else {
            if (comparer_(nextX->getPriority(), x->getPriority())) {
                x->setSibling(nextX->getSibling());
                binomialHeapLink(nextX, x);
            } else {
                if (!prevX) {
                    head = nextX;
                } else {
                    prevX->setSibling(nextX);
                }
                binomialHeapLink(x, nextX);
                x = nextX;
            }
        }
        nextX = x->getSibling();
    }
    head_ = head;
}

template <class _T, class _Priority, class _Comp>
typename PQBinomial::_NodePtr PQBinomial::copyNode(_NodePtr node, _NodePtr parent) {
    if (!node) {
        return nullptr;
    }
    _NodePtr result = std::make_shared<BinomialNode>(node->getKey(), node->getPriority());
    
    result->setParent(parent);
    result->setChild(copyNode(node->getChild(), result));
    result->setSibling(copyNode(node->getSibling(), parent));
    result->setDegree(node->getDegree());
    
    size_t id = base_++; 
    nodePtrs_[id] = result;
    result->setId(id);
    
    return result;
}

