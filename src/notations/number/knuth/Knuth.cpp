#include "googology/notations/number/knuth/Knuth.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>

namespace googology {
namespace number {

Capabilities Knuth::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Expand);
    c.set(Op::ExpandTo);
    return c;
}

// LaTeX of a ^c b, after `depth` single-step rewrites. No numeric evaluation:
// the result is always the notation's own symbolic form.
std::string Knuth::fmt(BigInt a, BigInt c, BigInt b, BigInt depth) {
    auto arrow = [](BigInt cc) -> std::string {
        if (cc == 1) return "\\uparrow";
        return "\\uparrow^{" + std::to_string(cc) + "}";
    };
    auto lit = [&](BigInt aa, BigInt cc, BigInt bb) -> std::string {
        return std::to_string(aa) + " " + arrow(cc) + " " + std::to_string(bb);
    };
    if (depth <= 0) return lit(a, c, b);
    if (b == 1) return std::to_string(a);          // a ^c 1 = a
    if (c == 1) return lit(a, 1, b);               // a^b, no further expansion
    std::string inner = fmt(a, c, b - 1, depth - 1);
    std::string outer = std::to_string(a) + " " + arrow(c - 1) + " ";
    if (inner.find('\\') != std::string::npos) return outer + "(" + inner + ")";
    return outer + inner;
}

void Knuth::string_to_it(const std::string& s) {
    std::string t = s;
    // accept the math symbol ↑ as well (UTF-8 string replace, no narrow-char literal)
    const std::string UP = "↑";
    for (size_t p = t.find(UP); p != std::string::npos; p = t.find(UP, p + 1))
        t.replace(p, UP.size(), "^");
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());

    size_t pos = t.find('^');
    if (pos == std::string::npos) throw std::invalid_argument("Knuth::string_to_it: missing '^'");

    BigInt a = std::stoll(t.substr(0, pos));
    size_t i = pos;
    int c = 0;
    while (i < t.size() && t[i] == '^') { ++c; ++i; }
    // height = number of consecutive '^' characters; a single '^' is ordinary
    // exponentiation (c = 1) and the following number is the exponent.
    if (c == 0) c = 1;
    BigInt b = std::stoll(t.substr(i));

    a_ = a;
    c_ = c;
    b_ = b;
}

std::string Knuth::to_string() const { return fmt(a_, c_, b_, 0); }
std::string Knuth::expand(BigInt n) const { return fmt(a_, c_, b_, n); }
std::string Knuth::expand_to(BigInt len) const { return expand(len); }

std::istream& operator>>(std::istream& is, Knuth& k) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) k.string_to_it(s);
    return is;
}

} // namespace number
} // namespace googology
