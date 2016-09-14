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

/*
Heavily modified by D. Lemire
*/


#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace rigtorp {

template <typename Key, typename T, typename Hash = std::hash<Key>, bool robinhood_ = true>
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
    HashMap(size_type bucket_count_var, key_type empty_key, float loadFactor) : empty_key_(empty_key), loadFactor_(loadFactor) {
        size_t pow2 = 1;
        while (pow2 < bucket_count_var) {
            pow2 <<= 1;
        }
        buckets_.resize(pow2, std::make_pair(empty_key_, T()));
    }

    HashMap(size_type bucket_count_var, key_type empty_key, float loadFactor, hasher h)
        : empty_key_(empty_key), loadFactor_(loadFactor), hasher_(h) {
        size_t pow2 = 1;
        while (pow2 < bucket_count_var) {
            pow2 <<= 1;
        }
        buckets_.resize(pow2, std::make_pair(empty_key_, T()));
    }

    HashMap(const HashMap &other, size_type bucket_count_var)
        : HashMap(bucket_count_var, other.empty_key_, other.loadFactor_, other.hasher_) {
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

    template <class MT>
    std::pair<iterator, bool> emplace(key_type key, MT value) {
        if(key == empty_key_) throw std::runtime_error("can't use the empty key.");
        reserve(size_ + 1);
        size_t ideal = key_to_idx(key);
        size_t keyhash;
        for (size_t idx = ideal;; idx = probe_next(idx)) {
            if (buckets_[idx].first == empty_key_) {

                buckets_[idx].second = value;
                buckets_[idx].first = key;
                size_++;
                return {iterator(this, idx), true};
            } else if (buckets_[idx].first == key) {
                return {iterator(this, idx), false};
            }
            if(robinhood_) {
              keyhash = key_to_idx(buckets_[idx].first);
              if  (diff(idx,keyhash) < diff(idx,ideal)) {// Robin-Hood
                  ideal = keyhash;
                  key_type tmpk = buckets_[idx].first;
                  buckets_[idx].first = key;
                  key = tmpk;
                  MT tmpv = std::move(buckets_[idx].second);
                  buckets_[idx].second = std::move(value);
                  value = std::move(tmpv);
              }
            }
        }
    };

    // check that this key is properly set (DEBUG function)
    bool sanity_check_key(key_type key) {
        if(key == empty_key_) throw std::runtime_error("can't check the empty key.");
        size_t ideal = key_to_idx(key);
        size_t expected_probes = probed_keys(key);
        size_t counted_probes = 0;

        for (size_t idx = ideal;; idx = probe_next(idx)) {
            counted_probes++;
            if(counted_probes != diff(idx,ideal) + 1) {
              printf("ideal = %zu counted_probes = %zu current index = %zu capacity = %zu diff(idx,ideal) = %zu ", ideal, counted_probes, idx, buckets_.size(), diff(idx,ideal));
            }
            assert(counted_probes == diff(idx,ideal) + 1);
            if (buckets_[idx].first == key) {
                assert(counted_probes == expected_probes);
                return true;
            }
            if (buckets_[idx].first == empty_key_) {
                fprintf(stderr, "An error occured, I should not have encountered an empty key.\n");
                return false;
            }
            if (robinhood_ && (diff(idx,key_to_idx(buckets_[idx].first)) < diff(idx,ideal))) {// Robin-Hood
                fprintf(stderr, "An error occured, Robin-Hood order is broken. Was checking key %zu with an ideal of %zu. Made it index %zu found value with an ideal of %zu .\n", (size_t)key, (size_t)ideal,(size_t)idx, (size_t)key_to_idx(buckets_[idx].first));
                return false;
            }
        }
    }


    // this checks the status of the hash table (DEBUG function)
    bool sanity_check() {
        for (size_t idx = 0; idx < buckets_.size(); idx += 1) {
            if (buckets_[idx].first == empty_key_) {
              continue;
            }
            if(!sanity_check_key(buckets_[idx].first)) {
              return false;
            }
        }
        return true; // we are good.
    }

    void erase(iterator it) {
        assert(! robinhood_); // todo: add robin-hood hashing
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
        assert(! robinhood_); // todo: add robin-hood hashing
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
/*
    mapped_type &operator[](key_type key) {
        return emplace(key).first->second;
    }*/

    size_type count(key_type key) const {
        return find(key) == end() ? 0 : 1;
    }

    iterator find(key_type key) {
        if(key == empty_key_) throw std::runtime_error("can't seek the empty key.");
        for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) {
            if (buckets_[idx].first == key) {
                return iterator(this, idx);
            }
            if (buckets_[idx].first == empty_key_) {
                return end();
            }
            // todo : could optimize for robin-hood hashing if unsuccessful searches are expected
        }

    }

    // return how many keys are probed
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
    void rehash(size_type count_var) {
        assert(count_var > size());
        count_var = std::max(count_var, size());
        HashMap other(*this, count_var);
        swap(other);
    }

    void reserve(size_type count_var) {
        if (count_var > buckets_.size() ) {
            rehash( count_var /  loadFactor_ + 1);
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
        const size_t mask = buckets_.size() - 1;
        return hasher_(key) & mask;
    }

    size_t probe_next(size_t idx) const {
        const size_t mask = buckets_.size() - 1;
        return (idx + 1) & mask;
    }


    size_t diff(size_t a, size_t b) const {
        const size_t mask = buckets_.size() - 1;
        return (buckets_.size() + (a - b)) & mask;
    }

    key_type empty_key_;
    const float loadFactor_;
    buckets buckets_;
    size_t size_ = 0;
    const hasher  hasher_;
};
}
