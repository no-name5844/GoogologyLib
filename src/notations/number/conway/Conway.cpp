#include "googology/notations/number/conway/Conway.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace googology {
namespace number {

Capabilities Conway::capabilities() const {
    Capabilities c;
    c.set(Op::Parse);
    c.set(Op::Serialize);
    c.set(Op::Expand);
    c.set(Op::Evaluate);
    return c;
}

BigInt Conway::evalChain(const std::vector<BigInt>& ch) {
    size_t n = ch.size();
    if (n == 1) return ch[0];
    if (n == 2) return ipow(ch[0], ch[1]);
    if (ch[n - 1] == 1) {  // trailing 1 -> drop (X -> a -> 1  =  X -> a)
        auto c = ch;
        c.pop_back();
        return evalChain(c);
    }
    if (ch[n - 2] == 1) {  // rule 2: X -> 1 -> Y = X
        auto c = ch;
        c.resize(n - 2);
        return evalChain(c);
    }
    // rule 3: X -> a -> b = X -> (X -> a-1 -> b) -> b-1
    BigInt a = ch[n - 2], b = ch[n - 1];
    std::vector<BigInt> X(ch.begin(), ch.begin() + (n - 2));
    std::vector<BigInt> inner = X;
    inner.push_back(a - 1);
    inner.push_back(b);
    BigInt innerVal = evalChain(inner);
    std::vector<BigInt> next = X;
    next.push_back(innerVal);
    next.push_back(b - 1);
    return evalChain(next);
}

// One expansion step. The inner sub-chain (X -> a-1 -> b) is collapsed to its
// value, keeping the result chain flat. (A fully symbolic nested form is a
// future extension; see doc/design.md.)
std::vector<BigInt> Conway::stepChain(const std::vector<BigInt>& ch) {
    size_t n = ch.size();
    if (n == 1 || n == 2) return ch;
    if (ch[n - 1] == 1) {
        auto c = ch;
        c.pop_back();
        return c;
    }
    if (ch[n - 2] == 1) {
        auto c = ch;
        c.resize(n - 2);
        return c;
    }
    BigInt a = ch[n - 2], b = ch[n - 1];
    std::vector<BigInt> X(ch.begin(), ch.begin() + (n - 2));
    std::vector<BigInt> inner = X;
    inner.push_back(a - 1);
    inner.push_back(b);
    BigInt innerVal = evalChain(inner);
    std::vector<BigInt> next = X;
    next.push_back(innerVal);
    next.push_back(b - 1);
    return next;
}

std::string Conway::ser(const std::vector<BigInt>& ch) {
    std::ostringstream os;
    for (size_t i = 0; i < ch.size(); ++i) {
        if (i) os << "->";
        os << ch[i];
    }
    return os.str();
}

void Conway::parse(const std::string& s) {
    std::string t = s;
    // normalize the unicode arrow "→" to the ASCII "->"
    const std::string ARROW = "→";
    for (size_t p = t.find(ARROW); p != std::string::npos; p = t.find(ARROW, p + 2))
        t.replace(p, ARROW.size(), "->");
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());

    chain_.clear();
    size_t start = 0;
    while (true) {
        size_t pos = t.find("->", start);
        if (pos == std::string::npos) {
            chain_.push_back(std::stoll(t.substr(start)));
            break;
        }
        chain_.push_back(std::stoll(t.substr(start, pos - start)));
        start = pos + 2;
    }
}

std::string Conway::serialize() const { return ser(chain_); }
BigInt Conway::evaluate() const { return evalChain(chain_); }

void Conway::expand(BigInt n) {
    for (BigInt i = 0; i < n; ++i) chain_ = stepChain(chain_);
}

} // namespace number
} // namespace googology
