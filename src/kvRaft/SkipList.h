//
// Created by epoch on 22-10-4.
//


#ifndef SKIPLIST_SKIPLIST_H
#define SKIPLIST_SKIPLIST_H

#include <memory>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <iostream>

using std::cout;
using std::endl;

/*
 * Skip Node;
 */

//forward declaration.
template<typename K, typename V>
class Node;

template<typename K, typename V>
class SkipList;

template<typename K, typename V>
using node_ptr = std::shared_ptr<Node<K, V>>;

template<typename K, typename V>
class Node{

public:
    using node_ptr = std::shared_ptr<Node<K, V>>;

    Node();
    Node(const K&, const V&, size_t level);
    ~Node();

    K get_key() const;
    V get_value() const;
    size_t get_level() const;


    friend SkipList<K, V>;

private:
    size_t level_;

    const K key_;
    V value_;
    std::vector<node_ptr> forward_;

    void unlink();
};

template<typename K, typename V>
Node<K, V>::Node() = default;

template<typename K, typename V>
Node<K, V>::Node(const K& k, const V& v, size_t level): key_(k), value_(v), level_(level), forward_(level_, nullptr) {}

template<typename K, typename V>
Node<K, V>::~Node() = default;

template<typename K, typename V>
K Node<K, V>::get_key() const { return key_; }

template<typename K, typename V>
V Node<K, V>::get_value() const { return value_; }

template<typename K, typename V>
size_t Node<K, V>::get_level() const {
    return level_;
}


template<typename K, typename V>
void Node<K, V>::unlink() {
    for (auto &p:forward_) {
        p = nullptr;
    }
}
/*
 * Skip list
 */
template<typename K, typename V>
class SkipList {

public:

    SkipList(size_t level);

    node_ptr<K, V> create_node(const K&, const V&, size_t level);

    bool search(const K&) const;
    void insert(const K&, const V&);

    node_ptr<K, V> erase(const K&);

    std::vector<node_ptr<K, V>> erase_all(const K&);
    std::vector<node_ptr<K, V>> erase_range(const K& lower, const K& upper);

    void print_list() const ;
    void print_level_size() const ;

    size_t size() const;
    size_t get_max_level() const;

private:
    node_ptr<K, V> search_ptr(const K&) const;

    std::vector<node_ptr<K, V>> get_previous_ptr(const K& k);

    size_t  get_random_level();

    //    size_t current_level_;
    //TODO: optimize it by using current level.
    size_t max_level_;

    std::atomic<size_t> element_count_;
    node_ptr<K, V> head_;

    /* concurrency support */
    mutable std::shared_mutex m;
};

template<typename K, typename V>
size_t SkipList<K, V>::get_max_level() const {
    return this->max_level_;
}


template<typename K, typename V>
node_ptr<K, V> SkipList<K, V>::create_node(const K &k, const V &v, size_t level) {
    return node_ptr<K, V>(new Node<K, V>(k, v, level));
}

template<typename K, typename V>
SkipList<K, V>::SkipList(size_t level): max_level_(level), element_count_(0) {
    //TODO: there is a little issue: k, v can be initialized by default constructor.
    //Solution: Use a Base node that has its default construcor.
    static_assert(std::is_default_constructible<K>(),"The Key type must has default constructor");
    static_assert(std::is_default_constructible<V>(), "The Key type must has default constructor");
    K k;
    V v;
    head_ = create_node(k, v, max_level_);
}


template<typename K, typename V>
node_ptr<K, V> SkipList<K, V>::search_ptr(const K &k) const {
    std::shared_lock<std::shared_mutex> sharedLock(m);
    node_ptr<K, V> prev = this->head_;
    for (ssize_t i = this->max_level_ - 1; i >= 0; --i) {
        while (prev->forward_[i] != nullptr && prev->forward_[i]->key_ < k) {
            prev = prev->forward_[i];
        }
        if (prev->forward_[i] != nullptr && prev->forward_[i]->key_ == k) {
            return prev;
        }
    }
    return nullptr;
}

template<typename K, typename V>
std::vector<node_ptr<K, V>> SkipList<K, V>::get_previous_ptr(const K &k) {

    std::vector<node_ptr<K, V>> prev(this->max_level_, this->head_);
    node_ptr<K, V> curr = this->head_;
    for (ssize_t i = this->max_level_ - 1; i >= 0; --i) {
        while (curr->forward_[i] != nullptr && curr->forward_[i]->key_ < k) {
            curr = curr->forward_[i];
        }
        prev[i] = curr;
    }
    return prev;

}

template<typename K, typename V>
bool SkipList<K, V>::search(const K &k) const {
    return search_ptr(k) != nullptr;
}


template<typename K, typename V>
void SkipList<K, V>::print_list() const {
    /* Read lock */
    std::shared_lock<std::shared_mutex> sharedLock(m);
    for (ssize_t i = this->max_level_ - 1; i >= 0; --i) {
         printf("Level %d : ", i);
         auto p = this->head_->forward_[i];
         while (p != nullptr) {
             std::cout << p->get_value() << ' ';
             p = p->forward_[i];
         }
         std::cout << std::endl;
     }
}

template<typename K, typename V>
void SkipList<K, V>::insert(const K &k, const V &v) {
    /*  write lock */
    std::unique_lock<std::shared_mutex> uniqueLock(m);
    node_ptr<K, V> node = create_node(k, v, get_random_level());
    auto previous_ptrs = get_previous_ptr(k);
    for (ssize_t i = node->get_level() - 1; i >= 0; --i) {
        node->forward_[i] = previous_ptrs[i]->forward_[i];
        previous_ptrs[i]->forward_[i] = node;
    }
    ++element_count_;
}


template<typename K, typename V>
size_t SkipList<K, V>::size() const {
    return this->element_count_;
}

template<typename K, typename V>
node_ptr<K, V> SkipList<K, V>::erase(const K& k) {
    /*  write lock */
    std::unique_lock<std::shared_mutex> uniqueLock(m);
    auto update = get_previous_ptr(k);

    /* not found key */
    if (update[0]->forward_[0] == nullptr || update[0]->forward_[0]->key_ != k) {
        return nullptr;
    }

    /* return the node ptr */
    auto return_ptr = update[0]->forward_[0];
    for (ssize_t i = 0; i < update.size(); ++i) {
        if (update[i]->forward_[i] != return_ptr) {
            break;
        }
        update[i]->forward_[i] = return_ptr->forward_[i];
    }
    --element_count_;
    return return_ptr;
}

template<typename K, typename V>
std::vector<node_ptr<K, V>> SkipList<K, V>::erase_all(const K& k) {
    /*  write lock */
    std::unique_lock<std::shared_mutex> uniqueLock(m);
    auto previous_ptr = get_previous_ptr(k);
    /* not found element*/
    if (previous_ptr[0]->forward_[0] == nullptr || previous_ptr[0]->forward_[0]->key_ != k) {
        return {};
    }

    /*create vector for erase ptr*/
    std::vector<node_ptr<K, V>> erase_ptrs;
    auto prev = previous_ptr[0];
    while (prev->forward_[0] != nullptr && prev->forward_[0]->key_ == k) {
        erase_ptrs.emplace_back(prev->forward_[0]);
        prev = prev->forward_[0];
    }
    for (ssize_t i = 0; i < this->max_level_; ++i) {
        if (previous_ptr[i]->forward_[i] == nullptr || previous_ptr[i]->forward_[i]->key_ != k) {
            break;
        }

        auto prev = previous_ptr[i];
        auto end = prev->forward_[i];
        while (end != nullptr && end->key_ == k) {
            end = end->forward_[i];
        }
        prev->forward_[i] = end;
    }
    this->element_count_ -= erase_ptrs.size();
    erase_ptrs.back()->unlink();
    return erase_ptrs;
}

template<typename K, typename V>
std::vector<node_ptr<K, V>> SkipList<K, V>::erase_range(const K &lower, const K &upper) {
    std::unique_lock<std::shared_mutex> uniqueLock(m);
    auto previous_ptr = get_previous_ptr(lower);

    /* not found element*/
    if (previous_ptr[0]->forward_[0] == nullptr || previous_ptr[0]->forward_[0]->key_ >= upper) {
        return {};
    }

    /*create vector for erase ptr*/
    std::vector<node_ptr<K, V>> erase_ptrs;
    auto prev = previous_ptr[0];
    while (prev->forward_[0] != nullptr && prev->forward_[0]->key_ < upper) {
        erase_ptrs.emplace_back(prev->forward_[0]);
        prev = prev->forward_[0];
    }
    for (ssize_t i = 0; i < this->max_level_; ++i) {
        if (previous_ptr[i]->forward_[i] == nullptr || previous_ptr[i]->forward_[i]->key_ >= upper) {
            break;
        }

        auto prev = previous_ptr[i];
        auto end = prev->forward_[i];
        while (end != nullptr && end->key_ < upper) {
            end = end->forward_[i];
        }
        prev->forward_[i] = end;
    }
    this->element_count_ -= erase_ptrs.size();
    erase_ptrs.back()->unlink();
    return erase_ptrs;
}


template<typename K, typename V>
size_t SkipList<K, V>::get_random_level() {
    size_t k = 1;
    while (rand() % 2) {
        k++;
    }
    return std::min(k, this->max_level_);
}


template<typename  K, typename V>
void SkipList<K, V>::print_level_size() const {
    /* Read lock */
    std::shared_lock<std::shared_mutex> sharedLock(m);
    for (ssize_t i = this->max_level_ - 1; i >= 0; --i) {
        int level_size = 0;
        auto p = head_;
        while (p->forward_[i] != nullptr) {
            ++level_size;
            p = p->forward_[i];
        }
        printf("Level %d: size-%d\n", i, level_size);
    }
}

#endif //SKIPLIST_SKIPLIST_H

