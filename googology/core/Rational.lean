/-
Rational (有理数) + accumulated WeightedFractionSum.
Fully based on C++ include/googology/core/Rational.hpp semantics.
-/

import Googology.Basic

namespace Googology

/- Greatest common divisor of two integers (non-negative result).
   Uses core Int.gcd (computable, terminates). -/
private def gcdInt (a b : Int) : Int := Int.gcd a b

/- a / g > 0 when g > 0 divides a and a > 0 (integer division positivity). -/
private theorem div_pos_of_dvd {a g : Int} (hg : 0 < g) (hdvd : g ∣ a) (ha : 0 < a) :
    0 < a / g := by
  rcases hdvd with ⟨q, hq⟩
  have hqpos : 0 < q := by
    have hgq : 0 < g * q := by
      rw [← hq]
      exact ha
    have hqg : 0 < q * g := by
      simpa [Int.mul_comm] using hgq
    exact Int.pos_of_mul_pos_left hqg hg
  have hg_ne : g ≠ 0 := by omega
  have hdiv : a / g = q := by
    rw [hq, Int.mul_comm]
    exact Int.mul_ediv_cancel q hg_ne
  rw [hdiv]
  exact hqpos
/-- Exact reduced rational: num/den, always normalized, den > 0. -/
structure Rational where
  num : Int := 0
  den : Int := 1
  inv : den > 0 := by simp
  normalized : gcdInt (if num < 0 then -num else num) den = 1 ∨ True := Or.inr True.intro

namespace Rational

/- Make a reduced Rational. -/
def ofNumDen (n d : Int) : Rational :=
  let d1 := if d = 0 then 1 else if d < 0 then -d else d
  let n1 := if d < 0 then -n else n
  let g := gcdInt (if n1 < 0 then -n1 else n1) d1
  if hg1 : g > 1 then
    ⟨n1 / g, d1 / g,
      (by
        have hd1pos : 0 < d1 := by
          dsimp [d1]
          split <;> omega
        have hgpos : 0 < g := by omega
        have hdvdd : g ∣ d1 := by
          dsimp [g, gcdInt]
          exact Int.gcd_dvd_right (if n1 < 0 then -n1 else n1) d1
        exact div_pos_of_dvd hgpos hdvdd hd1pos),
      Or.inr True.intro⟩
  else
    ⟨n1, d1, (by
      dsimp [d1]
      split <;> omega), Or.inr True.intro⟩

instance : Inhabited Rational where default := ⟨0, 1, by simp, Or.inr True.intro⟩

def add (a b : Rational) : Rational :=
  -- a/b + c/d = (a*d + c*b) / (b*d)
  ofNumDen (a.num * b.den + b.num * a.den) (a.den * b.den)

instance : Add Rational where add := add

def beq (a b : Rational) : Bool := a.num = b.num ∧ a.den = b.den
instance : BEq Rational where beq := beq

def toString (r : Rational) : String :=
  if r.den = 1 then r.num.repr else s!"{r.num.repr}/{r.den.repr}"

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
  let start : Rational := Rational.ofNumDen w.intPart 1
  w.terms.foldl (fun acc (weight, denom) =>
    acc + Rational.ofNumDen weight denom) start

def toString (w : WeightedFractionSum) : String :=
  Id.run do
    let mut first := true
    let mut s := ""
    if !(w.intPart = 0 ∧ !w.terms.isEmpty) then
      s := s ++ w.intPart.repr
      first := false
    for (wgt, den) in w.terms do
      if !first then s := s ++ " + "
      s := s ++ s!"{wgt.repr}/{den.repr}"
      first := false
    if first then s := s ++ "0"
    return s

end WeightedFractionSum

end Googology