#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

// An introspective hash set that rehashes when the probe length gets
// long, even if the set is not full and therefore doesn't need to
// double in capacity.
template <typename Key, typename HashFamily>
struct ReHashSet {
    using size_t = ::std::size_t;
    using SuccessDistance = ::std::pair<bool, size_t>;
    typename HashFamily::Randomness hasher_;
    size_t size_, capacity_, mask_, probe_distance_;
    int shift_;
    ::std::unique_ptr<Key[]> slots_;
    bool has_zero_;

    ReHashSet(size_t log_capacity = 3)
        : hasher_(),
          size_(0),
          capacity_(1ull << log_capacity),
          mask_(capacity_ - 1),
          probe_distance_(0),
          shift_(sizeof(size_t) * CHAR_BIT - log_capacity),
          slots_(new Key[capacity_]()),
          has_zero_(false) {
        HashFamily::InitRandomness(&hasher_);
    }

    static ::std::string Name() {
        return ::std::string("ReLP-") + HashFamily::NAME;
    }

    size_t Ndv() const { return size_ + has_zero_; }

   protected:
    size_t GetIndex(size_t hash_value, size_t offset) const {
        return mask_ & (hash_value + offset);
    }

    SuccessDistance InsertNonZeroWithoutResize(const Key k, Key slots[]) const {
        const size_t h = HashFamily::HashFunction(k, &hasher_, shift_);
        for (size_t i = 0; true; ++i) {
            const size_t s = GetIndex(h, i);
            if (k == slots[s]) return {false, i};
            if (0 == slots[s]) {
                slots[s] = k;
                return {true, i};
            }
        }
        __builtin_unreachable();
    }

    bool Full() const { return size_ > capacity_ / 2; }
    bool Crowded() const { return probe_distance_ > capacity_ * 2; }

   public:
    size_t FindPresent(const Key k) const {
        if (!k) return 0;
        const size_t h = HashFamily::HashFunction(k, &hasher_, shift_);
        for (size_t i = 0; true; ++i) {
            const size_t s = GetIndex(h, i);
            if (k == slots_[s]) return i;
        }
        __builtin_unreachable();
    }

   public:
    SuccessDistance Insert(const Key k) {
        if (0 == k) {
            const SuccessDistance result = {!has_zero_, 0};
            has_zero_ = true;
            return result;
        }
        const auto result = InsertNonZeroWithoutResize(k, slots_.get());
        if (result.first) {
            ++size_;
            probe_distance_ += result.second;
            if (Full()) {
                Resize(capacity_ * 2);
            } else if (Crowded()) {
                Resize(capacity_);
            }
        }
        return result;
    }

   protected:
    void Resize(size_t new_capacity) {
        const size_t old_capacity = capacity_;
        capacity_ = new_capacity;
        if (old_capacity == capacity_) {
            HashFamily::InitRandomness(&hasher_);
        }
        mask_ = capacity_ - 1;
        shift_ -= (old_capacity != capacity_);
        probe_distance_ = 0;
        decltype(slots_) new_slots(new Key[capacity_]());
        for (size_t i = 0; i < old_capacity; ++i) {
            if (0 != slots_[i]) {
                const auto result =
                    InsertNonZeroWithoutResize(slots_[i], new_slots.get());
                probe_distance_ += result.second;
                if (Crowded()) {
                    Resize(capacity_);
                    return;
                }
            }
        }
        ::std::swap(slots_, new_slots);
    }
};
