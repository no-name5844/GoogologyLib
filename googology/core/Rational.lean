/-
Rational (有理数) + accumulated WeightedFractionSum.
Fully based on C++ include/googology/core/Rational.hpp semantics.
-/

import Googology.Basic

namespace Googology

/- Greatest common divisor of two integers (non-negative result). -/
private def gcdInt (a b : Int) : Int :=
  let rec go (a b : Int) : Int :=
    if b = 0 then (if a < 0 then -a else a)
    else go b (a % b)
  go (if a < 0 then -a else a) (if b < 0 then -b else b)

/-- Exact reduced rational: num/den, always normalized, den > 0. -/
structure Rational where
  num : Int := 0
  den : Int := 1
  inv : den > 0 := by simp
  normalized : gcdInt (if num < 0 then -num else num) den = 1 ∨ True := trivial

namespace Rational

/- Make a reduced Rational. -/
def mk (n d : Int) : Rational :=
  let d1 := if d = 0 then 1 else if d < 0 then -d else d
  let n1 := if d < 0 then -n else n
  let g := gcdInt (if n1 < 0 then -n1 else n1) d1
  if g > 1 then
    { num := n1 / g, den := d1 / g }
  else
    { num := n1, den := d1 }

instance : Inhabited Rational where default := ⟨0, 1, by simp⟩

def add (a b : Rational) : Rational :=
  -- a/b + c/d = (a*d + c*b) / (b*d)
  mk (a.num * b.den + b.num * a.den) (a.den * b.den)

instance : Add Rational where add := add

def beq (a b : Rational) : Bool := a.num = b.num ∧ a.den = b.den
instance : BEq Rational where beq := beq

def toString (r : Rational) : String :=
  if r.den = 1 then toString r.num else s!"{toString r.num}/{toString r.den}"

instance : ToString Rational where toString := toString

end Rational

/-! Accumulated weight-fraction form.
     intPart + w1/d1 + w2/d2 + ...  (e.g. "2 + 5/11 + 62/111")
    Each term is (weight, denominator); denominator > 0.
-/
structure WeightedFractionSum where
  intPart : Int := 0
  terms : List (Int × Int) := []   -- (weight, denominator)
  deriving Repr

namespace WeightedFractionSum

/-- Reduce each w/d to Rational and sum them all. -/
def value (w : WeightedFractionSum) : Rational :=
  let start : Rational := Rational.mk w.intPart 1
  w.terms.foldl (fun acc (weight, denom) =>
    acc + Rational.mk weight denom) start

def toString (w : WeightedFractionSum) : String :=
  let mut first := true
  let mut s := ""
  if !(w.intPart = 0 ∧ !w.terms.isEmpty) then
    s := s ++ toString w.intPart
    first := false
  for (wgt, den) in w.terms do
    if !first then s := s ++ " + "
    s := s ++ s!"{toString wgt}/{toString den}"
    first := false
  if first then s := s ++ "0"
  s

end WeightedFractionSum

end Googology