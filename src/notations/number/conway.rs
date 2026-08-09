// Conway chained-arrow notation (康威链式箭头).
// Rules:
//   1. [a -> b]                = a ^ b
//   2. X -> 1 -> Y             = X
//   3. X -> a -> b             = X -> (X -> a-1 -> b) -> b-1
//
// Output is LaTeX (e.g. `3 \rightarrow 3 \rightarrow 2`). The library never
// computes a numeric value: expand()/expand_to() rewrite the internal chain
// and return self; to_string() turns it into LaTeX on demand.

use crate::core::capability::{Op, Capabilities};
use crate::core::notation::{Family, Notation};

/// A chain element: either an integer leaf, or a nested sub-chain.
#[derive(Clone, Debug)]
enum CNode {
    Value(i64),
    SubChain(Vec<CNode>),
}

impl CNode {
    fn value(v: i64) -> Self { CNode::Value(v) }
}

/// Conway chained-arrow notation.
pub struct Conway {
    chain: Vec<CNode>,
}

impl Conway {
    pub fn new() -> Self {
        Conway { chain: vec![] }
    }

    pub fn from_string(s: &str) -> Result<Self, String> {
        let mut c = Conway::new();
        c.string_to_it(s)?;
        Ok(c)
    }

    fn ser(ch: &[CNode]) -> String {
        if ch.len() == 2 {
            if let (CNode::Value(a), CNode::Value(b)) = (&ch[0], &ch[1]) {
                return format!("{}^{{{}}}", a, b);
            }
        }
        let parts: Vec<String> = ch.iter().map(|node| match node {
            CNode::Value(v) => v.to_string(),
            CNode::SubChain(s) => format!("({})", Self::ser(s)),
        }).collect();
        parts.join(" \\rightarrow ")
    }

    fn step_once(ch: &[CNode]) -> Vec<CNode> {
        let n = ch.len();
        if n <= 2 {
            if n <= 1 { return ch.to_vec(); }
            if let (CNode::Value(_), CNode::Value(_)) = (&ch[0], &ch[1]) {
                return ch.to_vec(); // terminal: a -> b
            }
            return Self::recurse_sub(ch);
        }

        // rule 2a: trailing 1  (X -> a -> 1  =  X -> a)
        if let CNode::Value(v) = &ch[n-1] {
            if *v == 1 {
                return ch[..n-1].to_vec();
            }
        }

        // rule 2b: middle 1  (X -> 1 -> Y  =  X)
        if let CNode::Value(v) = &ch[n-2] {
            if *v == 1 {
                return ch[..n-2].to_vec();
            }
        }

        // rule 3: X -> a -> b  =  X -> (X -> a-1 -> b) -> b-1
        if let (CNode::Value(a), CNode::Value(b)) = (&ch[n-2], &ch[n-1]) {
            let x = &ch[..n-2];
            let mut inner = x.to_vec();
            inner.push(CNode::Value(*a - 1));
            inner.push(CNode::Value(*b));
            let mut result = x.to_vec();
            result.push(CNode::SubChain(inner));
            result.push(CNode::Value(*b - 1));
            return result;
        }

        // no top-level rule: step any nested sub-chain
        Self::recurse_sub(ch)
    }

    fn recurse_sub(ch: &[CNode]) -> Vec<CNode> {
        let mut r = ch.to_vec();
        let mut changed = false;
        for node in r.iter_mut() {
            if let CNode::SubChain(sub) = node {
                let before = Self::ser(sub);
                let s = Self::step_once(sub);
                let after = Self::ser(&s);
                if after != before {
                    if s.len() == 1 {
                        if let CNode::Value(v) = s[0] {
                            *node = CNode::Value(v);
                        }
                    } else {
                        *node = CNode::SubChain(s);
                    }
                    changed = true;
                }
            }
        }
        if changed { r } else { ch.to_vec() }
    }
}

impl Notation for Conway {
    fn name(&self) -> &str { "conway" }
    fn family(&self) -> Family { Family::Number }
    fn creator(&self) -> &str { "John Conway" }
    fn version(&self) -> &str { "1" }

    fn capabilities(&self) -> Capabilities {
        let mut c = Capabilities::new();
        c.set(Op::FromString);
        c.set(Op::ToString);
        c.set(Op::Expand);
        c.set(Op::ExpandTo);
        c
    }

    fn string_to_it(&mut self, s: &str) -> Result<(), String> {
        let t = s.replace("→", "->").replace(' ', "");
        self.chain = t.split("->")
            .map(|part| part.parse::<i64>().map(CNode::Value))
            .collect::<Result<Vec<_>, _>>()
            .map_err(|e| format!("Conway: {}", e))?;
        Ok(())
    }

    fn to_string(&self) -> String {
        Self::ser(&self.chain)
    }

    fn expand(&mut self, n: i64) -> Result<(), String> {
        for _ in 0..n {
            self.chain = Self::step_once(&self.chain);
        }
        Ok(())
    }

    fn expand_to(&mut self, len: i64) -> Result<(), String> {
        let prev = self.to_string();
        loop {
            self.expand(1)?;
            let cur = self.to_string();
            if cur.len() >= len as usize || cur == prev {
                return Ok(());
            }
        }
    }
}

impl std::fmt::Display for Conway {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_and_string() {
        let c = Conway::from_string("3 -> 3 -> 2").unwrap();
        let s = c.to_string();
        println!("  Conway '3 -> 3 -> 2' -> {s}");
        assert!(s.contains("rightarrow"));
    }

    #[test]
    fn test_expand_once() {
        let mut c = Conway::from_string("3 -> 3 -> 2").unwrap();
        c.expand(1).unwrap();
        let s = c.to_string();
        println!("  Conway '3 -> 3 -> 2' expand(1) -> {s}");
    }

    #[test]
    fn test_trailing_one() {
        let mut c = Conway::from_string("3 -> 4 -> 1").unwrap();
        c.expand(1).unwrap();
        let s = c.to_string();
        println!("  Conway '3 -> 4 -> 1' expand(1) -> {s}");
        assert!(s.contains("4"));
    }

    #[test]
    fn test_capabilities() {
        let c = Conway::from_string("3 -> 3 -> 2").unwrap();
        assert!(c.can(Op::FromString));
        assert!(c.can(Op::ToString));
        assert!(c.can(Op::Expand));
        assert!(c.can(Op::ExpandTo));
        assert!(!c.can(Op::Compare));
    }
}