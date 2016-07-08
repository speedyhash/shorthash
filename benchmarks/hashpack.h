#include <memory>

extern "C" {
#include "clhash.h"
#include "cw-trick.h"
#include "multiply-shift.h"
#include "tabulated.h"
}

template <typename WordP, typename RandomnessP,
          void (*InitRandomnessP)(RandomnessP *),
          WordP (*HashFunctionP)(WordP, const RandomnessP *)>
class GenericPack {
public:
    typedef WordP Word;
    typedef RandomnessP Randomness;
    static inline void InitRandomness(Randomness *r) {
        InitRandomnessP(r);
    }
    static inline Word HashFunction(Word x, const Randomness *r) {
        return HashFunctionP(x, r);
    }

    /**
    * We want GenericPack to be usable as a C++ hasher
    */
    GenericPack() : myr(NULL) {
        myr = std::shared_ptr<Randomness>(new Randomness());
        if(myr != NULL) {
            InitRandomness(myr.get());
        }
    }

    GenericPack(const GenericPack & o) : myr(o.myr) {
    }



    size_t operator()(Word x) const {
        return HashFunction(x,myr.get());
    }
private:


    std::shared_ptr<Randomness> myr;
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

struct ClFast2Quadratic64Pack : public GenericPack<uint64_t, cl_fastquadratic2_t,
        cl_fastquadratic2_init, cl_fastquadratic2> {
    static constexpr auto NAME = "ClF2Q64";
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
