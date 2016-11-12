#pragma once

#include <memory>
#include <string>
#include <utility>

enum class FillLimit { HALF, FULL };

template <typename Key, typename HashFamily,
          FillLimit FILL_LIMIT = FillLimit::HALF>
struct HashSet {
    using size_t = ::std::size_t;
    using SuccessDistance = ::std::pair<bool, size_t>;
    HashFamily hasher_;
    size_t size_, capacity_, mask_;
    ::std::unique_ptr<Key[]> slots_;
    bool has_zero_;

    HashSet(size_t log_capacity = 3)
        : hasher_(),
          size_(0),
          capacity_(1ull << log_capacity),
          mask_(capacity_ - 1),
          slots_(new Key[capacity_]()),
          has_zero_(false) {
        hasher_.shift = sizeof(size_t) * CHAR_BIT - log_capacity;
    }

    static ::std::string Name() {
        return ::std::string("LP ") + HashFamily::NAME;
    }

    size_t Ndv() const { return size_ + has_zero_; }

   protected:
    size_t GetIndex(size_t hash_value, size_t offset) const {
        return mask_ & (hash_value + offset);
    }

    SuccessDistance InsertNonZeroWithoutResize(const Key k, Key slots[]) const {
        const size_t h = hasher_(k);
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

    bool Full() const {
        if (FILL_LIMIT == FillLimit::HALF) {
            return size_ > capacity_ / 2;
        } else {
            return size_ >= capacity_;
        }
    }

   public:
    size_t FindPresent(const Key k) const {
        const size_t h = hasher_(k);
        for (size_t i = 0; true; ++i) {
            const size_t s = GetIndex(h, i);
            if (k == slots_[s]) return i;
        }
        __builtin_unreachable();
    }

    SuccessDistance Insert(const Key k) {
        if (0 == k) {
            has_zero_ = true;
            return {true, 0};
        }
        const auto result = InsertNonZeroWithoutResize(k, slots_.get());
        if (result.first) {
            ++size_;
            if (Full()) Upsize();
        }
        return result;
    }

    void Upsize() {
        decltype(slots_) new_slots(new Key[2 * capacity_]());
        capacity_ *= 2;
        mask_ = capacity_ - 1;
        --hasher_.shift;
        for (size_t i = 0; i < capacity_ / 2; ++i) {
            if (0 != slots_[i]) {
                InsertNonZeroWithoutResize(slots_[i], new_slots.get());
            }
        }
        ::std::swap(slots_, new_slots);
    }
};
