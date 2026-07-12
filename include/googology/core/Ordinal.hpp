#pragma once
// Unified symbolic ordinal expression type.
//
// This is the foundation described in `study/notations/Ordinal.md`: for any
// non-closed ordinal notation / non-sequence-type notation, the article defines
//   * alpha + 0 = alpha ;  expand(alpha+beta, n) = alpha + expand(beta, n)
//   * alpha * 0 = 0   ;  expand(alpha*beta, n) = alpha * expand(beta, n)
//   * alpha^0 = 1      ;  expand(alpha^beta, n) = alpha^expand(beta, n)
//
// `Ordinal` is a VALUE type (not a Notation in the taxonomy). It represents
// an ordinal as a symbolic expression tree. `expand(n)` rewrites that tree and
// returns a new `Ordinal` (the library NEVER collapses to a number).
//
// Variants:
//   Zero, Omega (ω), Successor(a)=a+1, Add(a,b)=a+b, Mul(a,b)=a*b,
//   Pow(a,b)=a^b, WeakVeblen([(a_i,b_i)...]) = the (a@b) list notation,
//   Cnf(terms) = Cantor normal form (exponents descending, coeffs >= 1).
//
// CNF is the canonical form used only for COMPARISON / equality / predecessor /
// left-subtraction of *closed* ordinals (0,1,ω,ω+1,ω·2,ω^2,...). It is
// produced on demand by toCnf(); stored nodes stay as expression trees so that
// expand() can dispatch on the article's + / * / ^ forms.
//
// ---- Conventions where the article is SILENT (flagged, minimal additions) ----
// C2: expand(ω, n) = n. The article defines no fundamental sequence for ω;
//     this is the standard one. 0, 1 and every successor have no FS, so their
//     expand throws std::domain_error.
// C3: case-condition arithmetic (a=c+1, a_n=a_k+q, b_i>b_{i+1}, ...) is
//     evaluated on the Cantor normal form of the COMPONENTS. Components must be
//     *closed* ordinals (0,1,ω,ω+1,...). A component that is itself a
//     WeakVeblen expression cannot be reduced to a CNF value (the article does
//     not define the ordinal value of a weak-Veblen expression) -> toCnf() throws.
//
// No evaluation: the library never computes a numeric value for any notation.
#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace googology {

// A symbolic ordinal expression. See file header for the variant semantics.
class Ordinal {
public:
    enum class Kind { Zero, Omega, Succ, Add, Mul, Pow, WV, Cnf };

private:
    Kind kind_;
    std::shared_ptr<Ordinal> a_, b_;          // operands (Succ/Add/Mul/Pow)
    std::vector<std::pair<Ordinal, Ordinal>> comps_;  // WV: (a_i @ b_i)
    std::vector<std::pair<Ordinal, int>>      cnf_;    // Cnf: (exponent, coeff)

    Ordinal(Kind k) : kind_(k) {}

public:
    // ---- constructors (factories) ----
    Ordinal() : kind_(Kind::Zero) {}   // default -> the zero ordinal
    static Ordinal zero() { Ordinal o(Kind::Zero); return o; }
    static Ordinal one()  { return cnf({{zero(), 1}}); }
    static Ordinal omega(){ Ordinal o(Kind::Omega); return o; }
    static Ordinal fromInt(int n) {
        if (n <= 0) return zero();
        return cnf({{zero(), n}});
    }
    static Ordinal succ(const Ordinal& x) {
        // NOTE: do NOT route through isZero() (which calls toCnf and would
        // throw on a WeakVeblen node). Here we just need "is x the zero
        // ordinal?" so that 0+1 collapses to 1. A WV node is never the zero
        // ordinal, so it is wrapped as a Succ; an empty Cnf likewise stands
        // for zero (e.g. a subtract() result), so it collapses to 1 too.
        if (x.kind() == Kind::Zero) return one();                 // 0+1 = 1
        if (x.kind() == Kind::Cnf && x.cnf_.empty()) return one();
        Ordinal o(Kind::Succ); o.a_ = std::make_shared<Ordinal>(x); return o;
    }
    static Ordinal add(const Ordinal& x, const Ordinal& y) {
        Ordinal o(Kind::Add); o.a_ = std::make_shared<Ordinal>(x);
        o.b_ = std::make_shared<Ordinal>(y); return o;
    }
    static Ordinal mul(const Ordinal& x, const Ordinal& y) {
        Ordinal o(Kind::Mul); o.a_ = std::make_shared<Ordinal>(x);
        o.b_ = std::make_shared<Ordinal>(y); return o;
    }
    static Ordinal pow(const Ordinal& x, const Ordinal& y) {
        Ordinal o(Kind::Pow); o.a_ = std::make_shared<Ordinal>(x);
        o.b_ = std::make_shared<Ordinal>(y); return o;
    }
    static Ordinal wv(const std::vector<std::pair<Ordinal, Ordinal>>& c) {
        Ordinal o(Kind::WV); o.comps_ = c; return o;
    }
    static Ordinal cnf(const std::vector<std::pair<Ordinal, int>>& t) {
        Ordinal o(Kind::Cnf); o.cnf_ = normalizeCnf(t); return o;
    }

    Kind kind() const { return kind_; }

    // WV component list (read accessor). Public so notation wrappers
    // (e.g. WeakVeblen) can read the (a@b) pairs. The earlier
    // `friend class WeakVeblen;` was in the wrong namespace and granted
    // access to a distinct empty class, so we expose this read-only
    // accessor instead.
    const std::vector<std::pair<Ordinal, Ordinal>>& comps() const { return comps_; }

    // ---- arithmetic builders (return expression nodes, never reduce) ----
    Ordinal operator+(const Ordinal& o) const { return add(*this, o); }
    Ordinal operator*(const Ordinal& o) const { return mul(*this, o); }
    Ordinal pow(const Ordinal& o)    const { return pow(*this, o); }
    Ordinal successor()            const { return succ(*this); }

    // ---- comparison / predicates (via Cantor normal form) ----
    int compare(const Ordinal& o) const { return compareCnf(toCnf(), o.toCnf()); }
    bool equals(const Ordinal& o) const { return compare(o) == 0; }
    bool isZero()      const { return toCnf().empty(); }
    bool isSuccessor() const { auto t = toCnf(); return !t.empty() && t.back().first.isZero(); }

    // predecessor of a successor ordinal (throws if not a successor).
    Ordinal predecessor() const {
        auto t = toCnf();
        if (t.empty() || !t.back().first.isZero())
            throw std::domain_error("Ordinal::predecessor: not a successor");
        auto t2 = t;
        if (t2.back().second > 1) --t2.back().second;
        else t2.pop_back();
        if (t2.empty()) return zero();   // predecessor of 1 is the literal 0
        return cnf(t2);
    }

    // left subtraction: returns *this - o, valid only when *this >= o.
    Ordinal subtract(const Ordinal& o) const {
        return cnf(subCnf(toCnf(), o.toCnf()));
    }

    // ---- string (LaTeX-ish) ----
    std::string to_string() const;

    // ---- fundamental sequence: expand(n) rewrites the expression ----
    Ordinal expand(long long n) const;

    // ---- parser: integers, ω/w/omega, +, *, ^, parentheses ----
    static Ordinal parse(const std::string& s);

private:
    using Cnf = std::vector<std::pair<Ordinal, int>>;

    // --- Cantor normal form helpers ---
    static Cnf normalizeCnf(const Cnf& v) {
        if (v.empty()) return {};
        Cnf s = v;
        std::sort(s.begin(), s.end(), [](const std::pair<Ordinal,int>& x,
                                         const std::pair<Ordinal,int>& y) {
            int c = x.first.compare(y.first);
            return c > 0;   // descending by exponent
        });
        Cnf out;
        for (auto& term : s) {
            if (term.second == 0) continue;
            if (!out.empty() && out.back().first.equals(term.first))
                out.back().second += term.second;
            else
                out.push_back(term);
        }
        Cnf out2;
        for (auto& term : out)
            if (term.second != 0) out2.push_back(term);
        return out2;
    }

    static int compareCnf(const Cnf& A, const Cnf& B) {
        size_t i = 0;
        while (i < A.size() && i < B.size()) {
            int ce = A[i].first.compare(B[i].first);
            if (ce != 0) return ce;
            if (A[i].second != B[i].second)
                return A[i].second < B[i].second ? -1 : 1;
            ++i;
        }
        if (A.size() < B.size()) return -1;
        if (A.size() > B.size()) return 1;
        return 0;
    }

    static Cnf addCnf(const Cnf& A, const Cnf& B) {
        if (B.empty()) return A;
        if (A.empty()) return B;
        const auto& e1 = A[0].first; const int c1 = A[0].second;
        const auto& e2 = B[0].first; const int c2 = B[0].second;
        int cmp = e1.compare(e2);
        if (cmp < 0) return B;          // γ1 < δ1 : β dominates (α absorbed)
        if (cmp > 0) {                    // γ1 > δ1 : A.lead + (R + β)
            Cnf r = {A[0]};
            Cnf rest = addCnf(tail(A), B);
            for (auto& t : rest) r.push_back(t);
            return normalizeCnf(r);
        }
        Cnf rest = addCnf(tail(A), tail(B));   // γ1 == δ1 : combine leading + (R + S)
        Cnf r = {{e1, c1 + c2}};
        for (auto& t : rest) r.push_back(t);
        return normalizeCnf(r);
    }

    static Cnf mulCnf(const Cnf& A, const Cnf& B) {
        if (A.empty() || B.empty()) return {};
        const auto& e1 = A[0].first; const int c1 = A[0].second;
        Cnf c1B = B;
        for (int i = 1; i < c1; ++i) c1B = addCnf(c1B, B);
        Cnf lead;
        for (auto& t : c1B)
            lead.push_back({e1 + t.first, t.second});   // ω^e1 * ω^t.exp
        Cnf rest = mulCnf(tail(A), B);
        return normalizeCnf(addCnf(lead, rest));
    }

    static Cnf powCnf(const Cnf& base, const Cnf& exp) {
        if (exp.empty()) return {{zero(), 1}};          // α^0 = 1
        if (base.empty()) return {};                     // 0^β = 0 (β>0)
        // 1^β = 1
        if (base.size() == 1 && base[0].first.isZero() && base[0].second == 1)
            return {{zero(), 1}};
        // finite exponent β = k : α^k by repeated multiplication
        if (exp.size() == 1 && exp[0].first.isZero()) {
            int k = exp[0].second;
            Cnf r = {{zero(), 1}};
            for (int i = 0; i < k; ++i) r = mulCnf(r, base);
            return r;
        }
        // infinite exponent: exp = ω^f * d + S
        const auto& f = exp[0].first; const int d = exp[0].second;
        Cnf S = tail(exp);
        // base^{ω^f} = ω^{e1 * ω^f}  (e1 = base's leading exponent)
        Ordinal e1 = base[0].first;
        Ordinal e1wf = e1 * omega().pow(f);            // e1 * ω^f
        Cnf baseWf = {{e1wf, 1}};
        Cnf baseWfd;
        for (int i = 0; i < d; ++i) baseWfd = addCnf(baseWfd, baseWf);
        Cnf baseS = powCnf(base, S);
        return mulCnf(baseWfd, baseS);
    }

    static Cnf subCnf(const Cnf& A, const Cnf& B) {
        if (B.empty()) return A;
        if (A.empty()) return {};
        const auto& e1 = A[0].first; const int c1 = A[0].second;
        const auto& e2 = B[0].first; const int c2 = B[0].second;
        int cmp = e1.compare(e2);
        if (cmp < 0) return {};        // A < B : invalid (caller ensures A>=B)
        if (cmp > 0) return A;        // B absorbed
        if (c1 > c2) {
            Cnf rest = addCnf(tail(A), tail(B));
            Cnf head = {{e1, c1 - c2}};
            Cnf r = head;
            for (auto& t : rest) r.push_back(t);
            return normalizeCnf(r);
        }
        return subCnf(tail(A), tail(B));   // c1 == c2
    }

    static Cnf tail(const Cnf& v) {
        Cnf r;
        for (size_t i = 1; i < v.size(); ++i) r.push_back(v[i]);
        return r;
    }

    // CNF of this expression (throws for a WeakVeblen component — see C3).
    Cnf toCnf() const {
        switch (kind_) {
            case Kind::Zero:  return {};
            case Kind::Omega: return {{one(), 1}};           // ω = ω^1
            case Kind::Cnf:   return cnf_;
            case Kind::Succ: {
                Cnf x = a_->toCnf();
                if (x.empty()) return {{zero(), 1}};         // 0+1 = 1
                Cnf r = x;
                if (r.back().first.isZero()) ++r.back().second;
                else r.push_back({zero(), 1});
                return normalizeCnf(r);
            }
            case Kind::Add:  return addCnf(a_->toCnf(), b_->toCnf());
            case Kind::Mul:  return mulCnf(a_->toCnf(), b_->toCnf());
            case Kind::Pow:  return powCnf(a_->toCnf(), b_->toCnf());
            case Kind::WV:
                throw std::domain_error(
                    "Ordinal::toCnf: the article does not define the ordinal "
                    "value of a weak-Veblen expression (component must be closed)");
        }
        return {};
    }

    // expandWV (the 6 weak-Veblen cases) is an Ordinal-internal
    // helper, called only by Ordinal::expand(), so it stays private.
    Ordinal expandWV(long long n) const;   // the 6 WV cases

    // Render an (a@b) component: wrap in parens when its string contains a
    // space or parenthesis, so that e.g. (2@3,0@1)+1 does not read as
    // ((2@3,0@1)+1)@0 with @ binding tighter than +.
    static std::string dispPart_(const Ordinal& x) {
        std::string s = x.to_string();
        for (char ch : s)
            if (ch == ' ' || ch == '(' || ch == ')') return "(" + s + ")";
        return s;
    }
};

// ---------------------------------------------------------------------------
// to_string
// ---------------------------------------------------------------------------
inline std::string Ordinal::to_string() const {
    switch (kind_) {
        case Kind::Zero:  return "0";
        case Kind::Omega: return "ω";
        case Kind::Cnf: {
            if (cnf_.empty()) return "0";
            std::ostringstream os;
            for (size_t i = 0; i < cnf_.size(); ++i) {
                if (i) os << " + ";
                const Ordinal& e = cnf_[i].first;
                int c = cnf_[i].second;
                if (c == 1 && !e.isZero()) os << "ω^" << e.to_string();
                else if (e.isZero()) os << c;
                else os << "ω^" << e.to_string() << "·" << c;
            }
            return os.str();
        }
        case Kind::Succ: return a_->to_string() + " + 1";
        case Kind::Add:  return a_->to_string() + " + " + b_->to_string();
        case Kind::Mul:  return a_->to_string() + " * " + b_->to_string();
        case Kind::Pow:  return "(" + a_->to_string() + " ^ " + b_->to_string() + ")"; // wrap: article delimits the exponent
        case Kind::WV: {
            std::ostringstream os;
            os << "(";
            for (size_t i = 0; i < comps_.size(); ++i) {
                if (i) os << ", ";
                os << dispPart_(comps_[i].first) << "@" << dispPart_(comps_[i].second);
            }
            os << ")";
            return os.str();
        }
    }
    return "?";
}

// ---------------------------------------------------------------------------
// expand — fundamental sequence (rewrites, returns a new Ordinal)
// ---------------------------------------------------------------------------
inline Ordinal Ordinal::expand(long long n) const {
    switch (kind_) {
        case Kind::Zero:   // no fundamental sequence
        case Kind::Omega:  // C2: ω's FS is n
            if (kind_ == Kind::Omega) return Ordinal::fromInt(static_cast<int>(n));
            throw std::domain_error("Ordinal::expand: 0 has no fundamental sequence");
        case Kind::Cnf:    // closed ordinal FS undefined by the article
            throw std::domain_error("Ordinal::expand: closed ordinal has no defined FS");
        case Kind::Succ:   // successors have no fundamental sequence
            throw std::domain_error("Ordinal::expand: successor ordinal has no FS");
        case Kind::Add:    // expand(alpha+beta, n) = alpha + expand(beta, n)
            if (b_->isZero()) return *a_;
            return Ordinal::add(*a_, b_->expand(n));
        case Kind::Mul:    // expand(alpha*beta, n) = alpha * expand(beta, n)
            if (b_->isZero()) return Ordinal::zero();
            return Ordinal::mul(*a_, b_->expand(n));
        case Kind::Pow:    // expand(alpha^beta, n) = alpha^expand(beta, n)
            if (b_->isZero()) return Ordinal::one();
            return Ordinal::pow(*a_, b_->expand(n));
        case Kind::WV:     // the 6 cases (study/notations/weak-Veblen-like notation.md)
            return expandWV(n);
    }
    return *this;
}

// The 6 WeakVeblen cases, kept separate for clarity.
inline Ordinal Ordinal::expandWV(long long n) const {
    const auto& c = comps_;
    // Case 1: A = (0)  ->  expand(A, n) = n
    if (c.size() == 1 && c[0].first.isZero())
        return Ordinal::fromInt(static_cast<int>(n));
    // Case 2: A = (0@b1, #)  ->  expand(A, n) = (#)
    if (!c.empty() && c[0].first.isZero()) {
        std::vector<std::pair<Ordinal, Ordinal>> r;
        for (size_t i = 1; i < c.size(); ++i) r.push_back(c[i]);
        return Ordinal::wv(r);
    }
    // Examine the last component (a @ b).
    const Ordinal& aLast = c.back().first;
    const Ordinal& bLast = c.back().second;
    if (aLast.isSuccessor()) {
        Ordinal a = aLast.predecessor();   // aLast = a + 1
        if (bLast.isZero()) {
            // Case 3: A = (#, (a+1)@0)
            //   n=0      -> (#, a@0) + 1
            //   n=m+1    -> (#, a@0) ^ expand(A, m)
            std::vector<std::pair<Ordinal, Ordinal>> base;
            for (size_t i = 0; i + 1 < c.size(); ++i) base.push_back(c[i]);
            base.push_back({a, Ordinal::zero()});
            Ordinal baseWV = Ordinal::wv(base);
            if (n == 0) return baseWV.successor();
            Ordinal A = *this;                 // still (#, (a+1)@0)
            Ordinal rec = A.expand(static_cast<long long>(n - 1));
            return baseWV.pow(rec);
        } else if (bLast.isSuccessor()) {
            // Case 4: A = (#, (a+1)@(b+1))
            //   n=0      -> (#, a@(b+1)) + 1
            //   n=m+1    -> (#, a@(b+1), expand(A, m)@b)
            Ordinal b = bLast.predecessor();    // bLast = b + 1
            std::vector<std::pair<Ordinal, Ordinal>> base;
            for (size_t i = 0; i + 1 < c.size(); ++i) base.push_back(c[i]);
            base.push_back({a, b.successor()}); // (a@(b+1))
            Ordinal baseWV = Ordinal::wv(base);
            if (n == 0) return baseWV.successor();
            Ordinal A = *this;
            Ordinal rec = A.expand(static_cast<long long>(n - 1));
            std::vector<std::pair<Ordinal, Ordinal>> r = base;
            r.push_back({rec, b});              // (expand(A,m) @ b)
            return Ordinal::wv(r);
        } else {
            // Case 5: A = (#, (a+1)@b)  with b a limit (not successor)
            //   expand(A, n) = (#, a@b, 1@expand(b, n))
            std::vector<std::pair<Ordinal, Ordinal>> r;
            for (size_t i = 0; i + 1 < c.size(); ++i) r.push_back(c[i]);
            r.push_back({a, bLast});
            Ordinal eb = bLast.expand(n);
            r.push_back({Ordinal::one(), eb});  // (1 @ expand(b,n))
            return Ordinal::wv(r);
        }
    }
    // Case 6: A = (#, a@b) with a NOT a successor (a is a limit / zero)
    //   expand(A, n) = (#, expand(a, n)@b)
    std::vector<std::pair<Ordinal, Ordinal>> r;
    for (size_t i = 0; i + 1 < c.size(); ++i) r.push_back(c[i]);
    Ordinal ea = aLast.expand(n);
    r.push_back({ea, bLast});
    return Ordinal::wv(r);
}

// ---------------------------------------------------------------------------
// parser: integers, ω/w/omega, +, *, ^, parentheses.
// Right-assoc ^, then * · ×, then +.  (The WeakVeblen (a@b) form is
// parsed separately by WeakVeblen::string_to_it, which uses this for a/b.)
// ---------------------------------------------------------------------------
namespace {
struct OpState { std::string s; size_t i = 0;
    int peek() { while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
                 return i < s.size() ? s[i] : -1; }
    int get()  { while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
                 return i < s.size() ? s[i++] : -1; }
};

Ordinal parseExpr(OpState& st);
Ordinal parsePow(OpState& st);

// Accept an omega token in the input: ASCII 'w'/'W', the word "omega",
// or the UTF-8 encoding of ω (E0 8F 89). Returns true and consumes
// the input if matched. Does NOT consume on mismatch.
bool tryOmega(OpState& st) {
    int c = st.peek();
    if (c == 'w' || c == 'W') { st.get(); return true; }
    if (c == 'o') {                       // maybe "omega"
        size_t j = st.i;
        std::string w;
        while (j < st.s.size() && std::isalpha(static_cast<unsigned char>(st.s[j])))
            w += st.s[j++];
        if (w == "omega") { st.i = j; return true; }
    }
    if ((unsigned char)st.s[st.i] == 0xCF && st.i + 1 < st.s.size()) {   // UTF-8 ω = CF 89
        unsigned char b1 = static_cast<unsigned char>(st.s[st.i + 1]);
        if (b1 == 0x89) { st.i += 2; return true; }
    }
    return false;
}

Ordinal parseAtom(OpState& st) {
    if (tryOmega(st)) return Ordinal::omega();   // w / W / "omega" / UTF-8 ω (peek-based)
    int c = st.get();
    if (c == '(') {
        Ordinal v = parseExpr(st);
        if (st.get() != ')') throw std::runtime_error("Ordinal::parse: expected ')'");
        return v;
    }
    if (std::isdigit(static_cast<unsigned char>(c))) {
        int val = c - '0';
        while (std::isdigit(static_cast<unsigned char>(st.peek())))
            val = val * 10 + (st.get() - '0');
        return Ordinal::fromInt(val);
    }
    if (c == '-') return Ordinal::zero();   // tolerate a stray minus -> 0
    throw std::runtime_error("Ordinal::parse: unexpected character");
}

Ordinal parsePow(OpState& st) {
    Ordinal left = parseAtom(st);
    if (st.peek() == '^') { st.get(); Ordinal right = parsePow(st); return left.pow(right); }
    return left;
}

Ordinal parseMul(OpState& st) {
    Ordinal left = parsePow(st);
    for (;;) {
        // input multiplication token is '*' (ASCII). Display may use ·/×,
        // but those are multi-byte UTF-8 and are only emitted by to_string().
        if (st.peek() == '*') { st.get(); Ordinal right = parsePow(st); left = left * right; }
        else break;
    }
    return left;
}

Ordinal parseExpr(OpState& st) {
    Ordinal left = parseMul(st);
    for (;;) {
        if (st.peek() == '+') { st.get(); Ordinal right = parseMul(st); left = left + right; }
        else break;
    }
    return left;
}
} // namespace

inline Ordinal Ordinal::parse(const std::string& s) {
    OpState st; st.s = s; st.i = 0;
    Ordinal v = parseExpr(st);
    if (st.i < st.s.size())
        throw std::runtime_error("Ordinal::parse: trailing characters");
    return v;
}

} // namespace googology
