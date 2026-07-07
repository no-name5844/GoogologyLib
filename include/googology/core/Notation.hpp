#pragma once
#include <string>
#include <stdexcept>
#include "googology/config.hpp"
#include "googology/core/Capability.hpp"

namespace googology {

// Top-level taxonomy. Ordinal notations and large-number notations are
// fundamentally different objects (see doc/design.md):
//   * ordinal notations  -> represent ordinals; order (compare) is well-defined
//   * large-number notations -> represent integers/functions; comparison is
//     generally UNDEFINED, so compare() throws NotComparable.
//   * hierarchy notations -> bridge ordinal -> number (e.g. FGH/Hardy/SHG).
enum class Family { Ordinal, Number, Hierarchy };

struct UnsupportedOperation : std::runtime_error {
    UnsupportedOperation(const std::string& n, Op op)
        : std::runtime_error("Notation '" + n + "' does not implement the requested operation") {
        (void)op;
    }
};

struct NotComparable : std::runtime_error {
    explicit NotComparable(const std::string& n)
        : std::runtime_error("Notation '" + n +
                             "': comparison is undefined for large-number notations") {}
};

// Generic notation base. Every notation (ordinal or large-number) derives
// from this and declares which operations it supports via capabilities().
// Operations not supported throw UnsupportedOperation (or NotComparable for
// comparison on large-number notations).
class Notation {
public:
    virtual ~Notation() = default;

    // --- taxonomy (hierarchical) ---
    virtual std::string name() const = 0;
    virtual Family family() const = 0;
    virtual std::string subfamily() const { return ""; }   // e.g. "sequence"
    virtual std::string style() const { return ""; }       // e.g. "difference" / "marked_parent"

    // --- capability ---
    virtual Capabilities capabilities() const = 0;
    bool can(Op op) const { return capabilities().has(op); }

    // --- operations (defaults: unsupported) ---
    virtual void parse(const std::string&) { throw UnsupportedOperation(name(), Op::Parse); }
    virtual std::string serialize() const { throw UnsupportedOperation(name(), Op::Serialize); }
    virtual void normalize() { throw UnsupportedOperation(name(), Op::Normalize); }
    virtual int compare(const Notation&) const { throw NotComparable(name()); }
    virtual void expand(BigInt /*n*/) { throw UnsupportedOperation(name(), Op::Expand); }
    virtual void expand_to(BigInt /*len*/) { throw UnsupportedOperation(name(), Op::ExpandTo); }
    virtual BigInt evaluate() const { throw UnsupportedOperation(name(), Op::Evaluate); }
    virtual bool isSuccessor() const { throw UnsupportedOperation(name(), Op::Successor); }

    // Large-number notations return false: comparison is generally undefined.
    virtual bool comparable() const { return can(Op::Compare); }
};

} // namespace googology
