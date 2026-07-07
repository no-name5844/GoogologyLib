#include "googology/notations/number/conway/Conway.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>

using namespace googology::number;

int main() {
    // to_string() emits LaTeX (no numeric evaluation)
    assert(Conway("3 -> 2").to_string() == "3^{2}");
    assert(Conway("3 -> 3 -> 2").to_string() == "3 \\rightarrow 3 \\rightarrow 2");

    // expand() returns a Conway object; to_string() gives its LaTeX form
    assert(Conway("3 -> 3 -> 2").expand(1).to_string() ==
           "3 \\rightarrow (3 \\rightarrow 2 \\rightarrow 2) \\rightarrow 1");
    assert(Conway("3 -> 3 -> 2").expand(2).to_string() ==
           "3 \\rightarrow (3 \\rightarrow 2 \\rightarrow 2)");
    // reduce() actually rewrites the object (never to a bare number)
    assert(Conway("3 -> 3 -> 2").reduce().to_string() !=
           Conway("3 -> 3 -> 2").to_string());

    // comparison is undefined -> must throw NotComparable
    bool threw = false;
    try {
        Conway("3 -> 2").compare(Conway("4 -> 3"));
    } catch (const googology::NotComparable&) {
        threw = true;
    }
    assert(threw);

    assert(Conway().can(googology::Op::FromString));
    assert(Conway().can(googology::Op::ToString));
    assert(Conway().can(googology::Op::Expand));
    assert(!Conway().can(googology::Op::Compare));

    std::cout << "test_conway: PASS\n";
    return 0;
}
