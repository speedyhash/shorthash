#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

enum class FillLimit { HALF, FULL };

enum class ProbeStatus { EMPTY, FULL, LONG };
using SuccessDistance = ::std::pair<ProbeStatus, ::std::size_t>;

template <typename Key, typename HashFamily,
          FillLimit FILL_LIMIT = FillLimit::HALF, size_t MAX_PROBE = 0>
struct HashSet {
    using size_t = ::std::size_t;
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
        for (size_t i = 0; (!MAX_PROBE) || (i < MAX_PROBE); ++i) {
            const size_t s = GetIndex(h, i);
            if (k == slots[s]) return {ProbeStatus::FULL, i};
            if (0 == slots[s]) {
                slots[s] = k;
                return {ProbeStatus::EMPTY, i};
            }
        }
        return {ProbeStatus::LONG, MAX_PROBE};
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
        if (!k) return 0;
        const size_t h = hasher_(k);
        for (size_t i = 0; true; ++i) {
            const size_t s = GetIndex(h, i);
            if (k == slots_[s]) return i;
        }
        __builtin_unreachable();
    }

    SuccessDistance Find(const Key k) const {
        if (!k) return {has_zero_ ? ProbeStatus::FULL : ProbeStatus::EMPTY, 0};
        const size_t h = hasher_(k);
        for (size_t i = 0; (!MAX_PROBE) || (i < MAX_PROBE); ++i) {
            const size_t s = GetIndex(h, i);
            if (k == slots_[s]) return {ProbeStatus::FULL, i};
            if (0 == slots_[s]) return {ProbeStatus::EMPTY, i};
        }
        return {ProbeStatus::LONG, MAX_PROBE};
    }

   public:
    template <typename T = HashSet>
    SuccessDistance Insert(const Key k, T* backup = nullptr) {
        if (0 == k) {
            const SuccessDistance result = {
                has_zero_ ? ProbeStatus::FULL : ProbeStatus::EMPTY, 0};
            has_zero_ = true;
            return result;
        }
        const auto result = InsertNonZeroWithoutResize(k, slots_.get());
        if (result.first == ProbeStatus::EMPTY) {
            ++size_;
            if (Full()) Upsize(backup);
        }
        return result;
    }

   protected:
  template<typename T>
    void Upsize(T * backup) {
        decltype(slots_) new_slots(new Key[2 * capacity_]());
        capacity_ *= 2;
        mask_ = capacity_ - 1;
        --hasher_.shift;
        for (size_t i = 0; i < capacity_ / 2; ++i) {
            if (0 != slots_[i]) {
                const auto result =
                    InsertNonZeroWithoutResize(slots_[i], new_slots.get());
                if (result.first == ProbeStatus::LONG) {
                  --size_;
                  backup->Insert(slots_[i]);
                }
            }
        }
        ::std::swap(slots_, new_slots);
    }
};

template <typename Key, typename FastHashFamily, typename SlowHashFamily,
          size_t MAX_PROBE = 4>
struct SplitHashSet {
    using size_t = ::std::size_t;

    HashSet<Key, FastHashFamily, FillLimit::HALF, MAX_PROBE> fast_;
    HashSet<Key, SlowHashFamily, FillLimit::HALF> slow_;

    SplitHashSet(size_t log_capacity = 3)
        : fast_(log_capacity), slow_(log_capacity) {}

    static ::std::string Name() {
        ::std::ostringstream ss;
        ss << FastHashFamily::NAME << "+" << SlowHashFamily::NAME << " "
           << MAX_PROBE;
        return ss.str();
    }

    size_t Ndv() const { return fast_.Ndv() + slow_.Ndv(); }

    size_t FindPresent(const Key k) const {
        const auto easy = fast_.Find(k);
        if (easy.first != ProbeStatus::LONG) return easy.second;
        return slow_.FindPresent(k);
    }

     SuccessDistance Insert(const Key k) {
       const auto easy = fast_.Insert(k, &slow_);
        if (easy.first != ProbeStatus::LONG) return easy;
        return slow_.Insert(k);
    }
};
