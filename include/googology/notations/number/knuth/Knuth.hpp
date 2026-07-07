#pragma once
#include <memory>
#include <string>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace number {

// AST for Knuth up-arrow expressions.
//   Value(v)        = the integer v
//   Arrow(a,c,rhs)  = a ^^...^ rhs  (c up-arrows; c==1 means ordinary power a^rhs)
struct KExpr {
    enum class Kind { Value, Arrow } kind;
    BigInt a = 0;
    int c = 0;
    BigInt val = 0;
    std::shared_ptr<KExpr> rhs;

    static KExpr value(BigInt v) { KExpr e; e.kind = Kind::Value; e.val = v; return e; }
    static KExpr arrow(BigInt a_, int c_, KExpr r) {
        KExpr e;
        e.kind = Kind::Arrow;
        e.a = a_;
        e.c = c_;
        e.rhs = std::make_shared<KExpr>(std::move(r));
        return e;
    }
};

// Knuth up-arrow notation (高德纳箭头).
// Definition (per spec/notations/knuth.md):
//   a ^c b = a ^ b                      (c = 1)
//   a ^c b = a                          (b = 1)
//   a ^c b = a ^(c-1) (a ^c (b-1))      (c > 1, b > 1)
class Knuth : public Notation {
    std::shared_ptr<KExpr> root_;

    static BigInt evalExpr(const KExpr& e);
    static KExpr expandExpr(const KExpr& e);
    static std::string ser(const KExpr& e);

public:
    Knuth() = default;
    explicit Knuth(const std::string& s) { parse(s); }

    std::string name() const override { return "knuth"; }
    Family family() const override { return Family::Number; }

    // Supports Parse/Serialize/Expand/Evaluate. Compare is intentionally
    // NOT supported: large-number comparison is undefined.
    Capabilities capabilities() const override;

    void parse(const std::string& s) override;
    std::string serialize() const override;
    BigInt evaluate() const override;
    void expand(BigInt n) override;
};

} // namespace number
} // namespace googology
