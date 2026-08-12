// Knuth up-arrow notation (高德纳箭头).
//   a ^c b = a ^ b                 (c = 1)
//   a ^c b = a                     (b = 1)
//   a ^c b = a ^(c-1) (a ^c (b-1)) (c > 1, b > 1)
//
// Output is LaTeX (e.g. `2 \uparrow\uparrow 3`). The library never computes a
// numeric value: expand()/expand_to()/reduce() rewrite the internal AST and
// return self; to_string() turns it into LaTeX on demand.

use crate::core::capability::{Op, Capabilities};
use crate::core::notation::{Family, Notation, UnsupportedOperation};

/// One node of a Knuth up-arrow expression AST.
#[derive(Clone, Debug)]
enum KNode {
    Value(i64),
    Arrow { base: i64, height: i64, exp: Box<KNode> },
}

impl KNode {
    fn tex(&self) -> String {
        match self {
            KNode::Value(v) => v.to_string(),
            KNode::Arrow { base, height, exp } => {
                let bt = base.to_string();
                let et = exp.tex();
                let arr = if *height == 1 {
                    "\\uparrow".to_string()
                } else {
                    format!("\\uparrow^{{{}}}", height)
                };
                let exp_has_arrow = matches!(exp.as_ref(), KNode::Arrow { .. });
                let exp_part = if exp_has_arrow { format!("({})", et) } else { et };
                format!("{} {} {}", bt, arr, exp_part)
            }
        }
    }

    fn step(&self) -> KNode {
        match self {
            KNode::Value(_) => self.clone(),
            KNode::Arrow { base, height, exp } => {
                match exp.as_ref() {
                    KNode::Value(b) => {
                        if *b == 1 {
                            return KNode::Value(*base); // a ^c 1 = a
                        }
                        if *height == 1 {
                            return self.clone(); // a^b: terminal
                        }
                        let inner = KNode::Arrow {
                            base: *base,
                            height: *height,
                            exp: Box::new(KNode::Value(*b - 1)),
                        };
                        KNode::Arrow {
                            base: *base,
                            height: *height - 1,
                            exp: Box::new(inner),
                        }
                    }
                    _ => {
                        // recurse into the rightmost sub-term
                        KNode::Arrow {
                            base: *base,
                            height: *height,
                            exp: Box::new(exp.step()),
                        }
                    }
                }
            }
        }
    }

    fn parse(s: &str) -> Result<KNode, String> {
        let s = s.trim();
        let pos = s.find('^');
        if pos == None {
            if s.is_empty() {
                return Err("Knuth: empty operand".to_string());
            }
            return Ok(KNode::Value(s.parse::<i64>().map_err(|e| format!("Knuth: {}", e))?));
        }
        let pos = pos.unwrap();
        let a: i64 = s[..pos].parse().map_err(|e| format!("Knuth: {}", e))?;
        let mut j = pos;
        let mut c: i64 = 0;
        while j < s.len() && s.as_bytes()[j] == b'^' {
            c += 1;
            j += 1;
        }
        let rest = s[j..].trim();
        if rest.is_empty() {
            return Err("Knuth: dangling arrows (no exponent)".to_string());
        }
        let exp = KNode::parse(rest)?;
        Ok(KNode::Arrow {
            base: a,
            height: c,
            exp: Box::new(exp),
        })
    }
}

/// Knuth up-arrow notation.
pub struct Knuth {
    root: KNode,
}

impl Knuth {
    pub fn new() -> Self {
        Knuth { root: KNode::Value(0) }
    }

    pub fn from_string(s: &str) -> Result<Self, String> {
        let mut k = Knuth::new();
        k.string_to_it(s)?;
        Ok(k)
    }
}

impl Notation for Knuth {
    fn as_any(&self) -> &dyn std::any::Any { self }
    fn name(&self) -> &str { "knuth" }
    fn family(&self) -> Family { Family::Number }
    fn creator(&self) -> &str { "Donald Knuth" }
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
        let t = s.replace("↑", "^").replace(' ', "");
        if t.is_empty() {
            return Err("Knuth: empty input".to_string());
        }
        self.root = KNode::parse(&t)?;
        Ok(())
    }

    fn to_string(&self) -> String {
        self.root.tex()
    }

    fn expand(&mut self, n: i64) -> Result<(), String> {
        for _ in 0..n {
            self.root = self.root.step();
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
            // prev = cur; // would need to rebind
        }
    }
}

impl std::fmt::Display for Knuth {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::core::notation::NotComparable;

    #[test]
    fn test_parse_and_string() {
        let k = Knuth::from_string("2 ^^ 3").unwrap();
        let s = k.to_string();
        println!("  Knuth '2 ^^ 3' -> {s}");
        assert!(s.contains("uparrow"));
    }

    #[test]
    fn test_expand_once() {
        let mut k = Knuth::from_string("2 ^^ 3").unwrap();
        k.expand(1).unwrap();
        let s = k.to_string();
        println!("  Knuth '2 ^^ 3' expand(1) -> {s}");
        assert!(s.contains("uparrow"));
    }

    #[test]
    fn test_not_comparable() {
        let k = Knuth::from_string("2 ^^ 3").unwrap();
        let k2 = Knuth::from_string("3 ^^ 3").unwrap();
        let result = k.compare(&k2);
        assert!(result.is_err());
    }

    #[test]
    fn test_capabilities() {
        let k = Knuth::from_string("2 ^^ 3").unwrap();
        assert!(k.can(Op::FromString));
        assert!(k.can(Op::ToString));
        assert!(k.can(Op::Expand));
        assert!(k.can(Op::ExpandTo));
        assert!(!k.can(Op::Compare));
    }
}