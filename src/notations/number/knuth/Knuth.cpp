#include "googology/notations/number/knuth/Knuth.hpp"
#include <algorithm>
#include <cctype>

namespace googology {
namespace number {

Capabilities Knuth::capabilities() const {
    Capabilities c;
    c.set(Op::Parse);
    c.set(Op::Serialize);
    c.set(Op::Expand);
    c.set(Op::Evaluate);
    return c;
}

BigInt Knuth::evalExpr(const KExpr& e) {
    if (e.kind == KExpr::Kind::Value) return e.val;
    BigInt r = evalExpr(*e.rhs);
    if (e.c <= 1) return ipow(e.a, r);
    if (r == 1) return e.a;
    // a ^c b  =  a ^(c-1) (a ^c (b-1))
    return evalExpr(KExpr::arrow(e.a, e.c - 1, KExpr::arrow(e.a, e.c, KExpr::value(r - 1))));
}

KExpr Knuth::expandExpr(const KExpr& e) {
    if (e.kind == KExpr::Kind::Value) return e;
    BigInt r = evalExpr(*e.rhs);
    if (e.c <= 1) return e;  // a^b is the base case, no further rule
    if (r == 1) return KExpr::value(e.a);
    return KExpr::arrow(e.a, e.c - 1, KExpr::arrow(e.a, e.c, KExpr::value(r - 1)));
}

std::string Knuth::ser(const KExpr& e) {
    if (e.kind == KExpr::Kind::Value) return std::to_string(e.val);
    std::string rhs = ser(*e.rhs);
    if (e.c == 1) return std::to_string(e.a) + "^" + rhs;
    return std::to_string(e.a) + "^" + std::to_string(e.c) + "(" + rhs + ")";
}

void Knuth::parse(const std::string& s) {
    std::string t = s;
    // accept the math symbol ↑ as well (UTF-8 string replace, no narrow-char literal)
    const std::string UP = "↑";
    for (size_t p = t.find(UP); p != std::string::npos; p = t.find(UP, p + 1))
        t.replace(p, UP.size(), "^");
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());

    size_t pos = t.find('^');
    if (pos == std::string::npos) throw std::invalid_argument("Knuth::parse: missing '^'");

    BigInt a = std::stoll(t.substr(0, pos));
    size_t i = pos;
    int c = 0;
    while (i < t.size() && t[i] == '^') { ++c; ++i; }
    // height = number of consecutive '^' characters; a single '^' is ordinary
    // exponentiation (c = 1) and the following number is the exponent.
    if (c == 0) c = 1;
    BigInt b = std::stoll(t.substr(i));

    root_ = std::make_shared<KExpr>(KExpr::arrow(a, c, KExpr::value(b)));
}

std::string Knuth::serialize() const { return ser(*root_); }
BigInt Knuth::evaluate() const { return evalExpr(*root_); }

void Knuth::expand(BigInt n) {
    for (BigInt i = 0; i < n; ++i)
        root_ = std::make_shared<KExpr>(expandExpr(*root_));
}

} // namespace number
} // namespace googology
