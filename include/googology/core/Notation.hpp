#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "googology/config.hpp"
#include "googology/core/Capability.hpp"

namespace googology {

// Top-level taxonomy. Ordinal notations and large-number notations are
// fundamentally different objects (see doc/design.md):
//   * ordinal notations  -> represent ordinals; order (compare) is well-defined
//   * large-number notations -> represent integers/functions; comparison is
//     generally UNDEFINED, so compare() throws NotComparable.
//   * hierarchy notations -> bridge ordinal -> number (e.g. FGH/Hardy/SHG).
//   * real_sequence notations -> an ordinal-indexed REAL sequence (e.g. Ns /
//     n,m-Ns names x = α th NS, a real; it is NOT an ordinal notation and
//     NOT a large-number notation). Carved out of `ordinal` on 2026-07-16
//     per the user's directive.
enum class Family { Ordinal, Number, Hierarchy, RealSequence };

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
//
// The library NEVER computes a numeric value. "How to compute" is provided by
// expand() / expand_to() / reduce(), which rewrite the notation and return the
// notation's OWN object (so it can be serialized again or expanded further).
// Only the string functions to_string() / string_to_it() deal with strings:
// to_string() emits LaTeX, string_to_it() accepts ASCII or Unicode forms.
//
// Standard form (标准型) lives one level down, in OrdinalNotation (see
// core/OrdinalNotation.hpp): the *definition* is universal across all ordinal
// notations, and — for the ordinal SEQUENCE notations — the *algorithm* is
// identical too. Number notations (Knuth / Conway) derive straight from this
// class and never carry standard-form logic.
class Notation {
public:
    virtual ~Notation() = default;

    // --- taxonomy (hierarchical) ---
    virtual std::string name() const = 0;
    virtual Family family() const = 0;
    virtual std::string subfamily() const { return ""; }   // e.g. "sequence"
    virtual std::string style() const { return ""; }       // e.g. "difference" / "marked_parent"

    // --- attribution & version (optional metadata) ---
    // 创造者（命名者）/ creator: who introduced or named this notation.
    // 约定（记住 / convention）: the creator's name is part of the CLASS
    // name (prefixed), so each notation's class embeds its creator; the value
    // returned here is that name. Override in the notation's OWN class.
    // Default empty = unknown / to be annotated later.
    // 默认空串表示未知，待日后在"计划模程"中标明。
    virtual std::string creator() const { return ""; }
    // 版本号 / version: a STRING or a group of numeric codes
    // (e.g. "1", "2.3", "a17", "1.2.0"). Each notation / variant overrides
    // it in its own class. Default empty.
    // 可为字符串，也可是一串数字代码（默认空串）。
    virtual std::string version() const { return ""; }

    // --- capability ---
    virtual Capabilities capabilities() const = 0;
    bool can(Op op) const { return capabilities().has(op); }

    // --- string conversion (the ONLY string-touching operations) ---
    virtual void string_to_it(const std::string&) { throw UnsupportedOperation(name(), Op::FromString); }
    virtual std::string to_string() const { throw UnsupportedOperation(name(), Op::ToString); }

    // --- operations: these return the notation's OWN type (so it can be
    //     further expanded / serialized), NEVER a string. Only to_string()
    //     produces a string. ---
    virtual int compare(const Notation&) const { throw NotComparable(name()); }
    virtual Notation& expand(BigInt /*n*/) { throw UnsupportedOperation(name(), Op::Expand); }
    virtual Notation& expand_to(BigInt /*len*/) { throw UnsupportedOperation(name(), Op::ExpandTo); }

    // Large-number notations return false: comparison is generally undefined.
    virtual bool comparable() const { return can(Op::Compare); }

    // Print the LaTeX form to a stream (defaults to std::cout). Returns the stream.
    std::ostream& print(std::ostream& os = std::cout) const { return os << to_string(); }

    // Fully reduce (symbolically) by applying expand(1) until the form is
    // stable. Returns *this (the rewritten notation object), never a string.
    Notation& reduce() {
        std::string prev = to_string();
        for (BigInt k = 1; k <= 1000; ++k) {
            expand(1);
            std::string cur = to_string();
            if (cur == prev) return *this;
            prev = cur;
        }
        return *this;
    }

    // Stream output (LaTeX via to_string()).
    friend std::ostream& operator<<(std::ostream& os, const Notation& n) {
        return os << n.to_string();
    }
};

} // namespace googology
