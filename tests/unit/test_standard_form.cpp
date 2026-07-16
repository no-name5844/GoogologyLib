// Tests for the §12 standard-form decision, scoped to ORDINAL notations
// (core/StandardForm.hpp over core::Ordinal). The decision is NOT a generic
// property of arbitrary notations, so every case here is an ordinal.
#include <cassert>
#include <iostream>
#include <string>
#include "googology/core/StandardForm.hpp"
#include "googology/core/Ordinal.hpp"
#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include "googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp"

using namespace googology;
using namespace googology::ordinal;

static int g_fail = 0;
#define CHECK(cond)                                                  \
    do { if (!(cond)) {                                              \
        std::cerr << "FAIL: " #cond " @ line " << __LINE__ << "\n"; \
        ++g_fail; } } while (0)

// The default ordinal system has roots = {ω, ω^ω} and the article's
// fundamental sequences ω[n]=n, (ω^ω)[n]=ω^n. Reachable standard
// ordinals: {0,1,2,..., ω, ω^2, ω^3, ..., ω^ω}.
static const char* kStandard[] = {
    "0", "1", "5", "ω", "ω^2", "ω^5", "ω^ω", nullptr
};
// Ordinals that are genuine ordinals but NOT in this system's FS-closure
// (successor / additive / multiplicative forms the roots+FS cannot reach).
static const char* kNonStandard[] = {
    "ω + 1", "ω * 2", "ω^2 + ω", nullptr
};

static void test_bfs() {
    OrdinalSystem sys;
    for (int i = 0; kStandard[i]; ++i)
        CHECK(is_standard_bfs(sys, Ordinal::parse(kStandard[i])));
    for (int i = 0; kNonStandard[i]; ++i)
        CHECK(!is_standard_bfs(sys, Ordinal::parse(kNonStandard[i])));
    // convenience overload (default OrdinalSystem)
    CHECK(is_standard_bfs(Ordinal::parse("ω^3")));
    CHECK(!is_standard_bfs(Ordinal::parse("ω + 1")));
}

static void test_backtrack() {
    OrdinalSystem sys;
    for (int i = 0; kStandard[i]; ++i)
        CHECK(is_standard_backtrack(sys, Ordinal::parse(kStandard[i])));
    for (int i = 0; kNonStandard[i]; ++i)
        CHECK(!is_standard_backtrack(sys, Ordinal::parse(kNonStandard[i])));
    CHECK(is_standard_backtrack(Ordinal::parse("ω^ω")));   // a root
    CHECK(!is_standard_backtrack(Ordinal::parse("ω^2 + ω")));
}

// §12 decision now lives as a MEMBER of the ordinal-notation
// class: OrdinalNotation::is_standard(). It runs the generic engine
// within THIS notation system, using only the notation's OWN
// compare() (ordinal order) and expand() (its fundamental sequence).
// The roots() seed is each notation's MASTER LIMIT expression
// LIMIT, whose fundamental sequence is exactly limit(n) (user-specified).
static void test_member() {
    // Prss: master limit LIMIT = (0,1,2,...); limit(0)=(), limit(1)=(0),
    // limit(2)=(0,1). The bottom () and every limit(n) are reachable.
    CHECK(Prss::master_limit().is_standard());   // LIMIT itself -> standard
    CHECK(Prss("()").is_standard());            // limit(0)
    CHECK(Prss("(0)").is_standard());           // limit(1)
    CHECK(Prss("(0,1)").is_standard());        // limit(2)
    CHECK(Prss("(0,0)").is_standard());        // reachable from (0,1) by one expand
    CHECK(!Prss("(2)").is_standard());          // violates a_1=0, unreachable

    // EpspSS: master limit LIMIT = (1, ω); limit(0)=(), limit(n)=(1,n).
    CHECK(EpspSS::master_limit().is_standard()); // LIMIT -> standard
    CHECK(EpspSS("()").is_standard());           // limit(0)
    CHECK(EpspSS("(1,2)").is_standard());       // limit(2)
    CHECK(EpspSS("(1,5)").is_standard());       // limit(5)

    // EpsOmegaSS: same limit(n)=(1,n) convention.
    CHECK(EpsOmegaSS::master_limit().is_standard());
    CHECK(EpsOmegaSS("(1,3)").is_standard());
}

// The user-specified limit(n) API (design.md §12): the n-th term of
// the notation's limit expression.
static void test_limit() {
    // Prss
    CHECK(Prss::limit(0).to_string() == "()");
    CHECK(Prss::limit(1).to_string() == "(0)");
    CHECK(Prss::limit(2).to_string() == "(0, 1)");
    CHECK(Prss::limit(5).to_string() == "(0, 1, 2, 3, 4)");
    // ε_pSS / ε_ωSS
    CHECK(EpspSS::limit(0).to_string() == "()");
    CHECK(EpspSS::limit(1).to_string() == "(1, 1)");
    CHECK(EpspSS::limit(2).to_string() == "(1, 2)");
    CHECK(EpsOmegaSS::limit(0).to_string() == "()");
    CHECK(EpsOmegaSS::limit(3).to_string() == "(1, 3)");
    // LIMIT.expand(n) == limit(n); and LIMIT compares as the supremum.
    for (int n = 0; n <= 4; ++n) {
        Prss L = Prss::master_limit(); L.expand(n);
        CHECK(L.to_string() == Prss::limit(n).to_string());
        EpspSS E = EpspSS::master_limit(); E.expand(n);
        CHECK(E.to_string() == EpspSS::limit(n).to_string());
    }
    CHECK(Prss::master_limit().compare(Prss("(0,1)")) > 0);
    CHECK(Prss("(0,1)").compare(Prss::master_limit()) < 0);
}

int main() {
    test_bfs();
    test_backtrack();
    test_member();
    test_limit();
    if (g_fail == 0)
        std::cout << "test_standard_form: all assertions passed\n";
    else
        std::cout << "test_standard_form: " << g_fail << " FAILED\n";
    return g_fail ? 1 : 0;
}
