#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>

using namespace googology::number;

int main() {
    // evaluate (direct recursion per spec)
    assert(Knuth("2 ^ 3").evaluate() == 8);
    assert(Knuth("2 ^^ 3").evaluate() == 16);
    assert(Knuth("3 ^^ 2").evaluate() == 27);

    // expand preserves the value
    Knuth e("2 ^^ 3");
    e.expand(1);
    assert(e.evaluate() == 16);

    // comparison is undefined -> must throw NotComparable
    bool threw = false;
    try {
        Knuth("2 ^ 3").compare(Knuth("2 ^^ 3"));
    } catch (const googology::NotComparable&) {
        threw = true;
    }
    assert(threw);

    // capability report
    assert(Knuth().can(googology::Op::Evaluate));
    assert(!Knuth().can(googology::Op::Compare));

    std::cout << "test_knuth: PASS\n";
    return 0;
}
