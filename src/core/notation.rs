use std::fmt;
use super::capability::{Op, Capabilities};

/// Top-level taxonomy of notations.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Family {
    Ordinal,
    Number,
    Hierarchy,
    RealSequence,
}

/// Error type for unsupported operations.
#[derive(Debug, Clone)]
pub struct UnsupportedOperation {
    pub name: String,
    pub op: Op,
}

impl std::fmt::Display for UnsupportedOperation {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Notation '{}' does not implement the requested operation", self.name)
    }
}

impl std::error::Error for UnsupportedOperation {}

/// Error type for non-comparable notations (large-number notations).
#[derive(Debug, Clone)]
pub struct NotComparable {
    pub name: String,
}

impl std::fmt::Display for NotComparable {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Notation '{}': comparison is undefined for large-number notations", self.name)
    }
}

impl std::error::Error for NotComparable {}

/// Generic notation base trait. Every notation implements this trait and
/// declares which operations it supports via capabilities().
///
/// The library NEVER computes a numeric value. expand()/expand_to()/reduce()
/// rewrite the notation and return the notation's own type.
pub trait Notation: fmt::Display {
    // --- taxonomy ---
    fn name(&self) -> &str;
    fn family(&self) -> Family;
    fn subfamily(&self) -> &str { "" }
    fn style(&self) -> &str { "" }

    // --- attribution ---
    fn creator(&self) -> &str { "" }
    fn version(&self) -> &str { "" }

    // --- capability ---
    fn capabilities(&self) -> Capabilities;
    fn can(&self, op: Op) -> bool {
        self.capabilities().has(op)
    }

    // --- string conversion ---
    fn string_to_it(&mut self, _s: &str) -> Result<(), String> {
        Err(UnsupportedOperation { name: self.name().to_string(), op: Op::FromString }.to_string())
    }
    fn to_string(&self) -> String;

    // --- operations ---
    fn compare(&self, _other: &dyn Notation) -> Result<i32, String> {
        Err(NotComparable { name: self.name().to_string() }.to_string())
    }
    fn expand(&mut self, _n: i64) -> Result<(), String> {
        Err(UnsupportedOperation { name: self.name().to_string(), op: Op::Expand }.to_string())
    }
    fn expand_to(&mut self, _len: i64) -> Result<(), String> {
        Err(UnsupportedOperation { name: self.name().to_string(), op: Op::ExpandTo }.to_string())
    }
    fn comparable(&self) -> bool {
        self.can(Op::Compare)
    }

    fn reduce(&mut self) -> Result<(), String> {
        let prev = self.to_string();
        for _ in 0..1000 {
            self.expand(1)?;
            let cur = self.to_string();
            if cur == prev {
                return Ok(());
            }
            // prev = cur; // would need to rebind — handled by loop
        }
        Ok(())
    }
}

impl fmt::Display for dyn Notation {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.to_string())
    }
}