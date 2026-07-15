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

// Recursive-descent parser for right-associative arrow towers.
//   expr := INT ( '^'+ expr )?
// `3^^3^^3` -> arrow(3, 2, arrow(3, 2, value(3)))  ==  3 ↑↑ (3 ↑↑ 3)
// A bare integer (no '^') is a leaf. Any malformed tail throws.
KNode Knuth::parseExpr_(const std::string& s) {
    size_t pos = s.find('^');
    if (pos == std::string::npos) {
        // bare integer / leaf
        if (s.empty()) throw std::invalid_argument("Knuth::string_to_it: empty operand");
        BigInt v = std::stoll(s);
        return KNode::value(v);
    }
    BigInt a = std::stoll(s.substr(0, pos));
    size_t j = pos;
    int c = 0;
    while (j < s.size() && s[j] == '^') { ++c; ++j; }
    if (c == 0) c = 1;                 // defencive: never 0 after finding '^'
    std::string rest = s.substr(j);
    if (rest.empty())
        throw std::invalid_argument("Knuth::string_to_it: dangling arrows (no exponent)");
    KNode exp = parseExpr_(rest);        // exponent is the whole RHS, parsed recursively
    return KNode::arrow(a, c, std::move(exp));
}

void Knuth::string_to_it(const std::string& s) {
    std::string t = s;
    // accept the math symbol ↑ as well (UTF-8 string replace, no narrow-char literal)
    const std::string UP = "↑";
    for (size_t p = t.find(UP); p != std::string::npos; p = t.find(UP, p + 1))
        t.replace(p, UP.size(), "^");
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());
    if (t.empty()) throw std::invalid_argument("Knuth::string_to_it: empty input");

    // Recursive-descent: supports right-associative arrow towers
    // (e.g. `3^^3^^3` == `3 ↑↑ (3 ↑↑ 3)`). The old single-shot scan
    // silently dropped everything after the first exponent.
    root_ = parseExpr_(t);
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
