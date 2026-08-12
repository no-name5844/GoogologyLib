#include <iostream>
#include <iomanip>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"
#include "googology/core/Registry.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/notations/number/conway/Conway.hpp"
#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include "googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp"
#include "googology/notations/ordinal/sequence/marked_parent/pps/PPS.hpp"
#include "googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp"
#include "googology/notations/real_sequence/ns/Ns.hpp"

using namespace googology;
using namespace googology::number;
using namespace googology::ordinal;
using namespace googology::real_sequence;

static const char* familyName(Family f) {
    switch (f) {
        case Family::Ordinal: return "ordinal";
        case Family::Number: return "number";
        case Family::Hierarchy: return "hierarchy";
        case Family::RealSequence: return "real_sequence";
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
    registry().add("pps1", [] { return std::make_unique<Pps1>(); });
    registry().add("pps2", [] { return std::make_unique<Pps2>(); });
    registry().add("pps4", [] { return std::make_unique<Pps4>(); });
    registry().add("wpps4", [] { return std::make_unique<WPps4>(); });
    registry().add("tpps4", [] { return std::make_unique<TPps4>(); });
    registry().add("fpps4", [] { return std::make_unique<FPps4>(); });
    registry().add("weak_veblen", [] { return std::make_unique<WeakVeblen>(); });
    registry().add("ns", [] { return std::make_unique<Ns>(); });

    printCapabilityMatrix();
    std::cout << "\n";

    // creator() / version() metadata API (see doc/design.md §3c).
    WeakVeblen wv_meta;
    std::cout << "wv    creator = " << wv_meta.creator()
              << "   version = " << wv_meta.version() << "\n";
    Knuth k_meta;
    std::cout << "knuth creator = " << k_meta.creator()
              << "   version = " << k_meta.version() << "\n";
    Conway c_meta;
    std::cout << "conway creator= " << c_meta.creator()
              << "   version = " << c_meta.version() << "\n";
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

    // weak-Veblen-like demos (zahin). Implemented EXACTLY per the article's
    // 6-case expand; the ordinal arithmetic (+1, ^, expand(b,n)) is supplied
    // by core/Ordinal.hpp (which follows study/notations/Ordinal.md). The
    // article §1.3 DOES supply a compare (is_equal/is_greater/compare),
    // so compare() is implemented (lexicographic, primary key = second @b
    // coord, secondary = first @a coord; longer wins on prefix). No normalize
    // / expand_to is defined by the article.
    WeakVeblen v1("(2@3, 1@0)");
    std::cout << "wv (2@3, 1@0)        = " << v1.to_string() << "\n";
    WeakVeblen v2("(2@3, 1@0)");
    std::cout << "wv (2@3, 1@0) exp(0) = " << v2.expand(0).to_string() << "\n";
    WeakVeblen v3("(2@3, 1@0)");
    std::cout << "wv (2@3, 1@0) exp(1) = " << v3.expand(1).to_string() << "\n";
    WeakVeblen v4("(2@3, 1@1)");
    std::cout << "wv (2@3, 1@1) exp(0) = " << v4.expand(0).to_string() << "\n";
    WeakVeblen v5("(2@3, 1@ω)");
    std::cout << "wv (2@3, 1@ω) exp(3) = " << v5.expand(3).to_string() << "\n";
    WeakVeblen v6("(2@4, ω@3)");
    std::cout << "wv (2@4, ω@3) exp(3) = " << v6.expand(3).to_string() << "\n";
    // A[n] convenience (non-mutating): A[1] == expand(A,1)
    WeakVeblen v7("(2@3, 1@0)");
    std::cout << "wv (2@3, 1@0)[1]     = " << v7[1].to_string() << "\n";
    // compare (article §1.3): primary key = second coord @b, secondary = @a;
    // longer wins on prefix.
    WeakVeblen ca("(1@0)"), cb("(0@0)");
    std::cout << "wv compare((1@0),(0@0)) = " << ca.compare(cb) << "\n";   // +1 (a:1>0)
    WeakVeblen cc("(0@1)"), cd("(0@0)");
    std::cout << "wv compare((0@1),(0@0)) = " << cc.compare(cd) << "\n";   // +1 (i:1>0)
    WeakVeblen ce("(0@5)"), cf("(100@0)");
    std::cout << "wv compare((0@5),(100@0)) = " << ce.compare(cf) << "\n"; // +1 (i dominates)
    WeakVeblen cg("(1@0, 1@0)"), ch("(1@0)");
    std::cout << "wv compare((1@0,1@0),(1@0)) = " << cg.compare(ch) << "\n"; // +1 (prefix)

    // Ns / n,m-Ns demos (by 送到本到 & nnc). The notation names the REAL
    // x = α th NS, an element of an ordinal-indexed real sequence. The
    // library keeps the expression symbolic (α th NS) and exposes the value
    // as an ACCUMULATED FRACTION
    // of the step terms 1/f(β), exact only for FINITE α. Limit indices
    // need a fundamental sequence and are NOT auto-computed.
    Ns ns1("1 th NS");
    std::cout << "ns  1 th NS        = " << ns1.to_string()
              << "   value = " << ns1.value().to_string() << "\n";
    Ns ns2("2 th NS");
    std::cout << "ns  2 th NS        = " << ns2.to_string()
              << "   value = " << ns2.value().to_string()
              << "   (" << ns2.to_fraction_string() << ")\n";
    Ns ns3("3 th NS");
    std::cout << "ns  3 th NS        = " << ns3.to_string()
              << "   value = " << ns3.value().to_string()
              << "   (" << ns3.to_fraction_string() << ")\n";
    // n,m-Ns with n=3, m=2: a_2 = 1/3, a_3 = 4/9
    Ns nm(3, 2, "3 th NS");
    std::cout << "n,m-Ns(3,2) 3 th NS = " << nm.to_string()
              << "   value = " << nm.value().to_string()
              << "   (" << nm.to_fraction_string() << ")\n";
    // expand delegates to core::Ordinal (the §11 pluggable FS); for a
    // limit index ω, expand(ω, k) = k th NS.
    Ns nsl("ω th NS");
    std::cout << "ns  ω th NS expand(2) = " << nsl.expand(2).to_string() << "\n";
    // limit index value is NOT auto-computed (matches "不能自动计算").
    try { (void)Ns("ω th NS").value(); }
    catch (const std::domain_error&) {
        std::cout << "ns  ω th NS value() -> (not auto-computed, by design)\n";
    }

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
