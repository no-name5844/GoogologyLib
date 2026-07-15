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

    // right-associative arrow tower is legal: 3^^3^^3 == 3 ↑↑ (3 ↑↑ 3)
    assert(Knuth("3^^3^^3").to_string() == "3 \\uparrow^{2} (3 \\uparrow^{2} 3)");
    // single-arrow tower: 3^3^3 == 3^(3^3)
    assert(Knuth("3^3^3").to_string() == "3 \\uparrow (3 \\uparrow 3)");

    // expand()/reduce() return a Knuth object; to_string() gives its LaTeX form
    assert(Knuth("2 ^^ 3").expand(1).to_string() == "2 \\uparrow (2 \\uparrow^{2} 2)");
    assert(Knuth("2 ^^ 3").reduce().to_string() == "2 \\uparrow (2 \\uparrow 2)");

    // expand() rewrites a tower term-by-term (recurses into the exponent)
    assert(Knuth("3^^3^^3").expand(1).to_string()
           == "3 \\uparrow^{2} (3 \\uparrow (3 \\uparrow^{2} 2))");

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
