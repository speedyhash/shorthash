#include <cassert>
#include <memory>
#include <utility>

template <typename Key, typename HashFamily>
struct HashSet {
    HashFamily hasher;
    ::std::size_t size, capacity, mask;
    ::std::unique_ptr<Key[]> slots;
    bool has_zero;

    HashSet(::std::size_t log_capacity = 3)
        : hasher(),
          size(0),
          capacity(1ull << log_capacity),
          mask(capacity - 1),
          slots(new Key[capacity]()),
          has_zero(false) {}

    static ::std::pair<bool, ::std::size_t> InsertNonZeroWithoutResize(
        const HashFamily &hasher, const ::std::size_t capacity,
        const ::std::size_t mask, Key slots[], const Key k) {
        const size_t h = hasher(k);
        for (size_t i = 0; i < capacity; ++i) {
            const size_t s = (h + i) & mask;
            if (k == slots[s]) return ::std::make_pair(false, i);
            if (0 == slots[s]) {
                slots[s] = k;
                return ::std::make_pair(true, i);
            }
        }
        throw std::runtime_error("could not insert key");  // should we ever
                                                           // make it here, then
                                                           // there was no room
    }

    ::std::pair<bool, ::std::size_t> Insert(const Key k) {
        if (0 == k) {
            has_zero = true;
            return ::std::make_pair(true, 0);
        }
        const auto result =
            InsertNonZeroWithoutResize(hasher, capacity, mask, slots.get(), k);
        if (result.first) {
            ++size;
            if (size > capacity / 2) Upsize();
        }
        return result;
    }

    void Upsize() {
        ::std::unique_ptr<Key[]> new_slots(new Key[2 * capacity]());
        capacity *= 2;
        mask = capacity - 1;
        for (::std::size_t i = 0; i < capacity / 2; ++i) {
            if (0 != slots[i]) {
                InsertNonZeroWithoutResize(hasher, capacity, mask,
                                           new_slots.get(), slots[i]);
            }
        }
        ::std::swap(slots, new_slots);
    }

    ::std::pair<bool, ::std::size_t> Find(const Key k) const {
        if (0 == k) {
            return ::std::make_pair(has_zero, 1);
        }
        const size_t h = hasher(k);
        for (size_t i = 0; i < capacity; ++i) {
            const size_t s = (h + i) & mask;
            if (k == slots[s]) return ::std::make_pair(true, 1 + i);
            if (0 == slots[s]) return ::std::make_pair(false, 1 + i);
        }
        return ::std::make_pair(false, capacity);
    }

    ::std::size_t FindPresentWithOffset(const Key k,
                                        const size_t offset) const {
        if (0 == k) {
            return 1;
        }
        const size_t h = hasher(k) - offset;
        for (size_t i = 0; i < capacity; ++i) {
            const size_t s = (h + i) & mask;
            if (k == slots[s]) return 1 + i;
        }
        return capacity;
    }
};
