#include "googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp"
#include "googology/core/Ordinal.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace googology;
using namespace googology::ordinal;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAIL: " #cond " @ line " << __LINE__ << "\n"; ++failures; } } while (0)
#define CHECK_EQ(a, b) do { auto _a=(a); auto _b=(b); if (_a != _b) { std::cerr << "FAIL: " #a " == " #b "  (" << _a << " != " << _b << ") @ " << __LINE__ << "\n"; ++failures; } } while (0)

// --- ordinal core sanity (CNF arithmetic + parser) ---
static void test_ordinal_core() {
    CHECK_EQ(Ordinal::parse("ω").to_string(), "ω");
    CHECK_EQ(Ordinal::parse("ω+1").to_string(), "ω + 1");
    // ω+1 is a genuine successor (finite trailing part), distinct from ω.
    CHECK(Ordinal::parse("ω+1").isSuccessor());
    CHECK(Ordinal::parse("ω+1").predecessor().equals(Ordinal::parse("ω")));
    CHECK(Ordinal::parse("2").isSuccessor());
    CHECK(Ordinal::parse("2").predecessor().equals(Ordinal::parse("1")));
    CHECK(Ordinal::parse("ω*2 + 1").isSuccessor());
    CHECK(Ordinal::parse("ω*2 + 1").predecessor().equals(Ordinal::parse("ω*2")));
    CHECK_EQ(Ordinal::parse("ω*2").compare(Ordinal::parse("ω^2")), -1); // ω·2 < ω^2
    CHECK(Ordinal::parse("ω+5").subtract(Ordinal::parse("ω")).equals(Ordinal::parse("5")));
    CHECK_EQ(Ordinal::parse("ω").expand(3).to_string(), "3");   // C2: ω's FS = n
    bool threw = false;
    try { (void)Ordinal::parse("ω+1").expand(1); } catch (const std::domain_error&) { threw = true; }
    CHECK(threw);   // successor has no fundamental sequence
}

// --- the 6 weak-Veblen expand cases (study/notations/weak-Veblen-like notation.md) ---
static void test_wv_cases() {
    // Case 1: A=(0) -> n
    { WeakVeblen A("(0@0)"); CHECK_EQ(A.expand(3).to_string(), "3"); }
    // Case 2: A=(0@b1,#) -> (#)
    { WeakVeblen A("(0@5, 1@3)"); CHECK_EQ(A.expand(1).to_string(), "(1@3)"); }
    // Case 3: A=(#,(a+1)@0)
    // (expand() mutates *this, so use a fresh object per n)
    { WeakVeblen A("(2@3, 1@0)");
      CHECK_EQ(A.expand(0).to_string(), "(2@3, 0@0) + 1"); }
    { WeakVeblen A("(2@3, 1@0)");
      CHECK_EQ(A.expand(1).to_string(), "((2@3, 0@0) ^ (2@3, 0@0) + 1)"); }
    // Case 4: A=(#,(a+1)@(b+1))
    { WeakVeblen A("(2@3, 1@1)");
      CHECK_EQ(A.expand(0).to_string(), "(2@3, 0@1) + 1"); }
    { WeakVeblen A("(2@3, 1@1)");
      CHECK_EQ(A.expand(1).to_string(), "(2@3, 0@1, ((2@3, 0@1) + 1)@0)"); }
    // Case 5: A=(#,(a+1)@b) with b a limit (ω)
    { WeakVeblen A("(2@3, 1@ω)");
      CHECK_EQ(A.expand(3).to_string(), "(2@3, 0@ω, 1@3)"); }
    // Case 6: A=(#,a@b) with a a limit (ω)
    { WeakVeblen A("(2@4, ω@3)");
      CHECK_EQ(A.expand(3).to_string(), "(2@4, 3@3)"); }
}

// --- compare (article §1.3: is_equal / is_greater / compare) ---
// Lexicographic: PRIMARY key = second coord (i / @b), SECONDARY = first
// coord (a / @a); when prefixes match, the longer expression wins.
static void test_wv_compare() {
    auto cmp = [](const char* x, const char* y) {
        return WeakVeblen(x).compare(WeakVeblen(y));
    };
    CHECK_EQ(cmp("(0@0)", "(0@0)"), 0);                 // identical
    CHECK_EQ(cmp("(1@0)", "(0@0)"), 1);                 // secondary a: 1 > 0
    CHECK_EQ(cmp("(0@1)", "(0@0)"), 1);                 // primary i: 1 > 0
    CHECK_EQ(cmp("(0@0)", "(0@1)"), -1);                // mirror
    CHECK_EQ(cmp("(1@0, 1@0)", "(1@0)"), 1);          // prefix rule: longer wins
    CHECK_EQ(cmp("(1@0)", "(1@0, 1@0)"), -1);         // mirror
    CHECK_EQ(cmp("(1@0, 0@0)", "(1@0, 1@0)"), -1);   // comp1 a: 0 < 1
    CHECK_EQ(cmp("(2@3, 1@0)", "(2@3, 0@0)"), 1);   // comp1 a: 1 > 0
    CHECK_EQ(cmp("(0@5)", "(100@0)"), 1);                // primary i dominates (5 > 0)
    CHECK_EQ(cmp("(5@0)", "(1@0)"), 1);                  // i equal, secondary a: 5 > 1
    // anti-symmetry
    CHECK_EQ(cmp("(0@1)", "(0@0)"), -cmp("(0@0)", "(0@1)"));
    // capabilities report Compare
    CHECK(WeakVeblen("(1@0)").can(Op::Compare));

    // cross-family comparison is undefined -> NotComparable
    bool threw = false;
    try { (void)WeakVeblen("(1@0)").compare(googology::number::Knuth("3^3")); }
    catch (const NotComparable&) { threw = true; }
    catch (...) {}
    CHECK(threw);
}

int main() {
    test_ordinal_core();
    test_wv_cases();
    test_wv_compare();
    if (failures == 0) std::cout << "test_weakveblen: all assertions passed\n";
    else std::cout << "test_weakveblen: " << failures << " FAILURE(S)\n";
    return failures == 0 ? 0 : 1;
}
