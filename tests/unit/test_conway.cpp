#include "googology/notations/number/conway/Conway.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>

using namespace googology::number;

int main() {
    // evaluate (recursion per spec)
    assert(Conway("3 -> 2").evaluate() == 9);
    assert(Conway("4 -> 3").evaluate() == 64);
    assert(Conway("3 -> 3 -> 2").evaluate() == googology::ipow(3, 27));

    // expand preserves the value
    Conway e("3 -> 3 -> 2");
    e.expand(1);
    assert(e.evaluate() == googology::ipow(3, 27));

    // comparison is undefined -> must throw NotComparable
    bool threw = false;
    try {
        Conway("3 -> 2").compare(Conway("4 -> 3"));
    } catch (const googology::NotComparable&) {
        threw = true;
    }
    assert(threw);

    assert(Conway().can(googology::Op::Evaluate));
    assert(!Conway().can(googology::Op::Compare));

    std::cout << "test_conway: PASS\n";
    return 0;
}
