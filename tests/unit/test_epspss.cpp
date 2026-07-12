#include "googology/notations/ordinal/sequence/difference/epspss/EpspSS.hpp"
#include "googology/notations/ordinal/sequence/difference/epsilonomegass/EpsOmegaSS.hpp"
#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace googology;
using namespace googology::ordinal;

// EpspSS / EpsOmegaSS — difference-type ordinal sequence notations.
// Per the (corrected) reference article, valid expressions require a_1 = 1
// (the limit expression is of the form (1, n)). Tests use ONLY a_1 = 1
// sequences. expand(m)=expand(A,m), expand_to(M)=expandLen(A,M),
// parse/serialize round-trip.
//
// isSuccessor() follows the article's successor clause (ends in 1).
// normalize() computes the standard form (标准型) via the project's UNIVERSAL
// standard-form definition (an expression is a legal expression iff obtainable
// from a limit expression by finitely many expansions + taking a prefix); the
// user-specified limit expression for both ε_pSS and ε_ωSS is (1, n), so the
// canonical starter is (1, a_2). The article does NOT define compare, but
// the user specifies the notation admits lexicographic ordinal comparison
// under standard form (consistent with Prss) — see test_compare().
//
// NOTE: the article's case 3 (gap a_n = a_br + q) appends (q-1), NOT q.

static void test_eps_p_ss() {
    // p=2, A=(1,1,3): br=2 (a_2=1), L=1, q=2 <= p -> case 3 (+q-1 = +1)
    {
        EpspSS a(2, "(1, 1, 3)");
        assert(a.to_string() == "(1, 1, 3)");
        assert(a.p() == 2);
        EpspSS b(2, "(1, 1, 3)");
        assert(b.expand(0).to_string() == "(1, 1, 2)");    // expandLen(A,0)
        EpspSS c(2, "(1, 1, 3)");
        assert(c.expand(1).to_string() == "(1, 1, 2)");    // M = 1*1-1 = 0
        EpspSS d(2, "(1, 1, 3)");
        assert(d.expand(2).to_string() == "(1, 1, 2, 3)"); // +q-1=1 at running seq
    }
    // p=5, A=(1,1,3): q=2 <= p -> case 3 (+q-1=1), same result
    {
        EpspSS a(5, "(1, 1, 3)");
        assert(a.expand(2).to_string() == "(1, 1, 2, 3)");
    }
    // p=2, A=(1,2): br=1, L=1, q=1 -> case 2 (no add)
    {
        EpspSS a(2, "(1, 2)");
        assert(a.expand(1).to_string() == "(1, 1)");
        EpspSS b(2, "(1, 2)");
        assert(b.expand(2).to_string() == "(1, 1, 1)"); // (m+n-L)th running
    }
    // p=1, A=(1,4): br=1, L=1, q=3 > p -> case 4 (capped at p=1) [UNCHANGED by q-1]
    {
        EpspSS a(1, "(1, 4)");
        assert(a.expand(1).to_string() == "(1, 3)");
        EpspSS b(1, "(1, 4)");
        assert(b.expand(2).to_string() == "(1, 3, 4)");
        EpspSS c(1, "(1, 4)");
        assert(c.expand(3).to_string() == "(1, 3, 4, 5)");
    }
    // p=4, A=(1,4): q=3 <= p=4 -> case 3 (+q-1 = +2), differs from the p=1 cap
    {
        EpspSS a(4, "(1, 4)");
        assert(a.expand(2).to_string() == "(1, 3, 5)");
        EpspSS b(4, "(1, 4)");
        assert(b.expand(3).to_string() == "(1, 3, 5, 7)");
    }
    // parse variants
    {
        EpspSS a(2, "[1,1,3]");
        assert(a.to_string() == "(1, 1, 3)");
        EpspSS b(2, "1,1,3");
        assert(b.to_string() == "(1, 1, 3)");
    }
    // expand_to(M) = expandLen(A, M)
    {
        EpspSS a(2, "(1, 1, 3)");
        assert(a.expand_to(1).to_string() == "(1, 1, 2, 3)");
    }
}

static void test_eps_omega_ss() {
    // A=(1,1,3): q=2 -> case 3 (+q-1 = +1, unbounded)
    {
        EpsOmegaSS a("(1, 1, 3)");
        assert(a.to_string() == "(1, 1, 3)");
        EpsOmegaSS b("(1, 1, 3)");
        assert(b.expand(1).to_string() == "(1, 1, 2)");
        EpsOmegaSS c("(1, 1, 3)");
        assert(c.expand(2).to_string() == "(1, 1, 2, 3)");
        EpsOmegaSS d("(1, 2)");   // q=1 -> case 2
        assert(d.expand(2).to_string() == "(1, 1, 1)");
        EpsOmegaSS e("(1, 4)");    // q=3 -> case 3 (+q-1 = +2, unbounded)
        assert(e.expand(2).to_string() == "(1, 3, 5)");
        EpsOmegaSS f("(1, 1, 3)");
        assert(f.expand_to(1).to_string() == "(1, 1, 2, 3)");
    }
}

// The article does NOT define compare, but per the user's specification the
// notation satisfies lexicographic ordinal comparison under standard form
// (consistent with Prss). compare() orders two standard sequences
// lexicographically; cross-type arguments throw NotComparable.
// normalize()/isSuccessor() are implemented via the universal standard-form
// definition + successor clause — see test_indexer_and_normalize().
static void test_compare() {
    // --- EpspSS: lexicographic, same p ---
    {
        EpspSS a(2, "(1, 1, 3)"), b(2, "(1, 1, 3)");
        assert(a.compare(b) == 0);
        EpspSS c(2, "(1, 1, 3)"), d(2, "(1, 1, 4)");
        assert(c.compare(d) == -1);
        assert(d.compare(c) == 1);
        EpspSS e(2, "(1, 1, 3)"), f(2, "(1, 2)");
        assert(e.compare(f) == -1);   // index1: 1 vs 2
        assert(f.compare(e) == 1);
        EpspSS g(2, "(1, 2)"), h(2, "(1, 2, 1)");
        assert(g.compare(h) == -1);   // g shorter -> smaller
    }
    // --- EpsOmegaSS: lexicographic ---
    {
        EpsOmegaSS a("(1, 1, 3)"), b("(1, 1, 4)");
        assert(a.compare(b) == -1);
        assert(b.compare(a) == 1);
        EpsOmegaSS c("(1, 2)"), d("(1, 1, 3)");
        assert(c.compare(d) == 1);
    }
    // --- cross-type throws NotComparable ---
    {
        EpspSS a(2, "(1, 1, 3)");
        EpsOmegaSS w("(1, 1, 3)");
        bool threw = false;
        try { (void)a.compare(w); } catch (const NotComparable&) { threw = true; }
        assert(threw);
        bool threw2 = false;
        try { (void)w.compare(Prss("(0, 1, 2)")); } catch (const NotComparable&) { threw2 = true; }
        assert(threw2);
    }
    // --- comparable() reflects the Compare capability ---
    {
        EpspSS a(2, "(1, 1, 3)");
        assert(a.comparable() == true);
    }
}

// A[n] = expand(A, n), realized as operator[]; it must NOT mutate *this,
// so chained indexing A[1], A[2] is independent. normalize() is the
// standard-form (标准型) rewriter: a standard expression is left unchanged
// (idempotent).
static void test_indexer_and_normalize() {
    // --- EpspSS: A[n] equals expand(A,n), A untouched ---
    {
        EpspSS A(1, "(1, 4)");          // p=1 -> case 4 (capped at p=1)
        EpspSS pristine = A;
        assert(A[1].to_string() == "(1, 3)");
        assert(A[2].to_string() == "(1, 3, 4)");
        assert(A.to_string() == pristine.to_string());   // not mutated
        // independent chaining
        EpspSS B(1, "(1, 4)");
        (void)B[1];
        assert(B[2].to_string() == "(1, 3, 4)");  // B[2] unaffected by B[1]
    }
    // --- EpsOmegaSS: A[n] equals expand(A,n), A untouched ---
    {
        EpsOmegaSS A("(1, 4)");          // case 3 unbounded (+q-1=+2)
        EpsOmegaSS pristine = A;
        assert(A[2].to_string() == "(1, 3, 5)");
        assert(A.to_string() == pristine.to_string());
    }
    // --- Prss: A[n] equals expand(A,n), A untouched ---
    {
        Prss A("(0, 1, 2)");
        Prss pristine = A;
        assert(A[2].to_string() == "(0, 1, 1, 1)");
        assert(A.to_string() == pristine.to_string());
    }
    // --- isSuccessor(): ends in 1 ---
    assert((EpspSS(2, "(1, 2, 1)").isSuccessor()) == true);
    assert((EpspSS(1, "(1, 4)").isSuccessor()) == false);
    assert((EpsOmegaSS("(1, 2, 1)").isSuccessor()) == true);
    // --- normalize(): standard form is idempotent ---
    {
        EpspSS a(2, "(1, 2, 3)"); a.normalize();
        assert(a.to_string() == "(1, 2, 3)");
    }
    {
        EpsOmegaSS a("(1, 2, 3)"); a.normalize();
        assert(a.to_string() == "(1, 2, 3)");
    }
}

// The article leaves br=0 (no element < a_n) undefined. With a_1 = 1 this
// is reachable only when a_n < a_1, e.g. a decreasing tail A=(1,0)
// (a_n=0, nothing is < 0). Per the user's instruction we add NO closure;
// a defensive guard throws. Note: A=(1) is NOT this case — it ends in 1,
// so it is a successor and is stripped to ().
static void test_out_of_range_throws() {
    EpspSS a(2, "(1, 0)");   // a_n = 0, no element < 0 -> br = 0 -> article undefined
    bool threw = false;
    try { a.expand(1); } catch (const std::out_of_range&) { threw = true; }
    assert(threw);
    EpsOmegaSS w("(1, 0)");
    bool threw2 = false;
    try { w.expand(1); } catch (const std::out_of_range&) { threw2 = true; }
    assert(threw2);
}

int main() {
    test_eps_p_ss();
    test_eps_omega_ss();
    test_compare();
    test_out_of_range_throws();
    test_indexer_and_normalize();
    std::cout << "test_epspss: all assertions passed\n";
    return 0;
}
