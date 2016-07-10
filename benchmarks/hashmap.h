/*
Copyright (c) 2016 Erik Rigtorp <erik@rigtorp.se>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */



#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace rigtorp {

template <typename Key, typename T, typename Hash = std::hash<Key>>
class HashMap {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key, T>;
    using size_type = std::size_t;
    using hasher = Hash;
    using reference = value_type &;
    using const_reference = const value_type &;
    using buckets = std::vector<value_type>;

    template <typename ContT, typename IterVal> struct hm_iterator {
        using value_type = IterVal;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::forward_iterator_tag;

        bool operator==(const hm_iterator &other) const {
            return other.hm_ == hm_ && other.idx_ == idx_;
        }
        bool operator!=(const hm_iterator &other) {
            return !(other == *this);
        }

        hm_iterator &operator++() {
            ++idx_;
            advance_past_empty();
            return *this;
        }

        reference operator*() const {
            return hm_->buckets_[idx_];
        }
        pointer operator->() const {
            return &hm_->buckets_[idx_];
        }

    private:
        explicit hm_iterator(ContT *hm) : hm_(hm) {
            advance_past_empty();
        }
        explicit hm_iterator(ContT *hm, size_type idx) : hm_(hm), idx_(idx) {}
        template <typename OtherContT, typename OtherIterVal>
        hm_iterator(const hm_iterator<OtherContT, OtherIterVal> &other)
            : hm_(other.hm_), idx_(other.idx_) {}

        void advance_past_empty() {
            while (idx_ < hm_->buckets_.size() &&
                    hm_->buckets_[idx_].first == hm_->empty_key_) {
                ++idx_;
            }
        }

        ContT *hm_ = nullptr;
        typename ContT::size_type idx_ = 0;
        friend ContT;
    };

    using iterator = hm_iterator<HashMap, value_type>;
    using const_iterator = hm_iterator<const HashMap, const value_type>;

public:
    HashMap(size_type bucket_count, key_type empty_key, float loadFactor) : empty_key_(empty_key), loadFactor_(loadFactor) {
        size_t pow2 = 1;
        while (pow2 < bucket_count) {
            pow2 <<= 1;
        }
        buckets_.resize(pow2, std::make_pair(empty_key_, T()));
    }

    HashMap(size_type bucket_count, key_type empty_key, float loadFactor, hasher h)
        : empty_key_(empty_key), loadFactor_(loadFactor), hasher_(h) {
        size_t pow2 = 1;
        while (pow2 < bucket_count) {
            pow2 <<= 1;
        }
        buckets_.resize(pow2, std::make_pair(empty_key_, T()));
    }

    HashMap(const HashMap &other, size_type bucket_count)
        : HashMap(bucket_count, other.empty_key_, other.loadFactor_, other.hasher_) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            insert(*it);
        }
    }

    // Iterators
    iterator begin() {
        return iterator(this);
    }

    const_iterator begin() const {
        return const_iterator(this);
    }

    iterator end() {
        return iterator(this, buckets_.size());
    }

    const_iterator end() const {
        return const_iterator(this, buckets_.size());
    }

    // Capacity
    bool empty() const {
        return size() == 0;
    }
    size_type size() const {
        return size_;
    }
    size_type max_size() const {
        return std::numeric_limits<size_type>::max();
    }

    // Modifiers
    void clear() {
        HashMap other(bucket_count(), empty_key_);
        swap(other);
    }

    std::pair<iterator, bool> insert(const value_type &value) {
        return emplace(value.first, value.second);
    };

    std::pair<iterator, bool> insert(value_type &&value) {
        return emplace(value.first, std::move(value.second));
    };

    template <typename... Args>
    std::pair<iterator, bool> emplace(key_type key, Args &&... args) {
        reserve(size_ + 1);
        for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) {
            if (buckets_[idx].first == empty_key_) {
                buckets_[idx].second = mapped_type(std::forward<Args>(args)...);
                buckets_[idx].first = key;
                size_++;
                return {iterator(this, idx), true};
            } else if (buckets_[idx].first == key) {
                return {iterator(this, idx), false};
            }
        }
    };

    void erase(iterator it) {
        size_t bucket = it.idx_;
        for (size_t idx = probe_next(bucket);; idx = probe_next(idx)) {
            if (buckets_[idx].first == empty_key_) {
                buckets_[bucket].first = empty_key_;
                size_--;
                return;
            }
            size_t ideal = key_to_idx(buckets_[idx].first);
            if (diff(bucket, ideal) < diff(idx, ideal)) {
                // swap, bucket is closer to ideal than idx
                buckets_[bucket] = buckets_[idx];
                bucket = idx;
            }
        }
    }

    size_type erase(const key_type key) {
        auto it = find(key);
        if (it != end()) {
            erase(it);
            return 1;
        }
        return 0;
    }

    void swap(HashMap &other) {
        std::swap(buckets_, other.buckets_);
        std::swap(size_, other.size_);
        std::swap(empty_key_, other.empty_key_);
    }

    // Lookup
    mapped_type &at(key_type key) {
        iterator it = find(key);
        if (it != end()) {
            return it->second;
        }
        throw std::out_of_range("HashMap::at");
    }

    const mapped_type &at(key_type key) const {
        return at(key);
    }

    mapped_type &operator[](key_type key) {
        return emplace(key).first->second;
    }

    size_type count(key_type key) const {
        return find(key) == end() ? 0 : 1;
    }

    iterator find(key_type key) {
        for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) {
            if (buckets_[idx].first == key) {
                return iterator(this, idx);
            }
            if (buckets_[idx].first == empty_key_) {
                return end();
            }
        }
    }


    int probed_keys(key_type key) {
        int counter = 0;
        for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) {
            counter ++;
            if (buckets_[idx].first == key) {
                return counter;
            }
            if (buckets_[idx].first == empty_key_) {
                return counter;
            }
        }
    }

    const_iterator find(key_type key) const {
        return const_cast<HashMap *>(this)->find(key);
    }

    // Bucket interface
    size_type bucket_count() const {
        return buckets_.size();
    }

    // Hash policy
    void rehash(size_type count) {
        count = std::max(count, size());
        HashMap other(*this, count);
        swap(other);
    }

    void reserve(size_type count) {
        if (count > buckets_.size() ) {
            rehash( count /  loadFactor_ + 1);
        }
    }

    float load() {
        return size_ * 1.0 / buckets_.size();
    }

    // Observers
    hasher hash_function() const {
        return hasher_;
    }

private:

    size_t key_to_idx(key_type key) const {
        /* was:  const size_t mask = buckets_.size() - 1;
          return hasher_(key) & mask;
        */
        const uint64_t hash32 = hasher_(key) & UINT64_C(0xFFFFFFFF);
        const uint64_t siz32 = buckets_.size();
        return ( hash32 * siz32 ) >> 32; // see http://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
    }

    size_t probe_next(size_t idx) const {
        /* was:  const size_t mask = buckets_.size() - 1;
         return (idx + 1) & mask; */
        size_t nextidx = idx + 1;
        if(nextidx == buckets_.size()) { // branch rarely taken (cheap)
          return 0;
        }
        return nextidx;
    }


    size_t diff(size_t a, size_t b) const {
        /** was : const size_t mask = buckets_.size() - 1;
        return (buckets_.size() + (a - b)) & mask; */
        const size_t siz = buckets_.size();
        const size_t d = a - b + size;// between 0 and 2 size -1
        if(d >= buckets_.size()) return d - siz;
        return d;
    }

    key_type empty_key_;
    float loadFactor_;
    buckets buckets_;
    size_t size_ = 0;
    hasher  hasher_;
};
}
