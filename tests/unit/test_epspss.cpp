#include "googology/notations/ordinal/sequence/difference/epspss/EpspSS.hpp"
#include "googology/notations/ordinal/sequence/difference/epsilonomegass/EpsOmegaSS.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace googology;
using namespace googology::ordinal;

// EpspSS / EpsOmegaSS — difference-type ordinal sequence notations.
// Tests ONLY what the reference article defines (study/notations/
// "epsilon_nSS & epsilon_omegaSS.md"): expand(m)=expand(A,m),
// expand_to(M)=expandLen(A,M), and parse/serialize round-trip.
// The article does NOT define compare / normalize, so those are left
// to the base class (they throw) — see test_unsupported_ops().

static void test_eps_p_ss() {
    // p=2, A=(0,1,3): br=2 (a_2=1), L=1, q=2 <= p -> case 3 (+q)
    {
        EpspSS a(2, "(0, 1, 3)");
        assert(a.to_string() == "(0, 1, 3)");
        assert(a.p() == 2);
        EpspSS b(2, "(0, 1, 3)");
        assert(b.expand(0).to_string() == "(0, 1, 2)");    // expandLen(A,0)
        EpspSS c(2, "(0, 1, 3)");
        assert(c.expand(1).to_string() == "(0, 1, 2)");    // M = 1*1-1 = 0
        EpspSS d(2, "(0, 1, 3)");
        assert(d.expand(2).to_string() == "(0, 1, 2, 5)"); // +q=2 at tail
    }
    // p=5, A=(0,1,3): q=2 <= p -> case 3 (+q=2), same result
    {
        EpspSS a(5, "(0, 1, 3)");
        assert(a.expand(2).to_string() == "(0, 1, 2, 5)");
    }
    // p=2, A=(0,1,2): br=2, L=1, q=1 -> case 2 (no add)
    {
        EpspSS a(2, "(0, 1, 2)");
        assert(a.expand(2).to_string() == "(0, 1, 1, 2)"); // (m+n-L)th A
    }
    // parse variants
    {
        EpspSS a(2, "[0,1,3]");
        assert(a.to_string() == "(0, 1, 3)");
        EpspSS b(2, "0,1,3");
        assert(b.to_string() == "(0, 1, 3)");
    }
    // expand_to(M) = expandLen(A, M)
    {
        EpspSS a(2, "(0, 1, 3)");
        assert(a.expand_to(1).to_string() == "(0, 1, 2, 5)");
    }
}

static void test_eps_omega_ss() {
    // A=(0,1,3): q=2 -> case 3 (+q, unbounded)
    {
        EpsOmegaSS a("(0, 1, 3)");
        assert(a.to_string() == "(0, 1, 3)");
        EpsOmegaSS b("(0, 1, 3)");
        assert(b.expand(2).to_string() == "(0, 1, 2, 5)");
        EpsOmegaSS c("(0, 1, 2)");   // q=1 -> case 2
        assert(c.expand(2).to_string() == "(0, 1, 1, 2)");
        EpsOmegaSS d("(0, 1, 3)");
        assert(d.expand_to(1).to_string() == "(0, 1, 2, 5)");
    }
}

// The article does NOT define compare / normalize, so they are unsupported
// (base class throws). This documents that faithful behaviour.
static void test_unsupported_ops() {
    EpspSS a(2, "(0, 1, 3)");
    bool threw_compare = false;
    try { (void)a.compare(a); } catch (...) { threw_compare = true; }
    assert(threw_compare);
}

// The article's literal index (n+m-1)th A is out of range for inputs
// whose tail is too short (e.g. (0,4,4,4), L=3). Per the user's
// instruction we do NOT add a cyclic closure; the defensive guard throws.
static void test_out_of_range_throws() {
    EpspSS a(2, "(0, 4, 4, 4)");   // L=3, q=4>p -> case 4
    bool threw = false;
    try { a.expand(1); } catch (const std::out_of_range&) { threw = true; }
    assert(threw);
}

int main() {
    test_eps_p_ss();
    test_eps_omega_ss();
    test_unsupported_ops();
    test_out_of_range_throws();
    std::cout << "test_epspss: all assertions passed\n";
    return 0;
}
