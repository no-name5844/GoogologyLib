#pragma once
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace number {

// Conway chained-arrow notation.
// A chain X = [x0 -> x1 -> ... -> xk] is stored as a vector of BigInt.
// Rules (per spec/notations/conway.md):
//   1. [a -> b]                = a ^ b
//   2. X -> 1 -> Y             = X
//   3. X -> a -> b             = X -> (X -> a-1 -> b) -> b-1
class Conway : public Notation {
    std::vector<BigInt> chain_;

    static BigInt evalChain(const std::vector<BigInt>& ch);
    static std::vector<BigInt> stepChain(const std::vector<BigInt>& ch);
    static std::string ser(const std::vector<BigInt>& ch);

public:
    Conway() = default;
    explicit Conway(const std::string& s) { parse(s); }

    std::string name() const override { return "conway"; }
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
