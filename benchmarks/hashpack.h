#pragma once

#include <memory>

extern "C" {
#include "clhash.h"
#include "cw-trick.h"
#include "multiply-shift.h"
#include "tabulated.h"
#include "crc.h"
#include "murmur.h"
#include "fnv.h"
#include "linear.h"
#include "identity.h"
#include "oddmultiply.h"
#include "siphash24.h"
}

enum class HashBits { LOW, HIGH };

template <typename WordP, typename RandomnessP,
          void (*InitRandomnessP)(RandomnessP *),
          WordP (*HashFunctionP)(WordP, const RandomnessP *),
          HashBits HASH_BITS = HashBits::LOW>
class GenericPack {
   public:
    typedef WordP Word;
    typedef RandomnessP Randomness;

    static constexpr bool NEEDS_MASK = HASH_BITS == HashBits::LOW;

    static inline void InitRandomness(Randomness *r) { InitRandomnessP(r); }

    __attribute__((always_inline)) static inline Word
    HashFunction(Word x, const Randomness *r, const int shift) {
        if (HASH_BITS == HashBits::LOW) return HashFunctionP(x, r);
        return HashFunctionP(x, r) >> shift;
    }

    __attribute__((always_inline)) static inline Word
    HashFunction(Word x, const Randomness *r) {
        return HashFunctionP(x, r);
    }

    /**
    * We want GenericPack to be usable as a C++ hasher
    */
    GenericPack() : shift(0), myr(NULL) {
        myr = std::shared_ptr<Randomness>(new Randomness());
        if (myr != NULL) {
            InitRandomness(myr.get());
        }
    }

    GenericPack(const GenericPack &o) : myr(o.myr) {}

    __attribute__((always_inline)) inline size_t operator()(Word x) const {
        if (HASH_BITS == HashBits::LOW) return HashFunction(x, myr.get());
        return HashFunction(x, myr.get()) >> shift;
    }

    int shift;

   protected:
    std::shared_ptr<Randomness> myr;

};

struct SipPack {
    static constexpr auto NAME = "SipHash";
    typedef uint64_t Word;
    typedef unsigned __int128 Randomness;
    static inline void InitRandomness(Randomness *r) {
        uint64_t * rdata = reinterpret_cast<uint64_t *>(r);
        rdata[0] = get64rand();
        rdata[1] = get64rand();
    }

    __attribute__((always_inline))
    static inline Word HashFunction(Word x, const Randomness *r) {
        Word result;
        siphash(reinterpret_cast<uint8_t *>(&result),
                reinterpret_cast<const uint8_t *>(&x),
                reinterpret_cast<const uint8_t *>(r));
        return result;
    }

    SipPack() : myr(NULL) {
        myr = std::shared_ptr<Randomness>(new Randomness());
        if (myr != NULL) {
            InitRandomness(myr.get());
        }
    }

    SipPack(const SipPack &o) : myr(o.myr) {}

    __attribute__((always_inline)) inline size_t operator()(Word x) const {
        return HashFunction(x, myr.get());
    }

    int shift;

   protected:
    std::shared_ptr<Randomness> myr;
};

struct Identity64Pack
        : public GenericPack<uint64_t, identity_t, identity_init, identity64> {
    static constexpr auto NAME = "Identity64";
};


struct Cyclic64Pack
        : public GenericPack<uint64_t, cyclic_t, cyclic_init, cyclic> {
    static constexpr auto NAME = "Cyclic64";
};

struct Cyclic32Pack
        : public GenericPack<uint32_t, cyclic32_t, cyclic32_init, cyclic32> {
    static constexpr auto NAME = "Cyclic32";
};


struct CRC32_64Pack
        : public GenericPack<uint64_t, CRCRandomness, CRCInit, CRC32_64> {
    static constexpr auto NAME = "CRC32_64";
};



 struct CRC32Pack
        : public GenericPack<uint32_t, CRCRandomness, CRCInit, CRC32> {
     static constexpr auto NAME = "CRC32";
 };

struct Murmur32Pack
        : public GenericPack<uint32_t, murmur32_t, murmur32_init, murmur32> {
    static constexpr auto NAME = "Murmur32";
};

struct Murmur64Pack
        : public GenericPack<uint64_t, murmur64_t, murmur64_init, murmur64> {
    static constexpr auto NAME = "Murmur64";
};

struct FNV64Pack
        : public GenericPack<uint64_t, fnv64_t, fnv64_init, fnv64> {
    static constexpr auto NAME = "FNV64";
};


struct Stafford64Pack
        : public GenericPack<uint64_t, murmur64_t, staffordmix01_init, murmur64> {
    static constexpr auto NAME = "stafford64";
};


struct xxHash64Pack
        : public GenericPack<uint64_t, murmur64_t, xxhash_init, murmur64> {
    static constexpr auto NAME = "xxHash64";
};


struct ReversedOddMultiply64Pack
        : public GenericPack<uint64_t, oddmultiply64_t, oddmultiply64_init, reversed_oddmultiply64> {
    static constexpr auto NAME = "ReversedOddMultiply64";
};

struct Koloboke64Pack
        : public GenericPack<uint64_t, koloboke_t, koloboke_init, koloboke64> {
    static constexpr auto NAME = "Koloboke64";
};

struct RandomKoloboke64Pack
    : public GenericPack<uint64_t, random_koloboke_t, random_koloboke_init,
                         random_koloboke64> {
    static constexpr auto NAME = "RandKolo64";
};

struct RandomWeakKoloboke64Pack
    : public GenericPack<uint64_t, random_weak_koloboke_t,
                         random_weak_koloboke_init, random_weak_koloboke64> {
    static constexpr auto NAME = "RWKolo64";
};

struct Zobrist64Pack
        : public GenericPack<uint64_t, zobrist_t, zobrist_init, zobrist> {
    static constexpr auto NAME = "Zobrist64";
};

struct WZobrist64Pack
        : public GenericPack<uint64_t, wzobrist_t, wzobrist_init, wzobrist> {
    static constexpr auto NAME = "WZob64";
};


struct ThorupZhang64Pack
        : public GenericPack<uint64_t, thorupzhang_t, thorupzhang_init, thorupzhang> {
    static constexpr auto NAME = "TZ64";
};

struct ZobristTranspose64Pack
        : public GenericPack<uint64_t, zobrist_flat_t, zobrist_flat_init,
          zobrist_flat_transpose> {
    static constexpr auto NAME = "Transposed64";
};


struct BitMixing64Pack
        : public GenericPack<uint64_t, bitmixing_t, bitmixing_init, bitmixing> {
    static constexpr auto NAME = "BM64";
};

struct ClBitMixing64Pack
        : public GenericPack<uint64_t, cl_bitmixing_t, cl_bitmixing_init, cl_bitmixing> {
    static constexpr auto NAME = "ClBM64";
};

struct ClLinear64Pack
        : public GenericPack<uint64_t, cl_linear_t, cl_linear_init, cl_linear> {
    static constexpr auto NAME = "ClLinear64";
};

struct ClLinear32Pack
        : public GenericPack<uint32_t, cl_linear_t, cl_linear_init, cl_linear32> {
    static constexpr auto NAME = "ClLinear32";
};

struct MultiplyShift64Pack
        : public GenericPack<uint64_t, MultiplyShift64Randomness,
          MultiplyShift64Init, MultiplyShift64> {
    static constexpr auto NAME = "MS64";
};

struct UnivMultiplyShift64Pack
    : public GenericPack<uint64_t, UnivMultiplyShift64Randomness,
                         UnivMultiplyShift64Init, UnivMultiplyShift64,
                         HashBits::HIGH> {
    static constexpr auto NAME = "UMS64";
};

struct MultiplyTwice64Pack
        : public GenericPack<uint64_t, MultiplyTwice64Randomness,
          MultiplyTwice64Init, MultiplyTwice64> {
    static constexpr auto NAME = "M2-64";
};

struct MultiplyThrice64Pack
        : public GenericPack<uint64_t, MultiplyThrice64Randomness,
          MultiplyThrice64Init, MultiplyThrice64> {
    static constexpr auto NAME = "M3-64";
};

struct ClQuadratic64Pack : public GenericPack<uint64_t, cl_quadratic_t,
        cl_quadratic_init, cl_quadratic> {
    static constexpr auto NAME = "ClQuad64";
};

struct ClFastQuadratic64Pack : public GenericPack<uint64_t, cl_fastquadratic_t,
        cl_fastquadratic_init, cl_fastquadratic> {
    static constexpr auto NAME = "ClFQuad64";
};

struct ClFastQuadratic32Pack : public GenericPack<uint32_t, cl_fastquadratic32_t,
        cl_fastquadratic32_init, cl_fastquadratic32> {
    static constexpr auto NAME = "ClFQuad32";
};


struct ClCubic64Pack
        : public GenericPack<uint64_t, cl_cubic_t, cl_cubic_init, cl_cubic> {
    static constexpr auto NAME = "ClCubic64";
};

struct ThorupZhangCWLinear64Pack
        : public GenericPack<uint64_t, ThorupZhangCWLinear64_t, ThorupZhangCWLinear64Init, ThorupZhangCWLinear64> {
    static constexpr auto NAME = "TCWLinear64";
};

struct ThorupZhangCWQuadratic64Pack
        : public GenericPack<uint64_t, ThorupZhangCWQuadratic64_t, ThorupZhangCWQuadratic64Init, ThorupZhangCWQuadratic64> {
    static constexpr auto NAME = "TCWQuad64";
};

struct ThorupZhangCWCubic64Pack
        : public GenericPack<uint64_t, ThorupZhangCWCubic64_t, ThorupZhangCWCubic64Init, ThorupZhangCWCubic64> {
    static constexpr auto NAME = "TCWCubic64";
};




struct ClQuartic64Pack
        : public GenericPack<uint64_t, cl_quartic_t, cl_quartic_init, cl_quartic> {
    static constexpr auto NAME = "ClQuartic64";
};

struct Zobrist32Pack
        : public GenericPack<uint32_t, zobrist32_t, zobrist32_init, zobrist32> {
    static constexpr auto NAME = "Zobrist32";
};

struct WZobrist32Pack
        : public GenericPack<uint32_t, wzobrist32_t, wzobrist32_init, wzobrist32> {
    static constexpr auto NAME = "WZob32";
};

struct ThorupZhang32Pack
        : public GenericPack<uint32_t, thorupzhang32_t, thorupzhang32_init, thorupzhang32> {
    static constexpr auto NAME = "TZ32";
};



struct MultiplyShift32Pack
        : public GenericPack<uint32_t, MultiplyShift32Randomness,
          MultiplyShift32Init, MultiplyShift32> {
    static constexpr auto NAME = "MS32";
};


struct CWQuad32Pack
        : public GenericPack<uint32_t, CWRandomQuad32, CWRandomQuad32Init,
          CWQuad32> {
    static constexpr auto NAME = "CWQuad32";
};

struct ThorupZhangCWLinear32Pack
        : public GenericPack<uint32_t, ThorupZhangCWLinear32_t, ThorupZhangCWLinear32Init, ThorupZhangCWLinear32> {
    static constexpr auto NAME = "TCWLinear32";
};

struct ThorupZhangCWQuadratic32Pack
        : public GenericPack<uint32_t, ThorupZhangCWQuadratic32_t, ThorupZhangCWQuadratic32Init, ThorupZhangCWQuadratic32> {
    static constexpr auto NAME = "TCWQuad32";
};

struct ThorupZhangCWCubic32Pack
        : public GenericPack<uint32_t, ThorupZhangCWCubic32_t, ThorupZhangCWCubic32Init, ThorupZhangCWCubic32> {
    static constexpr auto NAME = "TCWCub32";
};

struct MultiplyOnly8Pack
        : public GenericPack<uint8_t, MultiplyOnly8Randomness,
          MultiplyOnly8Init, MultiplyOnly8> {
    static constexpr auto NAME = "Mo8";
    explicit MultiplyOnly8Pack(const uint16_t v) { myr->mult = v; }
};

struct Linear64Pack
        : public GenericPack<uint64_t, Linear64Randomness,
          Linear64Init, Linear64> {
    static constexpr auto NAME = "Linear64";
};

struct Toeplitz64Pack
        : public GenericPack<uint64_t, Toeplitz64Randomness,
          Toeplitz64Init, Toeplitz64> {
    static constexpr auto NAME = "Toeplitz64";
};

template <typename Splitter, typename Finalizer, size_t WIDTH>
struct SplitPack {
  static constexpr auto NAME = "Split";
    typedef typename Splitter::Word Word;
    struct Randomness {
        typename Splitter::Randomness splitter;
        typename Finalizer::Randomness finalizers[WIDTH];
    };

    static inline void InitRandomness(Randomness *r) {
        Splitter::InitRandomness(&r->splitter);
        for (size_t i = 0; i < WIDTH; ++i) {
            Finalizer::InitRandomness(&r->finalizers[i]);
        }
    }

    __attribute__((always_inline)) static inline Word
        HashFunction(Word x, const Randomness *r) {
        const auto i = Splitter::HashFunction(x, &r->splitter) % WIDTH;
        return Finalizer::HashFunction(x, &r->finalizers[i]);
    }

    /**
    * We want SplitPack to be usable as a C++ hasher
    */
    SplitPack() : myr(NULL) {
        myr = std::shared_ptr<Randomness>(new Randomness());
        if (myr != NULL) {
            InitRandomness(myr.get());
        }
    }

    SplitPack(const SplitPack &o) : myr(o.myr) {}

    __attribute__((always_inline)) inline size_t operator()(Word x) const {
        return HashFunction(x, myr.get());
    }

  protected:
    std::shared_ptr<Randomness> myr;
};

template <typename... Pack>
struct ForEachT {
    template <typename Worker, typename... Args>
    inline static void Go(Args &&... ) {
        Worker::Stop();
    }
};

template <typename Pack, typename... Rest>
struct ForEachT<Pack, Rest...> {
    template <typename Worker, typename... Args>
    inline static void Go(Args &&... args) {
        Worker::template Go<Pack>(args...);
        ForEachT<Rest...>::template Go<Worker>(args...);
    }
};
