#pragma once

#include <climits>
#include <memory>
#include <utility>
#include <vector>

template <typename Key, typename HashFamily>
struct SepChain {
    using size_t = ::std::size_t;
    using SuccessDistance = ::std::pair<bool, size_t>;
    using Chain = ::std::vector<Key>;
    HashFamily hasher_;
    size_t size_, capacity_, mask_;
    ::std::unique_ptr<Chain[]> slots_;

    SepChain(size_t log_capacity = 3)
        : hasher_(),
          size_(0),
          capacity_(1ull << log_capacity),
          mask_(capacity_ - 1),
          slots_(new Chain[capacity_]()) {
        hasher_.shift = sizeof(size_t) * CHAR_BIT - log_capacity;
    }

    static ::std::string Name() {
        return ::std::string("SC ") + HashFamily::NAME;
    }

    size_t Ndv() const { return size_; }

   protected:
    size_t GetIndex(const Key k) const {
        if (HashFamily::NEEDS_MASK) return hasher_(k) & mask_;
        return hasher_(k);
    }

    SuccessDistance InsertNonZeroWithoutResize(const Key k,
                                               Chain slots[]) const {
        const size_t h = GetIndex(k);
        for (size_t i = 0; i < slots[h].size(); ++i) {
            if (k == slots[h][i]) return {false, i};
        }
        slots[h].push_back(k);
        return {true, slots[h].size()};
    }

    bool Full() const { return size_ >= capacity_; }

   public:
    size_t FindPresent(const Key k) const {
        const size_t h = GetIndex(k);
        return std::find(slots_[h].begin(), slots_[h].end(), k) -
               slots_[h].begin();
    }

    SuccessDistance Insert(const Key k) {
        const auto result = InsertNonZeroWithoutResize(k, slots_.get());
        if (result.first) {
            ++size_;
            if (Full()) Upsize();
        }
        return result;
    }

    void Upsize() {
        decltype(slots_) new_slots(new Chain[2 * capacity_]());
        capacity_ *= 2;
        mask_ = capacity_ - 1;
        --hasher_.shift;
        for (size_t i = 0; i < capacity_ / 2; ++i) {
            for (Key k : slots_[i]) {
                InsertNonZeroWithoutResize(k, new_slots.get());
            }
        }
        ::std::swap(slots_, new_slots);
    }
};
