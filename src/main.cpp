#include <iostream>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"
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

static const char* opName(Op o) {
    switch (o) {
        case Op::FromString: return "FromString";
        case Op::ToString: return "ToString";
        case Op::Normalize: return "Normalize";
        case Op::Compare: return "Compare";
        case Op::Expand: return "Expand";
        case Op::ExpandTo: return "ExpandTo";
        case Op::Successor: return "Successor";
    }
    return "?";
}

// Print the capability matrix for all registered notations.
static void printCapabilityMatrix() {
    std::cout << "name      family    compare?   ops\n";
    std::cout << "--------------------------------------------\n";
    for (auto& name : registry().list()) {
        auto n = registry().create(name);
        if (!n) continue;
        std::cout << n->name();
        for (int i = 0; i < 10 - (int)n->name().size(); ++i) std::cout << ' ';
        std::cout << familyName(n->family()) << "   ";
        std::cout << (n->comparable() ? "yes" : "NO (undefined)") << "   ";
        bool first = true;
        for (uint8_t o = 0; o < 8; ++o) {
            if (n->can(static_cast<Op>(o))) {
                if (!first) std::cout << ",";
                std::cout << opName(static_cast<Op>(o));
                first = false;
            }
        }
        std::cout << "\n";
    }
}

int main() {
    // Register built-ins.
    registry().add("knuth", [] { return std::make_unique<Knuth>(); });
    registry().add("conway", [] { return std::make_unique<Conway>(); });

    printCapabilityMatrix();
    std::cout << "\n";

    // Knuth demos (output is LaTeX; the library never computes a value)
    Knuth k1("2 ^ 3");
    std::cout << "knuth 2^3    = " << k1.to_string() << "\n";
    Knuth k2("2 ^^ 3");
    std::cout << "knuth 2^^3   = " << k2.to_string() << "\n";
    std::cout << "knuth 2^^3 expand(1) = " << k2.expand(1) << "\n";
    std::cout << "knuth 2^^3 expand(2) = " << k2.expand(2) << "\n";
    std::cout << "knuth 2^^3 reduce()   = " << k2.reduce() << "\n";

    // Conway demos
    Conway c1("3 -> 2");
    std::cout << "conway 3->2        = " << c1.to_string() << "\n";
    Conway c2("3 -> 3 -> 2");
    std::cout << "conway 3->3->2      = " << c2.to_string() << "\n";
    std::cout << "conway 3->3->2 exp1 = " << c2.expand(1) << "\n";
    std::cout << "conway 3->3->2 exp2 = " << c2.expand(2) << "\n";
    std::cout << "conway 3->3->2 red  = " << c2.reduce() << "\n";

    // print() / operator<< both emit LaTeX
    std::cout << "operator<< : "; std::cout << k2 << "\n";
    std::cout << "print()    : "; k2.print(); std::cout << "\n";

    // Comparison is undefined for large-number notations.
    try {
        k1.compare(k2);
    } catch (const NotComparable& e) {
        std::cout << "compare(knuth,knuth): " << e.what() << "\n";
    }

    return 0;
}
