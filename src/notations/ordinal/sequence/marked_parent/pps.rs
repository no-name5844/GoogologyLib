//! PPS (Parented Predecessor Sequence) — marked-parent ordinal sequence
//! notation. Created by 318`4. Variants: PPS1 (original) / PPS2 / PPS4 /
//! wPPS4 / tPPS4 / fPPS4. PPS3 / ePPS4 / sPPS4 are N/A (no data) — not
//! implemented.
//!
//! Bad root = the x-th term (x = last value); b = bad-root value; L = y - x
//! (y = last column). expand_len replaces the last term (per variant) and
//! appends elements from the running sequence: a_i >= x -> a_i + L, else a_i.
//! 基本列[n] = expand to the (y + nL - 1)-th term (n >= 1). Library 0-index:
//! expand(k) = 基本列[k+1] = expand_len((k+1)L - 1). compare is lexicographic;
//! cross-variant returns Err(NotComparable).

use std::any::Any;

use crate::core::capability::{Capabilities, Op};
use crate::core::notation::{Family, Notation, NotComparable};

/// PPS variants.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PpsVariant {
    Pps1,
    Pps2,
    Pps4,
    WPps4,
    TPps4,
    FPps4,
}

impl PpsVariant {
    fn reg_name(self) -> &'static str {
        match self {
            PpsVariant::Pps1 => "pps1",
            PpsVariant::Pps2 => "pps2",
            PpsVariant::Pps4 => "pps4",
            PpsVariant::WPps4 => "wpps4",
            PpsVariant::TPps4 => "tpps4",
            PpsVariant::FPps4 => "fpps4",
        }
    }

    fn ver(self) -> &'static str {
        match self {
            PpsVariant::Pps1 => "1",
            PpsVariant::Pps2 => "2",
            PpsVariant::Pps4 => "4",
            PpsVariant::WPps4 => "4w",
            PpsVariant::TPps4 => "4t",
            PpsVariant::FPps4 => "4f",
        }
    }
}

/// PPS state: variant + sequence terms + master-limit flag.
#[derive(Clone, Debug)]
pub struct Pps {
    variant: PpsVariant,
    seq: Vec<i64>,
    is_master_limit: bool,
}

impl Pps {
    pub fn new(v: PpsVariant) -> Self {
        Pps {
            variant: v,
            seq: Vec::new(),
            is_master_limit: false,
        }
    }

    pub fn from_string(v: PpsVariant, s: &str) -> Result<Self, String> {
        let mut p = Pps::new(v);
        p.string_to_it(s)?;
        Ok(p)
    }

    /// limit(n) = (0,1,...,n-1) of a variant.
    pub fn limit(v: PpsVariant, n: i64) -> Self {
        let mut p = Pps::new(v);
        p.seq = (0..n).collect();
        p
    }

    /// Master limit expression of a variant (supremum).
    pub fn master_limit(v: PpsVariant) -> Self {
        let mut p = Pps::new(v);
        p.is_master_limit = true;
        p
    }

    /// Sequence ends with 0 => successor.
    pub fn is_successor(&self) -> bool {
        !self.is_master_limit && self.seq.last() == Some(&0)
    }

    pub fn to_latex(&self) -> String {
        if self.is_master_limit {
            return "(0, 1, 2, …)".to_string();
        }
        let parts: Vec<String> = self.seq.iter().map(|v| v.to_string()).collect();
        format!("({})", parts.join(", "))
    }

    /// Rightmost 1-based column j in the open interval (lo, hi) with value = b.
    fn rightmost_eq(&self, lo: i64, hi: i64, b: i64) -> i64 {
        let mut j = hi - 1;
        while j > lo {
            if j >= 1 && (j as usize) <= self.seq.len() && self.seq[(j - 1) as usize] == b {
                return j;
            }
            j -= 1;
        }
        0
    }

    /// Lexicographic compare of a_{k+j} vs a_{c+j} (j = 0,1,2,...).
    fn tail_cmp(&self, k: i64, c: i64) -> i32 {
        let y = self.seq.len() as i64;
        let mut j = 0i64;
        loop {
            let ik = k + j;
            let ic = c + j;
            let ok_k = ik >= 1 && ik <= y;
            let ok_c = ic >= 1 && ic <= y;
            if !ok_k && !ok_c {
                return 0;
            }
            if !ok_k {
                return -1;
            }
            if !ok_c {
                return 1;
            }
            let vk = self.seq[(ik - 1) as usize];
            let vc = self.seq[(ic - 1) as usize];
            if vk != vc {
                return if vk > vc { 1 } else { -1 };
            }
            j += 1;
        }
    }

    /// Per-variant last-term replacement: (new_last, strong).
    fn replace_last(&self, x: i64, b: i64, y: i64) -> (i64, bool) {
        match self.variant {
            PpsVariant::Pps1 => {
                let k = self.rightmost_eq(x, y, b);
                (if k != 0 { b } else { x - 1 }, false)
            }
            PpsVariant::Pps2 => {
                let k = self.rightmost_eq(x, y, b);
                (if k != 0 && self.tail_cmp(k, x) > 0 { b } else { x - 1 }, false)
            }
            _ => {
                let k = self.rightmost_eq(x, y, b);
                if k != 0 {
                    return (b, false); // weak expansion
                }
                let j = self.rightmost_eq(b, x, b); // strong expansion
                (if j != 0 { j } else { b }, true) // not found -> as weak
            }
        }
    }

    /// Copy rule; tPPS4/fPPS4 strong-branch modifiers on the last-term chain.
    fn copied(&self, src: i64, x: i64, l: i64, y: i64, i: i64, strong: bool) -> i64 {
        let base = if src >= x { src + l } else { src };
        if !strong {
            return base;
        }
        if matches!(self.variant, PpsVariant::TPps4 | PpsVariant::FPps4) {
            if i % l == y % l {
                if self.variant == PpsVariant::FPps4 && i == y {
                    return x;
                }
                return src + l;
            }
        }
        base
    }

    /// expand_len(A, M): replace last + append M elements (source i = q + y - L).
    fn expand_len(&mut self, m: i64) -> Result<(), String> {
        if self.seq.is_empty() {
            return Ok(());
        }
        let y = self.seq.len() as i64;
        let x = *self.seq.last().unwrap();
        if x == 0 {
            self.seq.pop();
            return Ok(());
        }
        if x >= y {
            return Err("Pps: bad root out of range (x >= y)".to_string());
        }
        let b = self.seq[(x - 1) as usize];
        let l = y - x;
        let (new_last, strong) = self.replace_last(x, b, y);
        self.seq[(y - 1) as usize] = new_last;
        for q in 1..=m {
            let i = q + y - l;
            if i < 1 || (i as usize) > self.seq.len() {
                return Ok(()); // defensive
            }
            let src = self.seq[(i - 1) as usize];
            self.seq.push(self.copied(src, x, l, y, i, strong));
        }
        Ok(())
    }
}

impl Notation for Pps {
    fn as_any(&self) -> &dyn Any {
        self
    }

    fn name(&self) -> &str {
        self.variant.reg_name()
    }

    fn family(&self) -> Family {
        Family::Ordinal
    }

    fn subfamily(&self) -> &str {
        "sequence"
    }

    fn style(&self) -> &str {
        "marked_parent"
    }

    fn creator(&self) -> &str {
        "318`4"
    }

    fn version(&self) -> &str {
        self.variant.ver()
    }

    fn capabilities(&self) -> Capabilities {
        let mut c = Capabilities::new();
        c.set(Op::FromString);
        c.set(Op::ToString);
        c.set(Op::Normalize);
        c.set(Op::Compare);
        c.set(Op::Expand);
        c.set(Op::ExpandTo);
        c.set(Op::Successor);
        c
    }

    fn string_to_it(&mut self, s: &str) -> Result<(), String> {
        self.seq.clear();
        self.is_master_limit = false;
        let t: String = s.chars().filter(|c| !c.is_whitespace()).collect();
        let t = t
            .trim_start_matches(['(', '[', '{'])
            .trim_end_matches([')', ']', '}']);
        if t.is_empty() {
            return Ok(());
        }
        for p in t.split(',') {
            if !p.is_empty() {
                self.seq
                    .push(p.parse::<i64>().map_err(|e| format!("Pps: {}", e))?);
            }
        }
        Ok(())
    }

    fn to_string(&self) -> String {
        self.to_latex()
    }

    /// Library 0-index: expand(k) = 基本列[k+1] = expand_len((k+1)L - 1).
    fn expand(&mut self, n: i64) -> Result<(), String> {
        if self.is_master_limit {
            self.seq = (0..n).collect();
            self.is_master_limit = false;
            return Ok(());
        }
        if self.seq.is_empty() {
            return Ok(());
        }
        let x = *self.seq.last().unwrap();
        if x == 0 {
            self.seq.pop();
            return Ok(());
        }
        let y = self.seq.len() as i64;
        if x >= y {
            return Err("Pps: bad root out of range (x >= y)".to_string());
        }
        let l = y - x;
        self.expand_len((n + 1) * l - 1)
    }

    fn expand_to(&mut self, len: i64) -> Result<(), String> {
        self.expand_len(len)
    }

    fn compare(&self, other: &dyn Notation) -> Result<i32, String> {
        let o = other
            .as_any()
            .downcast_ref::<Pps>()
            .ok_or_else(|| NotComparable { name: self.name().to_string() }.to_string())?;
        if self.variant != o.variant {
            return Err(NotComparable { name: self.name().to_string() }.to_string());
        }
        if self.is_master_limit || o.is_master_limit {
            return Ok(match (self.is_master_limit, o.is_master_limit) {
                (true, true) => 0,
                (true, false) => 1,
                (false, true) => -1,
                (false, false) => 0,
            });
        }
        let m = self.seq.len().min(o.seq.len());
        for i in 0..m {
            if self.seq[i] > o.seq[i] {
                return Ok(1);
            }
            if self.seq[i] < o.seq[i] {
                return Ok(-1);
            }
        }
        Ok(match self.seq.len().cmp(&o.seq.len()) {
            std::cmp::Ordering::Greater => 1,
            std::cmp::Ordering::Less => -1,
            std::cmp::Ordering::Equal => 0,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn e(v: PpsVariant, s: &str) -> Pps {
        Pps::from_string(v, s).unwrap()
    }

    #[test]
    fn pps1_parse_and_expand() {
        assert_eq!(e(PpsVariant::Pps1, "(0,1,2)").to_string(), "(0, 1, 2)");
        let mut p = e(PpsVariant::Pps1, "(0,1,2)");
        p.expand(0).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 1)");
        let mut p = e(PpsVariant::Pps1, "(0,1,2)");
        p.expand(2).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 1, 1, 1)");
    }

    #[test]
    fn pps1_no_term_between() {
        let mut p = e(PpsVariant::Pps1, "(0,1,0,3)");
        p.expand(0).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 0, 2)");
    }

    #[test]
    fn pps1_term_equal_b() {
        let mut p = e(PpsVariant::Pps1, "(0,1,0,1,2)");
        p.expand(0).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 0, 1, 1, 0, 1)");
    }

    #[test]
    fn successor() {
        let mut p = e(PpsVariant::Pps1, "(0,1,0)");
        p.expand(3).unwrap();
        assert_eq!(p.to_string(), "(0, 1)");
        assert!(e(PpsVariant::Pps1, "(0,1,0)").is_successor());
        assert!(!e(PpsVariant::Pps1, "(0,1,2)").is_successor());
    }

    #[test]
    fn expand_to() {
        let mut p = e(PpsVariant::Pps1, "(0,1,2)");
        p.expand_to(2).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 1, 1, 1)");
    }

    #[test]
    fn pps1_vs_pps2() {
        let mut p1 = e(PpsVariant::Pps1, "(0,2,4,4,2,3)");
        p1.expand(0).unwrap();
        assert_eq!(p1.to_string(), "(0, 2, 4, 4, 2, 4, 7, 2)");
        let mut p2 = e(PpsVariant::Pps2, "(0,2,4,4,2,3)");
        p2.expand(0).unwrap();
        assert_eq!(p2.to_string(), "(0, 2, 4, 4, 2, 2, 7, 2)");
    }

    #[test]
    fn pps4_weak_strong() {
        let mut p = e(PpsVariant::Pps4, "(0,1,0,2,2,3)");
        p.expand(0).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 0, 2, 2, 1, 2, 2)");
        let mut p = e(PpsVariant::Pps4, "(0,1,2,1,3)");
        p.expand(0).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 2, 1, 2, 1)");
    }

    #[test]
    fn tpps4_fpps4() {
        let mut p = e(PpsVariant::TPps4, "(0,1,0,2,2,3)");
        p.expand(1).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 0, 2, 2, 1, 2, 2, 4, 2, 2)");
        let mut p = e(PpsVariant::FPps4, "(0,1,0,2,2,3)");
        p.expand(1).unwrap();
        assert_eq!(p.to_string(), "(0, 1, 0, 2, 2, 1, 2, 2, 3, 2, 2)");
    }

    #[test]
    fn limit_and_master() {
        assert_eq!(Pps::limit(PpsVariant::Pps1, 3).to_string(), "(0, 1, 2)");
        let mut m = Pps::master_limit(PpsVariant::Pps1);
        m.expand(3).unwrap();
        assert_eq!(m.to_string(), "(0, 1, 2)");
    }

    #[test]
    fn compare_lexicographic() {
        let a = e(PpsVariant::Pps1, "(0,1,2)");
        let b = e(PpsVariant::Pps1, "(0,1,3)");
        assert_eq!(a.compare(&b).unwrap(), -1);
        assert_eq!(b.compare(&a).unwrap(), 1);
        assert_eq!(a.compare(&a).unwrap(), 0);
        let c = e(PpsVariant::Pps1, "(0,1,2,0)");
        assert_eq!(a.compare(&c).unwrap(), -1);
    }

    #[test]
    fn cross_variant_not_comparable() {
        let a = e(PpsVariant::Pps1, "(0,1,2)");
        let b = e(PpsVariant::Pps2, "(0,1,2)");
        assert!(a.compare(&b).is_err());
    }

    #[test]
    fn taxonomy() {
        assert_eq!(e(PpsVariant::Pps1, "(0,1,2)").name(), "pps1");
        assert_eq!(e(PpsVariant::TPps4, "(0,1,2)").name(), "tpps4");
        assert_eq!(e(PpsVariant::Pps1, "(0,1,2)").style(), "marked_parent");
        assert_eq!(e(PpsVariant::Pps1, "(0,1,2)").creator(), "318`4");
        assert_eq!(e(PpsVariant::TPps4, "(0,1,2)").version(), "4t");
    }
}
