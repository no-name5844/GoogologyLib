#include "googology/notations/ordinal/sequence/difference/epspss/EpspSS.hpp"
#include "googology/notations/ordinal/sequence/difference/epsilonomegass/EpsOmegaSS.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>

using namespace googology;
using namespace googology::ordinal;
using namespace googology::number;

// ε_pSS / ε_ωSS — difference-type ordinal sequence notations.
// expand(m) = expand(A, m*L-1); expand_to(M) = expandLen(A, M).
// Hand-computed references below (see spec/notations/epsilon_{p_ss,omega_ss}.md).

static void test_eps_p_ss() {
    // p=2, A=(0,1,3): gap q=3 > p -> case 4 (+p)
    {
        EpspSS a(2, "(0, 1, 3)");
        assert(a.to_string() == "(0, 1, 3)");
        EpspSS b(2, "(0, 1, 3)");
        assert(b.expand(0).to_string() == "(0, 1, 2)");   // just decrement last
        EpspSS c(2, "(0, 1, 3)");
        assert(c.expand(1).to_string() == "(0, 1, 2)");   // M = 1*1-1 = 0
        EpspSS d(2, "(0, 1, 3)");
        assert(d.expand(2).to_string() == "(0, 1, 2, 5)");  // M=1: +p=2 at tail
    }
    // p=2, A=(0,4,4,4): tail L=3, q=4 > p -> case 4 (+p), wraps
    {
        EpspSS a(2, "(0, 4, 4, 4)");
        assert(a.expand(0).to_string() == "(0, 4, 4, 3)");
        EpspSS b(2, "(0, 4, 4, 4)");
        assert(b.expand(1).to_string() == "(0, 4, 4, 3, 6, 6)");
    }
    // p=5, A=(0,1,3): br=2 (a_2=1), q = 3-1 = 2 <= p -> case 3 (+q=2)
    {
        EpspSS a(5, "(0, 1, 3)");
        assert(a.expand(2).to_string() == "(0, 1, 2, 5)");  // +q=2 at tail
    }
    // parse variants
    {
        EpspSS a(2, "[0,1,3]");
        assert(a.to_string() == "(0, 1, 3)");
        EpspSS b(2, "0,1,3");
        assert(b.to_string() == "(0, 1, 3)");
        assert(b.p() == 2);
    }
    // expand_to (length-parameterized)
    {
        EpspSS a(2, "(0, 1, 3)");
        assert(a.expand_to(2).to_string() == "(0, 1, 2, 5, 5)");
    }
}

static void test_eps_omega_ss() {
    // A=(0,1,3): q=3 added in full (unbounded)
    {
        EpsOmegaSS a("(0, 1, 3)");
        assert(a.to_string() == "(0, 1, 3)");
        EpsOmegaSS b("(0, 1, 3)");
        assert(b.expand(0).to_string() == "(0, 1, 2)");
        EpsOmegaSS c("(0, 1, 3)");
        assert(c.expand(1).to_string() == "(0, 1, 2)");   // M=0
        EpsOmegaSS d("(0, 1, 3)");
        assert(d.expand(2).to_string() == "(0, 1, 2, 5)");  // br=2,a_k=1,q=2 added
    }
    // A=(0,4,4,4): tail L=3, q=4 added in full, wraps
    {
        EpsOmegaSS a("(0, 4, 4, 4)");
        assert(a.expand(0).to_string() == "(0, 4, 4, 3)");
        EpsOmegaSS b("(0, 4, 4, 4)");
        assert(b.expand(1).to_string() == "(0, 4, 4, 3, 8, 8)");
    }
    // expand_to
    {
        EpsOmegaSS a("(0, 1, 3)");
        assert(a.expand_to(2).to_string() == "(0, 1, 2, 5, 5)");
    }
}

static void test_compare() {
    // lexicographic within the same family
    EpspSS a(2, "(0, 1, 3)"), b(2, "(0, 1, 2)");
    assert(a.compare(b) == 1);
    EpspSS c(2, "(0, 1, 2)"), d(2, "(0, 1, 2)");
    assert(c.compare(d) == 0);
    EpspSS e(2, "(0, 1)"), f(2, "(0, 1, 2)");
    assert(e.compare(f) == -1);   // shorter is smaller
    // cross-family comparison throws
    EpspSS g(2, "(0, 1, 3)");
    Knuth h("2 ^ 3");
    bool threw = false;
    try { g.compare(h); } catch (const NotComparable&) { threw = true; }
    assert(threw);
}

int main() {
    test_eps_p_ss();
    test_eps_omega_ss();
    test_compare();
    std::cout << "test_epspss: all assertions passed\n";
    return 0;
}
