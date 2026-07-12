#include "googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace googology {
namespace ordinal {

Capabilities WeakVeblen::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Expand);
    // The article defines NO compare / normalize / expand_to for this
    // notation, so they are intentionally omitted (base class throws).
    return c;
}

// Split `body` (no outer parens) on TOP-LEVEL commas into "a@b" pieces,
// then split each piece on its TOP-LEVEL '@' into the a / b ordinals.
void WeakVeblen::parseBody_(const std::string& body) {
    std::vector<std::pair<Ordinal, Ordinal>> out;
    size_t i = 0, n = body.size();
    while (i < n) {
        while (i < n && body[i] == ' ') ++i;     // skip spaces
        if (i >= n) break;
        int depth = 0;
        std::string comp;
        while (i < n) {
            char c = body[i];
            if (c == '(') ++depth;
            else if (c == ')') { if (depth > 0) --depth; }
            else if (c == ',' && depth == 0) break;
            comp += c;
            ++i;
        }
        if (i < n && body[i] == ',') ++i;       // consume comma
        // split comp on top-level '@'
        int d2 = 0;
        size_t at = std::string::npos;
        for (size_t j = 0; j < comp.size(); ++j) {
            char c = comp[j];
            if (c == '(') ++d2;
            else if (c == ')') { if (d2 > 0) --d2; }
            else if (c == '@' && d2 == 0) { at = j; break; }
        }
        std::string as, bs;
        if (at == std::string::npos) { as = comp; bs = "0"; }   // missing @b -> b=0
        else { as = comp.substr(0, at); bs = comp.substr(at + 1); }
        Ordinal a = Ordinal::parse(as);
        Ordinal b = Ordinal::parse(bs);
        out.push_back({a, b});
    }
    ord_ = Ordinal::wv(out);
}

void WeakVeblen::string_to_it(const std::string& s) {
    std::string t = s;
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());
    while (!t.empty() && (t.front() == '(' || t.front() == '[' || t.front() == '{'))
        t.erase(t.begin());
    while (!t.empty() && (t.back() == ')' || t.back() == ']' || t.back() == '}'))
        t.pop_back();
    if (t.empty()) {
        // the zero notation: a single (0@0) -> article's case 1 A=(0)
        ord_ = Ordinal::wv({{Ordinal::zero(), Ordinal::zero()}});
        return;
    }
    parseBody_(t);
}

std::string WeakVeblen::to_string() const {
    return ord_.to_string();
}

WeakVeblen& WeakVeblen::expand(BigInt n) {
    ord_ = ord_.expand(static_cast<long long>(n));
    return *this;
}

WeakVeblen WeakVeblen::operator[](BigInt n) const {
    WeakVeblen tmp = *this;   // A[n] must NOT mutate *this
    tmp.expand(n);
    return tmp;
}

std::istream& operator>>(std::istream& is, WeakVeblen& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
