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

/-! isFiniteInt_: Some k when α is a finite ordinal (a succ-chain), k = ord α.
    Structural test (no string round-trip needed). -/
private def natOfFinite : OrdinalExpr → Option Nat
  | .zero => some 0
  | .succ a => (natOfFinite a).map (fun n => n + 1)
  | _ => none

def isFiniteInt (a : OrdinalExpr) : Option GoogInt :=
  (natOfFinite a).map (fun n => (n : GoogInt))

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
private partial def valueFiniteLoop (x : Ns) (k β : GoogInt) (acc : Rational) : Rational :=
  if β ≥ k then acc
  else
    let fβ : GoogInt := fAux x (OrdinalExpr.fromInt β)
    valueFiniteLoop x k (β + 1) (acc + Rational.ofNumDen 1 fβ)

def valueFinite (x : Ns) (k : GoogInt) : Rational :=
  valueFiniteLoop x k 1 (Rational.ofNumDen 0 1)

/-! Exact rational value; none for limit index. -/
def value (x : Ns) : Option Rational :=
  isFiniteInt x.alpha |>.map (valueFinite x)

/-! Accumulated weight-fraction form. -/
private partial def buildTerms (x : Ns) (k β : GoogInt) (acc : List (Int × Int)) : List (Int × Int) :=
  if β ≥ k then acc.reverse
  else
    let fβ := fAux x (OrdinalExpr.fromInt β)
    buildTerms x k (β + 1) ((1, fβ) :: acc)

def accumulatedWeightFractions (x : Ns) : Option WeightedFractionSum :=
  isFiniteInt x.alpha |>.map fun k =>
    { intPart := 0, terms := buildTerms x k 1 [] }

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
private def trim (s : String) : String := (s.trimAscii).copy

def fromString (x : Ns) (s : String) : Ns :=
  let t := trim s
  let alphaStr :=
    match t.toLower.splitOn "th" with
    | head :: _ => if head.isEmpty then t else trim ((t.take head.length).copy)
    | [] => t
  let alphaStr := if alphaStr.isEmpty then "1" else alphaStr
  match OrdinalExpr.parse alphaStr with
  | .ok α => { x with alpha := α }
  | .error _ => { x with alpha := OrdinalExpr.fromInt 1 }

def toString (x : Ns) : String :=
  (repr x.alpha).pretty ++ " th \\mathbb{NS}"

/-! expand: §11 pluggable FS on α (limit only). -/
def expand (x : Ns) (k : GoogInt) : Ns :=
  match x.alpha.expand k with
  | .ok α' => { x with alpha := α' }
  | .error _ => x

/-! compare: strictly increasing ⇒ compare α via Mathlib.Ordinal semantics. -/
noncomputable def compare (a b : Ns) : Int := OrdinalExpr.compare a.alpha b.alpha

end Ns

noncomputable instance : Notation Ns where
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
  reduce x := x   -- Ns is already in reduced form

end Googology