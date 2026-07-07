#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>

using namespace googology::ordinal;

int main() {
    // to_string() emits LaTeX-style (0, 1, 2); parse accepts ASCII
    assert(Prss("(0, 1, 2)").to_string() == "(0, 1, 2)");
    assert(Prss("0,1,2").to_string() == "(0, 1, 2)");
    assert(Prss("[0,1,2]").to_string() == "(0, 1, 2)");

    // expand(m) per spec/notations/prss.md (cyclic tail wrap).
    // (0,1,2): n=3, a_n=2, br=2, L=1, tail=(2).
    assert(Prss("(0,1,2)").expand(0).to_string() == "(0, 1, 1)");
    assert(Prss("(0,1,2)").expand(1).to_string() == "(0, 1, 1)");
    assert(Prss("(0,1,2)").expand(2).to_string() == "(0, 1, 1, 2)");

    // (0,1,2,2,2): n=5, a_n=2, br=2, L=3, tail=(2,2,2).
    assert(Prss("(0,1,2,2,2)").expand(0).to_string() == "(0, 1, 2, 2, 1)");
    assert(Prss("(0,1,2,2,2)").expand(1).to_string() == "(0, 1, 2, 2, 1, 2, 2)");

    // ends with 1: expand drops the trailing 1
    assert(Prss("(0,1,2,1)").expand(1).to_string() == "(0, 1, 2)");

    // expand_to(M) = expandLen(A, M): decrement last, append M tail elements.
    // (0,1,2) tail=(2): expand_to(3) -> (0,1,1) + 2,2,2 = (0,1,1,2,2,2)
    assert(Prss("(0,1,2)").expand_to(3).to_string() == "(0, 1, 1, 2, 2, 2)");

    // compare() is lexicographic (from YSequence.cpp _compare)
    assert(Prss("(0,1,2)").compare(Prss("(0,1,3)")) == -1);
    assert(Prss("(0,1,3)").compare(Prss("(0,1,2)")) == 1);
    assert(Prss("(0,1,2)").compare(Prss("(0,1,2)")) == 0);
    assert(Prss("(0,1,2)").compare(Prss("(0,1,2,0)")) == -1); // prefix < longer
    assert(Prss("(0,1,2,0)").compare(Prss("(0,1,2)")) == 1);

    // cross-family compare must throw NotComparable
    bool threw = false;
    try {
        Prss("(0,1,2)").compare(googology::number::Knuth("2 ^ 3"));
    } catch (const googology::NotComparable&) {
        threw = true;
    }
    assert(threw);

    // capability report
    Prss p;
    assert(p.can(googology::Op::FromString));
    assert(p.can(googology::Op::ToString));
    assert(p.can(googology::Op::Normalize));
    assert(p.can(googology::Op::Compare));
    assert(p.can(googology::Op::Expand));
    assert(p.can(googology::Op::ExpandTo));
    assert(p.can(googology::Op::Successor));

    std::cout << "test_prss: PASS\n";
    return 0;
}
