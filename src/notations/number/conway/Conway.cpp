#include "googology/notations/number/conway/Conway.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

namespace googology {
namespace number {

Capabilities Conway::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Expand);
    c.set(Op::ExpandTo);
    return c;
}

// LaTeX serialization of a (possibly nested) chain. A length-2 chain is the
// base case a -> b = a^b, rendered as a^{b}.
std::string Conway::ser(const std::vector<CNode>& ch) {
    if (ch.size() == 2 && ch[0].isInt && ch[1].isInt)
        return std::to_string(ch[0].val) + "^{" + std::to_string(ch[1].val) + "}";
    std::ostringstream os;
    for (size_t i = 0; i < ch.size(); ++i) {
        if (i) os << " \\rightarrow ";
        if (ch[i].isInt) os << ch[i].val;
        else os << "(" << ser(ch[i].sub) << ")";
    }
    return os.str();
}

// One single rewrite step (rules 2/3) applied at the outermost applicable
// position. If the top level has no rule but a nested sub-chain does, the
// step is applied one level inside that sub-chain. Linear in chain size;
// never expands exponentially and never computes a numeric value.
std::vector<CNode> Conway::stepOnce(const std::vector<CNode>& ch) {
    size_t n = ch.size();
    if (n <= 2) return ch;  // terminal: [a] or [a -> b]
    // rule 2a: trailing 1  (X -> a -> 1  =  X -> a)
    if (ch.back().isInt && ch.back().val == 1) {
        auto r = ch;
        r.pop_back();
        return r;
    }
    // rule 2b: middle 1  (X -> 1 -> Y  =  X)
    if (ch[n - 2].isInt && ch[n - 2].val == 1) {
        auto r = ch;
        r.resize(n - 2);
        return r;
    }
    // rule 3: X -> a -> b  =  X -> (X -> a-1 -> b) -> b-1   (top level)
    if (ch[n - 1].isInt && ch[n - 2].isInt) {
        BigInt a = ch[n - 2].val, b = ch[n - 1].val;
        std::vector<CNode> X(ch.begin(), ch.begin() + (n - 2));
        std::vector<CNode> inner = X;
        inner.push_back(CNode::value(a - 1));
        inner.push_back(CNode::value(b));
        std::vector<CNode> next = X;
        next.push_back(CNode::subchain(inner));
        next.push_back(CNode::value(b - 1));
        return next;
    }
    // otherwise recurse one step into each nested sub-chain
    std::vector<CNode> r = ch;
    for (auto& node : r)
        if (!node.isInt) node.sub = stepOnce(node.sub);
    return r;
}

void Conway::string_to_it(const std::string& s) {
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
            chain_.push_back(CNode::value(std::stoll(t.substr(start))));
            break;
        }
        chain_.push_back(CNode::value(std::stoll(t.substr(start, pos - start))));
        start = pos + 2;
    }
}

std::string Conway::to_string() const { return ser(chain_); }
std::string Conway::expand(BigInt n) const {
    std::vector<CNode> ch = chain_;
    for (BigInt i = 0; i < n; ++i) ch = stepOnce(ch);
    return ser(ch);
}
std::string Conway::expand_to(BigInt len) const { return expand(len); }

std::istream& operator>>(std::istream& is, Conway& c) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) c.string_to_it(s);
    return is;
}

} // namespace number
} // namespace googology
