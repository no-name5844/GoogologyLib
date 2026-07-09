#include <iostream>
#include <iomanip>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"
#include "googology/core/Registry.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/notations/number/conway/Conway.hpp"
#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include "googology/notations/ordinal/sequence/difference/epspss/EpspSS.hpp"
#include "googology/notations/ordinal/sequence/difference/epsilonomegass/EpsOmegaSS.hpp"

using namespace googology;
using namespace googology::number;
using namespace googology::ordinal;

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
        std::cout << std::left << std::setw(18) << n->name()
                  << familyName(n->family()) << "   ";
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
    registry().add("prss", [] { return std::make_unique<Prss>(); });
    registry().add("epsilon_p_ss", [] { return std::make_unique<EpspSS>(); });
    registry().add("epsilon_omega_ss", [] { return std::make_unique<EpsOmegaSS>(); });

    printCapabilityMatrix();
    std::cout << "\n";

    // Knuth demos (output is LaTeX; the library never computes a value).
    // expand()/reduce() return a Knuth object; to_string() renders the LaTeX.
    Knuth k1("2 ^ 3");
    std::cout << "knuth 2^3    = " << k1.to_string() << "\n";
    Knuth k2("2 ^^ 3");
    std::cout << "knuth 2^^3   = " << k2.to_string() << "\n";
    std::cout << "knuth 2^^3 expand(1) = " << k2.expand(1).to_string() << "\n";
    std::cout << "knuth 2^^3 expand(2) = " << k2.expand(2).to_string() << "\n";
    std::cout << "knuth 2^^3 reduce()   = " << k2.reduce().to_string() << "\n";

    // Conway demos (expand()/reduce() return a Conway object)
    Conway c1("3 -> 2");
    std::cout << "conway 3->2        = " << c1.to_string() << "\n";
    Conway c2("3 -> 3 -> 2");
    std::cout << "conway 3->3->2      = " << c2.to_string() << "\n";
    std::cout << "conway 3->3->2 exp1 = " << c2.expand(1).to_string() << "\n";
    std::cout << "conway 3->3->2 exp2 = " << c2.expand(2).to_string() << "\n";
    std::cout << "conway 3->3->2 red  = " << c2.reduce().to_string() << "\n";

    // PrSS demos (ordinal / 阶差型). expand()/expand_to() return a Prss object;
    // to_string() emits LaTeX. Compare is lexicographic (defined for ordinals).
    Prss p1("(0, 1, 2)");
    std::cout << "prss (0,1,2)            = " << p1.to_string() << "\n";
    Prss p2("(0, 1, 2)");
    std::cout << "prss (0,1,2) expand(1)  = " << p2.expand(1).to_string() << "\n";
    Prss p3("(0, 1, 2)");
    std::cout << "prss (0,1,2) expand(2)  = " << p3.expand(2).to_string() << "\n";
    Prss p4("(0, 1, 2, 2, 2)");
    std::cout << "prss (0,1,2,2,2) exp(1) = " << p4.expand(1).to_string() << "\n";

    Prss pa("(0, 1, 2)"), pb("(0, 1, 3)");
    std::cout << "compare((0,1,2),(0,1,3)) = " << pa.compare(pb) << "\n";
    Prss pc("(0, 1, 2)"), pd("(0, 1, 2, 0)");
    std::cout << "compare((0,1,2),(0,1,2,0)) = " << pc.compare(pd) << "\n";

    Prss pn("(0, 1, 2, 1)");
    pn.normalize();
    std::cout << "prss (0,1,2,1) normalize  = " << pn.to_string() << "\n";

    // ε_pSS demos — implemented EXACTLY per the article
    // (study/notations/epsilon_nSS & epsilon_omegaSS.md). expand(m)=expand(A,m);
    // expand_to(M)=expandLen(A,M); no closure / no extra opinion added.
    EpspSS e1(2, "(0, 1, 3)");            // p=2, q=2<=p -> case 3 (+q)
    std::cout << "eps_p(2) (0,1,3)         = " << e1.to_string() << "\n";
    EpspSS e2(2, "(0, 1, 3)");
    std::cout << "eps_p(2) (0,1,3) exp(1)  = " << e2.expand(1).to_string() << "\n";
    EpspSS e3(2, "(0, 1, 3)");
    std::cout << "eps_p(2) (0,1,3) exp(2)  = " << e3.expand(2).to_string() << "\n";
    EpspSS e4(5, "(0, 1, 3)");            // p=5, q=2<=p -> case 3 (+q)
    std::cout << "eps_p(5) (0,1,3) exp(2)  = " << e4.expand(2).to_string() << "\n";
    EpspSS e5(2, "(0, 1, 2)");            // p=2, q=1 -> case 2 (no add)
    std::cout << "eps_p(2) (0,1,2) exp(2)  = " << e5.expand(2).to_string() << "\n";

    // ε_ωSS demos — no parameter p and no case 4 (q added in full).
    EpsOmegaSS w1("(0, 1, 3)");            // q=2 -> case 3 (+q, unbounded)
    std::cout << "eps_omega (0,1,3) exp(2) = " << w1.expand(2).to_string() << "\n";
    EpsOmegaSS w2("(0, 1, 2)");            // q=1 -> case 2 (no add)
    std::cout << "eps_omega (0,1,2) exp(2) = " << w2.expand(2).to_string() << "\n";
    EpsOmegaSS w3("(0, 1, 3)");
    std::cout << "eps_omega (0,1,3) exp_to1 = " << w3.expand_to(1).to_string() << "\n";

    // print() / operator<< both emit LaTeX (a fresh object, since expand/reduce mutate)
    Knuth k3("2 ^^ 3");
    std::cout << "operator<< : "; std::cout << k3 << "\n";
    std::cout << "print()    : "; k3.print(); std::cout << "\n";

    // Comparison is undefined for large-number notations.
    try {
        k1.compare(k2);
    } catch (const NotComparable& e) {
        std::cout << "compare(knuth,knuth): " << e.what() << "\n";
    }

    return 0;
}
