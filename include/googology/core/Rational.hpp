#pragma once
// Exact rational arithmetic + accumulated weight-fraction form for Ns / n,m-Ns.
//
// The library never collapses a notation to a number; a Rational is only
// produced ON DEMAND for FINITE ordinal indices (where the value is an
// exact rational). Limit indices require a fundamental sequence and are NOT
// auto-computed (see spec/notations/ns.*.md and design.md §11).
//
// The user's chosen representation is the ACCUMULATED WEIGHT FRACTION:
//   int + w1/d1 + w2/d2 + ...
// e.g. "2 + 5/11 + 62/111" — an integer part plus a sum of (weight,
// denominator) terms whose numerators (weights) need NOT be 1.
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include "googology/config.hpp"

namespace googology {

struct Rational {
    BigInt num = 0;   // numerator
    BigInt den = 1;   // denominator (>0), always reduced

    Rational(BigInt n = 0, BigInt d = 1) {
        if (d < 0) { n = -n; d = -d; }
        if (d == 0) d = 1;
        num = n; den = d;
        reduce_();
    }

    Rational& operator+=(const Rational& o) {
        // a/b + c/d = (a*d + c*b) / (b*d)
        BigInt a = num, b = den, c = o.num, d = o.den;
        num = a * d + c * b;
        den = b * d;
        reduce_();
        return *this;
    }
    Rational operator+(const Rational& o) const { return Rational(*this) += o; }
    bool operator==(const Rational& o) const { return num == o.num && den == o.den; }
    bool operator!=(const Rational& o) const { return !(*this == o); }

    std::string to_string() const {
        if (den == 1) return std::to_string(num);
        return std::to_string(num) + "/" + std::to_string(den);
    }

private:
    void reduce_() {
        if (den == 0) den = 1;
        if (den < 0) { num = -num; den = -den; }
        BigInt nn = num < 0 ? -num : num;
        BigInt g = std::gcd(nn, den);
        if (g > 1) { num /= g; den /= g; }
    }
};

// Accumulated weight-fraction form:  int + w1/d1 + w2/d2 + ...
// (the user's "累加权重分数"; e.g. "2 + 5/11 + 62/111").
// Each term is a (weight, denominator) pair with denominator > 0.
struct WeightedFractionSum {
    BigInt intPart = 0;                                 // integer part
    std::vector<std::pair<BigInt, BigInt>> terms;        // (weight, denominator)

    // Total reduced value: intPart + Σ weight/denominator.
    Rational value() const {
        Rational r(intPart, 1);
        for (const auto& t : terms) r += Rational(t.first, t.second);
        return r;
    }

    std::string to_string() const {
        std::ostringstream os;
        bool first = true;
        // Omit a "0 +" prefix when there are term(s) to show.
        if (!(intPart == 0 && !terms.empty())) {
            os << intPart;
            first = false;
        }
        for (const auto& t : terms) {
            if (!first) os << " + ";
            os << t.first << "/" << t.second;
            first = false;
        }
        if (first) os << "0";   // both intPart==0 and no terms
        return os.str();
    }
};

} // namespace googology
