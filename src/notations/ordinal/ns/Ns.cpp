#include "googology/notations/ordinal/ns/Ns.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace googology;
using namespace googology::ordinal;

namespace {
// Strip leading / trailing whitespace.
std::string trim_(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}
} // namespace

Capabilities Ns::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Expand);
    c.set(Op::Compare);
    return c;
}

bool Ns::isFiniteInt_(const Ordinal& a, BigInt& outK) {
    std::string t = a.to_string();
    if (t.empty()) return false;
    for (char ch : t)
        if (!std::isdigit(static_cast<unsigned char>(ch))) return false;
    try { outK = std::stoll(t); } catch (...) { return false; }
    return true;
}

Rational Ns::valueFinite_(BigInt k) const {
    // a_1 = 0; a_{β+1} = a_β + 1/n^β  =>  a_k = sum_{β=1}^{k-1} 1/n^β
    Rational r(0);
    for (BigInt beta = 1; beta < k; ++beta) {
        Rational step(1, ipow(n_, beta));   // 1 / n^beta
        r += step;
    }
    return r;
}

void Ns::string_to_it(const std::string& s) {
    std::string t = trim_(s);
    // forms: "α th NS", "α th 𝕟𝕊", or bare "α". Take the part before "th".
    std::string alphaStr = t;
    std::string low = t;
    std::transform(low.begin(), low.end(), low.begin(),
                   [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
    auto pos = low.find("th");
    if (pos != std::string::npos) {
        alphaStr = trim_(t.substr(0, pos));
    }
    if (alphaStr.empty()) alphaStr = "1";
    alpha_ = Ordinal::parse(alphaStr);
}

std::string Ns::to_string() const {
    return alpha_.to_string() + " th \\mathbb{NS}";
}

Ns& Ns::expand(BigInt k) {
    // §11: fundamental sequence is the pluggable core::Ordinal expansion.
    // For a limit index α, α.expand(k) gives the k-th term of the FS,
    // i.e. the index of the k-th element approaching x_α. A finite /
    // successor index has no fundamental sequence; core::Ordinal throws
    // std::domain_error, which we surface as UnsupportedOperation
    // (the "can't auto-compute" case the user noted).
    try {
        alpha_ = alpha_.expand(static_cast<long long>(k));
    } catch (const std::domain_error&) {
        throw UnsupportedOperation(name(), Op::Expand);
    }
    return *this;
}

int Ns::compare(const Notation& other) const {
    const Ns* o = dynamic_cast<const Ns*>(&other);
    if (!o) throw NotComparable(name());
    // a_α strictly increasing in α  =>  order of values == order of indices.
    return alpha_.compare(o->alpha_);
}

Rational Ns::value() const {
    BigInt k;
    if (!isFiniteInt_(alpha_, k))
        throw std::domain_error(
            "Ns::value: only a finite ordinal index yields an exact rational; "
            "limit indices need a fundamental sequence (not auto-computed)");
    return valueFinite_(k);
}

WeightedFractionSum Ns::accumulated_weight_fractions() const {
    BigInt k;
    if (!isFiniteInt_(alpha_, k))
        throw std::domain_error(
            "Ns::accumulated_weight_fractions: only finite ordinal index");
    WeightedFractionSum w;
    w.intPart = 0;
    for (BigInt beta = 1; beta < k; ++beta)
        w.terms.push_back({1, ipow(n_, beta)});  // weight 1, denom n^beta
    return w;
}

std::string Ns::to_fraction_string() const {
    BigInt k;
    if (!isFiniteInt_(alpha_, k))
        throw std::domain_error(
            "Ns::to_fraction_string: only finite ordinal index");
    if (k <= 1) return "0";
    return accumulated_weight_fractions().to_string();
}
