#include <iostream>
#include "googology/core/Notation.hpp"
#include "googology/core/Registry.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/notations/number/conway/Conway.hpp"

using namespace googology;
using namespace googology::number;

static const char* familyName(Family f) {
    switch (f) {
        case Family::Ordinal: return "ordinal";
        case Family::Number: return "number";
        case Family::Hierarchy: return "hierarchy";
    }
    return "?";
}

// Print the capability matrix for all registered notations.
static void printCapabilityMatrix() {
    std::cout << "name      family    compare?\n";
    std::cout << "-----------------------------\n";
    for (auto& name : registry().list()) {
        auto n = registry().create(name);
        if (!n) continue;
        std::cout << n->name();
        for (int i = 0; i < 10 - (int)n->name().size(); ++i) std::cout << ' ';
        std::cout << familyName(n->family()) << "   ";
        std::cout << (n->comparable() ? "yes" : "NO (undefined)") << "\n";
    }
}

int main() {
    // Register built-ins.
    registry().add("knuth", [] { return std::make_unique<Knuth>(); });
    registry().add("conway", [] { return std::make_unique<Conway>(); });

    printCapabilityMatrix();
    std::cout << "\n";

    // Knuth demos
    Knuth k1("2 ^ 3");
    std::cout << "knuth 2^3 = " << k1.evaluate() << "\n";
    Knuth k2("2 ^^ 3");
    std::cout << "knuth 2^^3 = " << k2.evaluate() << "\n";

    Knuth k3("2 ^^ 3");
    k3.expand(1);
    std::cout << "knuth 2^^3 expand(1) = " << k3.serialize()
              << "  (value still " << k3.evaluate() << ")\n";

    // Conway demos
    Conway c1("3 -> 2");
    std::cout << "conway 3->2 = " << c1.evaluate() << "\n";
    Conway c2("3 -> 3 -> 2");
    std::cout << "conway 3->3->2 = " << c2.evaluate() << "\n";

    // Comparison is undefined for large-number notations.
    try {
        k1.compare(k2);
    } catch (const NotComparable& e) {
        std::cout << "compare(knuth, knuth): " << e.what() << "\n";
    }

    return 0;
}
