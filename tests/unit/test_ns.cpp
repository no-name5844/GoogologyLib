#include "googology/notations/real_sequence/ns/Ns.hpp"
#include "googology/core/Ordinal.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace googology;
using namespace googology::real_sequence;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAIL: " #cond " @ line " << __LINE__ << "\n"; ++failures; } } while (0)
#define CHECK_EQ(a, b) do { auto _a=(a); auto _b=(b); if (_a != _b) { std::cerr << "FAIL: " #a " == " #b "  (" << _a << " != " << _b << ") @ " << __LINE__ << "\n"; ++failures; } } while (0)

// --- metadata API (doc/design.md §3c) ---
static void test_metadata() {
    Ns x;
    CHECK_EQ(std::string(x.creator()), "送到本到, nnc");
    CHECK_EQ(std::string(x.version()), "1");
    CHECK_EQ(std::string(x.name()), "ns");
    CHECK(x.family() == Family::RealSequence);
    CHECK_EQ(std::string(x.subfamily()), "ns");
    // capabilities: FromString / ToString / Expand / Compare only
    CHECK(x.can(Op::FromString));
    CHECK(x.can(Op::ToString));
    CHECK(x.can(Op::Expand));
    CHECK(x.can(Op::Compare));
    CHECK(!x.can(Op::Normalize));   // article defines no standard form
    CHECK(!x.can(Op::Successor));   // article defines no successor
}

// --- symbolic expression + string round-trip ---
static void test_expression() {
    Ns a("1 th NS");
    CHECK_EQ(a.to_string(), "1 th \\mathbb{NS}");
    Ns b("ω th NS");
    CHECK_EQ(b.to_string(), "ω th \\mathbb{NS}");
    Ns c("ω+1 th NS");
    CHECK_EQ(c.to_string(), "ω + 1 th \\mathbb{NS}");
    // bare ordinal without "th" also accepted
    Ns d("3");
    CHECK_EQ(d.to_string(), "3 th \\mathbb{NS}");
    // params default to (2,2)
    CHECK(d.isDefaultParams());
    CHECK_EQ(d.params().first, 2);
    CHECK_EQ(d.params().second, 2);
}

// --- accumulated weight-fraction value (the "采用累加权重分数来表达" view) ---
// Only FINITE α yields an exact rational; limit indices are NOT auto-computed.
static void test_accumulated_fraction() {
    // Ns (n=2, m=2): a_1=0, a_2=1/2, a_3=3/4, a_4=7/8
    CHECK_EQ(Ns("1 th NS").value().to_string(), "0");
    CHECK_EQ(Ns("2 th NS").value().to_string(), "1/2");
    CHECK_EQ(Ns("3 th NS").value().to_string(), "3/4");
    CHECK_EQ(Ns("4 th NS").value().to_string(), "7/8");

    // rendered sum of step terms
    CHECK_EQ(Ns("1 th NS").to_fraction_string(), "0");
    CHECK_EQ(Ns("2 th NS").to_fraction_string(), "1/2");
    CHECK_EQ(Ns("3 th NS").to_fraction_string(), "1/2 + 1/4");
    CHECK_EQ(Ns("4 th NS").to_fraction_string(), "1/2 + 1/4 + 1/8");

    // n,m-Ns with n=3, m=2: a_1=0, a_2=1/3, a_3=4/9
    Ns nm(3, 2, "1 th NS");
    CHECK_EQ(nm.params().first, 3);
    CHECK_EQ(nm.params().second, 2);
    CHECK_EQ(Ns(3, 2, "2 th NS").value().to_string(), "1/3");
    CHECK_EQ(Ns(3, 2, "3 th NS").value().to_string(), "4/9");
    CHECK_EQ(Ns(3, 2, "3 th NS").to_fraction_string(), "1/3 + 1/9");

    // limit index -> NOT auto-computed (matches "不能自动计算")
    bool threw = false;
    try { (void)Ns("ω th NS").value(); } catch (const std::domain_error&) { threw = true; }
    CHECK(threw);
}

// --- accumulated weight-fraction form (the "累加权重分数" representation) ---
// The general form is  int + w1/d1 + w2/d2 + ...  (weights need not be 1).
static void test_weighted_fraction() {
    // NS finite index => int part 0, each weight 1 (terms 1/n^β).
    auto w = Ns("3 th NS").accumulated_weight_fractions();
    CHECK_EQ(w.intPart, 0);
    CHECK_EQ(w.terms.size(), 2u);
    CHECK_EQ(w.terms[0].first, 1);  CHECK_EQ(w.terms[0].second, 2);
    CHECK_EQ(w.terms[1].first, 1);  CHECK_EQ(w.terms[1].second, 4);
    CHECK_EQ(w.to_string(), "1/2 + 1/4");
    CHECK_EQ(w.value().to_string(), "3/4");

    // Example form: a weight-fraction with an integer part and
    // non-unit weights. Demonstrates the general capability.
    WeightedFractionSum ex;
    ex.intPart = 2;
    ex.terms = {{5, 11}, {62, 111}};
    CHECK_EQ(ex.to_string(), "2 + 5/11 + 62/111");
    // value == 2 + 5/11 + 62/111 (computed here to avoid hand-arithmetic)
    Rational expected(2, 1);
    expected += Rational(5, 11);
    expected += Rational(62, 111);
    CHECK(ex.value() == expected);
    CHECK_EQ(ex.value().to_string(), expected.to_string());
}

// --- expand: delegutes to core::Ordinal (§11 pluggable FS) ---
static void test_expand() {
    // limit index ω: expand(ω, k) = k  =>  a_{ω,k} = k th NS
    Ns a("ω th NS");
    CHECK_EQ(a.expand(2).to_string(), "2 th \\mathbb{NS}");
    Ns b("ω th NS");
    CHECK_EQ(b.expand(3).to_string(), "3 th \\mathbb{NS}");

    // finite / successor index has no fundamental sequence -> UnsupportedOperation
    bool threw = false;
    try { (void)Ns("2 th NS").expand(1); } catch (const UnsupportedOperation&) { threw = true; }
    CHECK(threw);
}

// --- compare: reduces to comparing the ordinal indices ---
static void test_compare() {
    auto cmp = [](const char* x, const char* y) {
        return Ns(x).compare(Ns(y));
    };
    CHECK_EQ(cmp("1 th NS", "1 th NS"), 0);
    CHECK_EQ(cmp("2 th NS", "3 th NS"), -1);
    CHECK_EQ(cmp("3 th NS", "2 th NS"), 1);
    CHECK_EQ(cmp("ω th NS", "3 th NS"), 1);    // ω > 3
    // anti-symmetry
    CHECK_EQ(cmp("2 th NS", "3 th NS"), -cmp("3 th NS", "2 th NS"));

    // cross-family comparison is undefined -> NotComparable
    bool threw = false;
    try { (void)Ns("2 th NS").compare(googology::number::Knuth("3^3")); }
    catch (const NotComparable&) { threw = true; }
    catch (...) {}
    CHECK(threw);
}

// --- core auxiliary function f(α) (spec §1.1 / §1.5) ---
static void test_f() {
    // Ns (n=2, m=2): f(0)=1, f(k)=2^k, f(ω)=f(expand(ω,2))=f(2)=4
    CHECK_EQ(Ns().f(0), 1);
    CHECK_EQ(Ns().f(1), 2);
    CHECK_EQ(Ns().f(2), 4);
    CHECK_EQ(Ns().f(3), 8);
    // limit ordinal ω: expand(ω,2)=2 -> f(ω)=f(2)=4
    CHECK_EQ(Ns("ω th NS").f(Ordinal::parse("ω")), 4);
    // n,m-Ns (n=3, m=2): f(k)=3^k, f(ω)=f(expand(ω,2))=f(2)=9
    Ns nm(3, 2, "1 th NS");
    CHECK_EQ(nm.f(0), 1);
    CHECK_EQ(nm.f(1), 3);
    CHECK_EQ(nm.f(2), 9);
    CHECK_EQ(nm.f(3), 27);
    CHECK_EQ(nm.f(Ordinal::parse("ω")), 9);
}

int main() {
    test_metadata();
    test_expression();
    test_accumulated_fraction();
    test_weighted_fraction();
    test_f();
    test_expand();
    test_compare();
    if (failures == 0) std::cout << "test_ns: all assertions passed\n";
    else std::cout << "test_ns: " << failures << " FAILURE(S)\n";
    return failures == 0 ? 0 : 1;
}
