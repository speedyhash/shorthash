#include <cassert>
#include <memory>
#include <utility>

template <typename Key, typename HashFamily, bool UseFullCapacity = false,
          bool UseHighBits = false>
struct HashSet {
    HashFamily hasher;
    ::std::size_t size, capacity, mask, log_capacity;
    ::std::unique_ptr<Key[]> slots;
    bool has_zero;

    HashSet(::std::size_t log_capacity_param = 3)
        : hasher(),
          size(0),
          capacity(1ull << log_capacity_param),
          mask(capacity - 1),
          log_capacity(log_capacity_param),
          slots(new Key[capacity]()),
          has_zero(false) {}

    static ::std::pair<bool, ::std::size_t> InsertNonZeroWithoutResize(
        const HashFamily &hasher, const ::std::size_t capacity,
        const ::std::size_t mask, const ::std::size_t log_capacity, Key slots[],
        const Key k) {
        const size_t h = hasher(k);
        for (size_t i = 0; i < capacity; ++i) {
            const size_t s =
                mask & (i + (UseHighBits ? (h >> (64 - log_capacity)) : h));
            if (k == slots[s]) return ::std::make_pair(false, i);
            if (0 == slots[s]) {
                slots[s] = k;
                return ::std::make_pair(true, i);
            }
        }
        __builtin_unreachable();
        throw std::runtime_error("could not insert key");  // should we ever
                                                           // make it here, then
                                                           // there was no room
    }

  ::std::size_t FindPresentWithOffset(const Key k, ::std::size_t offset) const {
        const size_t h = hasher(k);
        for (size_t i = 0; i < capacity; ++i) {
            const size_t s =
                mask &
                (i + (UseHighBits ? (h >> (64 - log_capacity)) : h) - offset);
            if (k == slots[s]) return i;
        }
        __builtin_unreachable();
        throw std::runtime_error("could not find key");  // should we ever
                                                         // make it here, then
                                                         // there was no room
    }



    ::std::pair<bool, ::std::size_t> Insert(const Key k) {
        if (0 == k) {
            has_zero = true;
            return ::std::make_pair(true, 0);
        }
        const auto result = InsertNonZeroWithoutResize(
            hasher, capacity, mask, log_capacity, slots.get(), k);
        if (result.first) {
            ++size;
            if (size >= (capacity/(UseFullCapacity ? 1 : 2))) Upsize();
        }
        return result;
    }

    void Upsize() {
        ::std::unique_ptr<Key[]> new_slots(new Key[2 * capacity]());
        capacity *= 2;
        mask = capacity - 1;
        log_capacity++;
        for (::std::size_t i = 0; i < capacity / 2; ++i) {
            if (0 != slots[i]) {
                InsertNonZeroWithoutResize(hasher, capacity, mask, log_capacity,
                                           new_slots.get(), slots[i]);
            }
        }
        ::std::swap(slots, new_slots);
    }

};
