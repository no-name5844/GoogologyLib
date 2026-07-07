#include "googology/notations/number/knuth/Knuth.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

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

// Deep clone (unique_ptr makes KNode non-copyable).
KNode Knuth::cloneNode(const KNode& n) {
    KNode c;
    c.isVal = n.isVal;
    c.val = n.val;
    c.baseVal = n.baseVal;
    c.height = n.height;
    if (n.exp) c.exp = std::make_unique<KNode>(cloneNode(*n.exp));
    return c;
}

// LaTeX (no numeric evaluation) of a Knuth AST node.
std::string Knuth::tex(const KNode& n) {
    if (n.isVal) return std::to_string(n.val);
    std::string bt = std::to_string(n.baseVal);
    std::string et = n.exp ? tex(*n.exp) : "";
    std::string arr = (n.height == 1)
        ? "\\uparrow"
        : "\\uparrow^{" + std::to_string(n.height) + "}";
    // parenthesize the exponent only when it is itself an arrow term
    bool expHasArrow = n.exp && !n.exp->isVal;
    std::string expPart = expHasArrow ? "(" + et + ")" : et;
    return bt + " " + arr + " " + expPart;
}

// One single rewrite step on the AST. No numeric evaluation: the result is
// always another Knuth AST node (a rewritten form of the same notation).
//   - a ^c 1            = a                (collapse to base)
//   - a ^1 b (= a^b)    : terminal, unchanged
//   - a ^c b (c>1,b>1)  = a ^(c-1) (a ^c (b-1))
// When the exponent is itself an arrow, the step recurses into that exponent.
KNode Knuth::step(const KNode& node) {
    if (node.isVal) return cloneNode(node);            // constant: terminal
    if (node.exp && node.exp->isVal) {
        BigInt B = node.exp->val;
        if (B == 1) return KNode::value(node.baseVal);  // a ^c 1 = a
        if (node.height == 1) return cloneNode(node);   // a^b: terminal
        KNode inner = KNode::arrow(node.baseVal, node.height, KNode::value(B - 1));
        KNode outer = KNode::arrow(node.baseVal, node.height - 1, std::move(inner));
        return outer;
    }
    // recurse into the rightmost sub-term
    KNode r;
    r.isVal = false;
    r.baseVal = node.baseVal;
    r.height = node.height;
    r.exp = std::make_unique<KNode>(step(*node.exp));
    return r;
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

    // store as the AST node  a ↑^c b
    root_ = KNode::arrow(a, c, KNode::value(b));
}

std::string Knuth::to_string() const { return tex(root_); }

// Rewrite n single steps, mutating this; return *this (a Knuth object).
Knuth& Knuth::expand(BigInt n) {
    for (BigInt i = 0; i < n; ++i) root_ = step(root_);
    return *this;
}

// Expand until the LaTeX length reaches `len` or the form is stable.
Knuth& Knuth::expand_to(BigInt len) {
    std::string prev = to_string();
    while (true) {
        expand(1);
        std::string cur = to_string();
        if (cur.size() >= static_cast<size_t>(len) || cur == prev) return *this;
        prev = cur;
    }
}

std::istream& operator>>(std::istream& is, Knuth& k) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) k.string_to_it(s);
    return is;
}

} // namespace number
} // namespace googology
