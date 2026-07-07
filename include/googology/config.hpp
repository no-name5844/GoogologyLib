#pragma once
#include <cstdint>
#include <stdexcept>

namespace googology {

// Integer type used throughout the library.
//
// NOTE: the library never computes a numeric value for a notation (see
// doc/design.md, "no numeric evaluation"). BigInt is used ONLY for parse
// parameters and expansion indices (e.g. the arrow height c, chain length).
// The default int64_t keeps the build dependency-free; it is sufficient
// because we never materialize the (astronomically large) values themselves.
#ifdef GOOGOLOGY_USE_BOOST
#include <boost/multiprecision/cpp_int.hpp>
using BigInt = boost::multiprecision::cpp_int;
#else
using BigInt = int64_t;
#endif

// Integer exponentiation helper (kept generic; harmless here because the
// library does not evaluate large-number notations to a value).
inline BigInt ipow(BigInt base, BigInt exp) {
    if (exp < 0) throw std::invalid_argument("ipow: negative exponent");
    BigInt result = 1;
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

} // namespace googology
