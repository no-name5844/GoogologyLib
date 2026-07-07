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
// step is applied inside that sub-chain. Linear in chain size; never expands
// exponentially and never computes a numeric value.
std::vector<CNode> Conway::stepOnce(const std::vector<CNode>& ch) {
    size_t n = ch.size();
    if (n <= 2) {
        if (n == 0 || n == 1) return ch;
        // genuine base case a -> b (both integers): terminal
        if (ch[0].isInt && ch[1].isInt) return ch;
        // a length-2 chain whose operand is a sub-chain: step that sub-chain
        return recurseSub(ch);
    }
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
    // no top-level rule: step any nested sub-chain
    return recurseSub(ch);
}

// Apply stepOnce to each immediate sub-chain; return ch unchanged if none can
// step (this is what makes reduce() drive nested chains to a stable form).
std::vector<CNode> Conway::recurseSub(const std::vector<CNode>& ch) {
    std::vector<CNode> r = ch;
    bool changed = false;
    for (auto& node : r) {
        if (!node.isInt) {
            std::string before = ser(node.sub);
            std::vector<CNode> s = stepOnce(node.sub);
            if (ser(s) != before) {
                // a sub-chain that shrinks to a single number collapses to a plain int
                if (s.size() == 1 && s[0].isInt) node = CNode::value(s[0].val);
                else node.sub = std::move(s);
                changed = true;
            }
        }
    }
    return changed ? r : ch;
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

// Rewrite n single steps, mutating this; return *this (a Conway object).
Conway& Conway::expand(BigInt n) {
    for (BigInt i = 0; i < n; ++i) chain_ = stepOnce(chain_);
    return *this;
}

// Expand until the LaTeX length reaches `len` or the form is stable.
Conway& Conway::expand_to(BigInt len) {
    std::string prev = to_string();
    while (true) {
        expand(1);
        std::string cur = to_string();
        if (cur.size() >= static_cast<size_t>(len) || cur == prev) return *this;
        prev = cur;
    }
}

std::istream& operator>>(std::istream& is, Conway& c) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) c.string_to_it(s);
    return is;
}

} // namespace number
} // namespace googology
