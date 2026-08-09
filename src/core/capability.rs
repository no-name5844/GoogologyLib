/// Operations a notation may support.
/// This is the capability model: every notation declares which operations
/// it implements. Operations not declared throw UnsupportedOperation.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Op {
    FromString = 0,
    ToString = 1,
    Normalize = 2,
    Compare = 3,
    Expand = 4,
    ExpandTo = 5,
    Successor = 6,
}

/// Bitset of supported operations.
#[derive(Clone, Debug, Default)]
pub struct Capabilities(u8);

impl Capabilities {
    pub fn new() -> Self {
        Self(0)
    }

    pub fn set(&mut self, op: Op) {
        self.0 |= 1u8 << (op as u8);
    }

    pub fn has(&self, op: Op) -> bool {
        (self.0 & (1u8 << (op as u8))) != 0
    }
}