#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>

using namespace googology::number;

int main() {
    // to_string() emits LaTeX (no numeric evaluation)
    assert(Knuth("2 ^ 3").to_string() == "2 \\uparrow 3");
    assert(Knuth("2 ^^ 3").to_string() == "2 \\uparrow^{2} 3");
    assert(Knuth("3 ^^ 2").to_string() == "3 \\uparrow^{2} 2");

    // expand() returns the symbolic rewrite (LaTeX), not a value
    assert(Knuth("2 ^^ 3").expand(1) == "2 \\uparrow (2 \\uparrow^{2} 2)");
    assert(Knuth("2 ^^ 3").reduce() == "2 \\uparrow (2 \\uparrow 2)");

    // comparison is undefined -> must throw NotComparable
    bool threw = false;
    try {
        Knuth("2 ^ 3").compare(Knuth("2 ^^ 3"));
    } catch (const googology::NotComparable&) {
        threw = true;
    }
    assert(threw);

    // capability report
    assert(Knuth().can(googology::Op::FromString));
    assert(Knuth().can(googology::Op::ToString));
    assert(Knuth().can(googology::Op::Expand));
    assert(!Knuth().can(googology::Op::Compare));

    std::cout << "test_knuth: PASS\n";
    return 0;
}
