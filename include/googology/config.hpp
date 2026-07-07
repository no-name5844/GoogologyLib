#pragma once
#include <cstdint>
#include <stdexcept>

namespace googology {

// Integer type used throughout the library.
//
// For true big-number evaluation (values far beyond 2^63) enable Boost and
// define GOOGOLOGY_USE_BOOST before including this header, or pass
// -DGOOGOLOGY_USE_BOOST to the compiler. The default is int64_t so the
// library compiles with NO external dependencies (sufficient for checking
// small inputs and for auditing the recursion logic).
#ifdef GOOGOLOGY_USE_BOOST
#include <boost/multiprecision/cpp_int.hpp>
using BigInt = boost::multiprecision::cpp_int;
#else
using BigInt = int64_t;
#endif

// Integer exponentiation, works for both int64_t and boost::multiprecision::cpp_int.
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
