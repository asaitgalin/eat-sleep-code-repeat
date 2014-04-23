#define PQBinary PriorityQueueBinary<_T, _Priority, _Comp>

template <class _T, class  _Priority, class _Comp>
PQBinary::PriorityQueueBinary(const std::vector<std::pair<_T, _Priority>> &items, 
                    const _Comp &compare) {
    base_ = 0;
    comparer_ = compare;
    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        data_.push_back(PriorityQueueNode<_T, _Priority>(it->first, it->second));
    }
    heapDestroyed_(std::make_shared<bool>(false));
    buildHeap();     
}

template <class _T, class  _Priority, class _Comp>
PQBinary::PriorityQueueBinary(const PQBinary &pq) {
    comparer_ = pq.comparer_;
    data_ = pq.data_;
    base_ = pq.base_; 
    positions_ = pq.positions_;
    adr_ = pq.adr_;
    heapDestroyed_ = std::make_shared<bool>(false);
}

template <class _T, class  _Priority, class _Comp>
PQBinary & PQBinary::operator= (const PQBinary &rhs) {
    if (&rhs != this) {
        comparer_ = rhs.comparer_;
        data_ = rhs.data_;
        base_ = rhs.base_; 
        positions_ = rhs.positions_;
        adr_ = rhs.adr_;
        heapDestroyed_ = std::make_shared<bool>(false);
    }
}

template <class _T, class _Priority, class _Comp>
std::shared_ptr<typename PQBinary::_BasePtr> PQBinary::insert(const _T &data, const _Priority &priority) {
    size_t id = base_++;
    size_t index = data_.size();
    data_.push_back(_NodeType(data, priority));
    positions_.push_back(id);
    adr_.insert(std::make_pair(id, index));
    siftUp(index);
    return std::make_shared<_BinaryPtr>(this, id);
}

template <class _T, class _Priority, class _Comp>
void PQBinary::extractTop() {
    swapNodes(0, data_.size() - 1);
    data_.pop_back();
    adr_.erase(positions_.back());
    positions_.pop_back();
    heapify(0);
}
    
template <class _T, class _Priority, class _Comp>
void PQBinary::updatePriority(std::shared_ptr<_BasePtr> pointer, const _Priority &newPriority) {
    if (!pointer) {
        throw std::runtime_error("update key error: null pointer");
    }
    if (!pointer->isValid()) {
        throw std::runtime_error("update key error: invalid pointer");
    }
    std::shared_ptr<_BinaryPtr> node = std::dynamic_pointer_cast<_BinaryPtr, _BasePtr>(pointer);
    if (!node || node->getParent() != this) {
        throw std::runtime_error("update key error: invalid pointer");
    }
    size_t dataIndex = adr_[node->getId()];
    if (comparer_(newPriority, data_[dataIndex].getPriority())) {
        throw std::invalid_argument("update key error: bad new priority");
    }            
    data_[dataIndex].setPriority(newPriority);
    siftUp(dataIndex);
}


template <class _T, class _Priority, class _Comp>
void PQBinary::buildHeap() {
    for (size_t i = data_.size() / 2 - 1; i >= 0; --i) {
        heapify(i);
    }
}

template <class _T, class _Priority, class _Comp>
void PQBinary::siftUp(size_t i) {
    size_t p = (i - 1) / 2;
    while (p < data_.size() && comparer_(data_[p].getPriority(), data_[i].getPriority())) {
        swapNodes(i, p);
        i = p;
        p = (i - 1) / 2;
    }
}

template <class _T, class _Priority, class _Comp>
void PQBinary::heapify(size_t i) {
    size_t l = 2 * i + 1;
    size_t r = 2 * i + 2;
    size_t swapIndex;
    while (r < data_.size()) {
        if (comparer_(data_[l].getPriority(), data_[i].getPriority()) 
                && comparer_(data_[r].getPriority(), data_[i].getPriority())) {
            break;
        }
        swapIndex = comparer_(data_[l].getPriority(), data_[r].getPriority()) 
            ? r : l;
        swapNodes(i, swapIndex);
        i = swapIndex;
        l = 2 * i + 1;
        r = 2 * i + 2;
    }
    if (l == data_.size() - 1 && comparer_(data_[i].getPriority(), 
                data_[l].getPriority())) {
        swapNodes(l, i);
    }
}

