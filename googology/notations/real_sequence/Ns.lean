/-
Ns / n,m-Ns — Nonlinear successo real-sequence notation.
By 送到本到 & nnc.

Fully based on C++:
  include/googology/notations/real_sequence/ns/Ns.hpp
  src/notations/real_sequence/ns/Ns.cpp

Storage:
  alpha_ : Ordinal (ordinal index α — "α th NS")
  n_, m_ : Int   (successor factor & limit FS index; Ns default n=m=2)
  isDefaultParams() = (n=2 ∧ m=2).

Core f(α): Ns defining function (spec §1.1 / §1.5):
    f(0)       = 1
    f(β+1)     = f(β) · n
    f(limit α) = f(expand(α, m))   (Ns: m=2 -> expand(α,2))
Returns BigInt for every α; recursion terminates because expand(α,m) < α.
For finite α=k: f(k) = n^k.

Value / accumulated fractions:
    a_1 = 0;  a_{β+1} = a_β + 1/f(β)  =>  a_k = Σ_{β=1}^{k-1} 1/f(β)
Only FINITE ordinal α gives an exact rational; limit indices are NOT
auto-computed (library policy).

expand(k): §11 pluggable FS → α.expand(k) (maps limit indices only).
compare : strictly increasing a_α in α ⟹ order on values = order on α.
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation
import Googology.core.Ordinal
import Googology.core.Rational

namespace Googology

open Capabilities

structure Ns where
  alpha : Ordinal := Ordinal.fromInt 1
  n : GoogInt := 2
  m : GoogInt := 2
  deriving Repr

namespace Ns

/-! params helpers -/
def isDefaultParams (x : Ns) : Bool := x.n = 2 ∧ x.m = 2
def setParams (x : Ns) (n m : GoogInt) : Ns := { x with n, m }
def params (x : Ns) : GoogInt × GoogInt := (x.n, x.m)

/-! isFiniteInt_ (C++): returns Option k when `a` prints as a digit-only
    string (= finite CNF integer k ≥ 0). None otherwise. -/
def isFiniteInt (a : Ordinal) : Option GoogInt :=
  let t := Ordinal.toString a
  if t.isEmpty then none
  else
    if t.all (fun c => c.isDigit) then
      -- parse digits
      let rec loop (i : Nat) (acc : Int) : Int :=
        if i ≥ t.length then acc
        else
          let d := (t[i].toNat - 48 : Int)
          loop (i + 1) (acc * 10 + d)
      some (loop 0 0)
    else none

/-! f(α) — the Ns core function, recursive on α. -/
partial def fAux (x : Ns) (α : Ordinal) : GoogInt :=
  if Ordinal.isZero' α then 1
  else if Ordinal.isSuccessor α then
    match Ordinal.predecessor α with
    | .ok pred => fAux x pred * x.n
    | .error _ => 1   -- should be unreachable if isSuccessor
  else
    -- limit ordinal: reduce via expand(α, m). C++ uses pluggable FS here.
    -- If expand is undefined (e.g. closed CNF), we can't reduce → return 1.
    match α.expand x.m with
    | .ok α' => fAux x α'
    | .error _ => 1   -- defensive fallback

/-! valueFinite_ for finite index k ≥ 1:
     a_k = Σ_{β=1}^{k-1} 1/f(β). -/
def valueFinite (x : Ns) (k : GoogInt) : Rational :=
  let rec loop (β : GoogInt) (acc : Rational) : Rational :=
    if β ≥ k then acc
    else
      let fβ : GoogInt := fAux x (Ordinal.fromInt β)
      let step : Rational := Rational.mk 1 fβ
      loop (β + 1) (acc + step)
  loop 1 (Rational.mk 0 1)

/-! Exact rational value; none for limit index. -/
def value (x : Ns) : Option Rational :=
  isFiniteInt x.alpha |>.map (valueFinite x)

/-! Accumulated weight-fraction form (int part + w/d terms). -/
def accumulatedWeightFractions (x : Ns) : Option WeightedFractionSum :=
  isFiniteInt x.alpha |>.map fun k =>
    let rec build (β : GoogInt) (acc : List (Int × Int)) : List (Int × Int) :=
      if β ≥ k then acc.reverse
      else
        let fβ := fAux x (Ordinal.fromInt β)
        build (β + 1) ((1, fβ) :: acc)
    { intPart := 0, terms := build 1 [] }

/-! Fraction string (e.g. "1/2 + 1/4"). -/
def toFractionString (x : Ns) : String :=
  match isFiniteInt x.alpha with
  | none => "limit-index"
  | some k =>
    if k ≤ 1 then "0"
    else
      match accumulatedWeightFractions x with
      | some w => w.toString
      | none => "0"

/-! Parsing: "α th NS" or just "α". -/
private def trim (s : String) : String :=
  let rec front (i : Nat) : Nat :=
    if i < s.length ∧ (s[i] = ' ' ∨ s[i] = '\t') then front (i + 1) else i
  let rec back (j : Nat) : Nat :=
    if j > 0 ∧ let c := s[j - 1]; c = ' ' ∨ c = '\t' then back (j - 1) else j
  let a := front 0; let b := back s.length
  s.extract a b

def fromString (x : Ns) (s : String) : Ns :=
  let t := trim s
  -- find "th" case-insensitively
  let lower := t.toLower
  let alphaStr :=
    match t.toLower.indexOf "th" with
    | 0 => t   -- fallback, shouldn't be at start
    | pos =>
      -- substring before pos
      if pos > 0 then
        let before := t.extract 0 pos
        trim before
      else t
  let alphaStr := if alphaStr.isEmpty then "1" else alphaStr
  match Ordinal.parse alphaStr with
  | .ok α => { x with alpha := α }
  | .error _ => { x with alpha := Ordinal.fromInt 1 }

def toString (x : Ns) : String :=
  s!"{Ordinal.toString x.alpha} th \\mathbb{{NS}}"

/-! expand: §11 pluggable FS on α (limit only). -/
def expand (x : Ns) (k : GoogInt) : Ns :=
  match x.alpha.expand k with
  | .ok α' => { x with alpha := α' }
  | .error _ => x   -- undefined for finite/successor (capability set but no-op)

/-! compare: strictly increasing ⇒ compare α. -/
def compare (a b : Ns) : Int := Ordinal.compare a.alpha b.alpha

end Ns

instance : Notation Ns where
  name _ := "ns"
  family _ := Family.realSequence
  subfamily _ := "ns"
  style _ := "real_sequence"
  creator _ := "送到本到, nnc"
  version _ := "1"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.expand)
      |> (·.set Op.compare)

  stringToIt ns s := ns.fromString s
  toLatex x := x.toString
  expand x k := x.expand k
  expandTo b _ := b   -- article does not define expand_to for Ns
  compare a o := a.compare o

end Googology