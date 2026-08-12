/-
Ns / n,m-Ns — Nonlinear successo real-sequence notation.
By 送到本到 & nnc.

Fully based on C++ Ns implementation (spec §1.1 / §1.5).
Uses `OrdinalExpr` (symbolic expand) + `Mathlib.Ordinal` (semantic values).
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation
import Googology.core.Ordinal
import Googology.core.Rational

namespace Googology

open Capabilities

/-- Ns state: α (ordinal index), n (successor factor), m (§11 FS index). -/
structure Ns where
  alpha : OrdinalExpr := OrdinalExpr.fromInt 1
  n : GoogInt := 2
  m : GoogInt := 2
  deriving Repr

namespace Ns

/-! parameter helpers -/
def isDefaultParams (x : Ns) : Bool := x.n = 2 ∧ x.m = 2
def setParams (x : Ns) (n m : GoogInt) : Ns := { x with n, m }
def params (x : Ns) : GoogInt × GoogInt := (x.n, x.m)

/-! isFiniteInt_: Some k when α prints as pure digits (finite nat k ≥ 0). -/
def isFiniteInt (a : OrdinalExpr) : Option GoogInt :=
  let t := OrdinalExpr.toString a
  if t.isEmpty then none
  else
    if t.all (fun c => c.isDigit) then
      let rec loop (i : Nat) (acc : Int) : Int :=
        if i ≥ t.length then acc
        else loop (i + 1) (acc * 10 + (t[i].toNat - 48 : Int))
      some (loop 0 0)
    else none

/-! f(α) — the Ns core function, recursive on α (§1.1 / §1.5).
    f(0) = 1; f(β+1) = f(β) · n; f(limit α) = f(expand(α, m)).
    Recursion terminates because expand(α,m) < α in the Mathlib.Ordinal
    sense whenever α is a closed limit (the semantic ordinal strictly
    decreases); for open α (containing WV) we fall back to structural
    descent because the article only defines Ns over closed ordinal
    indices. -/
partial def fAux (x : Ns) (α : OrdinalExpr) : GoogInt :=
  if OrdinalExpr.isZero α then 1
  else if OrdinalExpr.isSuccessor α then
    match OrdinalExpr.predecessor α with
    | some pred => fAux x pred * x.n
    | none => 1   -- unreachable when isSuccessor true
  else
    -- limit ordinal: reduce via expand(α, m) (§11 pluggable FS)
    match α.expand x.m with
    | .ok α' => fAux x α'
    | .error _ => 1

/-! valueFinite: a_k = Σ_{β=1}^{k-1} 1/f(β). -/
def valueFinite (x : Ns) (k : GoogInt) : Rational :=
  let rec loop (β : GoogInt) (acc : Rational) : Rational :=
    if β ≥ k then acc
    else
      let fβ : GoogInt := fAux x (OrdinalExpr.fromInt β)
      loop (β + 1) (acc + Rational.mk 1 fβ)
  loop 1 (Rational.mk 0 1)

/-! Exact rational value; none for limit index. -/
def value (x : Ns) : Option Rational :=
  isFiniteInt x.alpha |>.map (valueFinite x)

/-! Accumulated weight-fraction form. -/
def accumulatedWeightFractions (x : Ns) : Option WeightedFractionSum :=
  isFiniteInt x.alpha |>.map fun k =>
    let rec build (β : GoogInt) (acc : List (Int × Int)) : List (Int × Int) :=
      if β ≥ k then acc.reverse
      else
        let fβ := fAux x (OrdinalExpr.fromInt β)
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
  let lower := t.toLower
  let alphaStr := Id.run do
    let mut res := t
    for hi : List Nat ← Id.run do pure [0] do
      let pos := t.toLower.indexOf "th"
      if pos > 0 then
        let before := t.extract 0 pos
        res := trim before
    pure res
  let alphaStr := if alphaStr.isEmpty then "1" else alphaStr
  match OrdinalExpr.parse alphaStr with
  | .ok α => { x with alpha := α }
  | .error _ => { x with alpha := OrdinalExpr.fromInt 1 }

def toString (x : Ns) : String :=
  s!"{OrdinalExpr.toString x.alpha} th \\mathbb{{NS}}"

/-! expand: §11 pluggable FS on α (limit only). -/
def expand (x : Ns) (k : GoogInt) : Ns :=
  match x.alpha.expand k with
  | .ok α' => { x with alpha := α' }
  | .error _ => x

/-! compare: strictly increasing ⇒ compare α via Mathlib.Ordinal semantics. -/
def compare (a b : Ns) : Int := OrdinalExpr.compare a.alpha b.alpha

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
  expandTo b _ := b
  compare a o := a.compare o

end Googology