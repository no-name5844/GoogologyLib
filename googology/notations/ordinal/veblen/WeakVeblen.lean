/-
Weak-Veblen-like notation (zahin).
Fully based on C++ implementation. Uses `OrdinalExpr` (syntax expand) +
`Mathlib.Ordinal` (semantics for closed-component compare).
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation
import Googology.core.Ordinal

namespace Googology

open Capabilities

/-- weak-Veblen-like wrapper; ordinal field is an OrdinalExpr (WV or closed). -/
structure WeakVeblen where
  ord : OrdinalExpr := OrdinalExpr.wv [(OrdinalExpr.zero, OrdinalExpr.zero)]
  deriving Repr

namespace WeakVeblen

/-! parse helpers ------------------------------------------------------- -/

/-- Split on commas at top nesting depth; substrings are `body`-slices. -/
private partial def splitTopLevelCommasLoop (body : String) (p : body.Pos) (depth : Nat)
    (start : body.Pos) (acc : List String) : List String :=
  match p.get? with
  | none => if p = start then acc else acc ++ [body.extract start p]
  | some c =>
    match c with
    | '(' => splitTopLevelCommasLoop body p.next! (depth + 1) start acc
    | ')' => splitTopLevelCommasLoop body p.next! (if depth > 0 then depth - 1 else 0) start acc
    | ',' =>
      if depth = 0 then
        splitTopLevelCommasLoop body p.next! 0 p.next! (acc ++ [body.extract start p])
      else splitTopLevelCommasLoop body p.next! depth start acc
    | _ => splitTopLevelCommasLoop body p.next! depth start acc

private def splitTopLevelCommas (body : String) : List String :=
  splitTopLevelCommasLoop body body.startPos 0 body.startPos []

/-- Position of the top-level '@' in `s`, if any. -/
private partial def findTopLevelAtLoop (s : String) (p : s.Pos) (depth : Nat) : Option s.Pos :=
  match p.get? with
  | none => none
  | some c =>
    match c with
    | '(' => findTopLevelAtLoop s p.next! (depth + 1)
    | ')' => findTopLevelAtLoop s p.next! (if depth > 0 then depth - 1 else 0)
    | '@' => if depth = 0 then some p else findTopLevelAtLoop s p.next! depth
    | _ => findTopLevelAtLoop s p.next! depth

private def findTopLevelAt (s : String) : Option s.Pos :=
  findTopLevelAtLoop s s.startPos 0

/-- Drop leading '(' / '[' / '{' (and whitespace) and matching trailing
    ')' / ']' / '}' from both ends. -/
private partial def stripFrontLoop (s : String) (p : s.Pos) : s.Pos :=
  match p.get? with
  | some c => if c = '(' ∨ c = '[' ∨ c = '{' ∨ c = ' ' ∨ c = '\t' then stripFrontLoop s p.next! else p
  | none => p

private partial def stripBackLoop (s : String) (p : s.Pos) (last : s.Pos) : s.Pos :=
  match p.get? with
  | some c =>
    if c = ')' ∨ c = ']' ∨ c = '}' ∨ c = ' ' ∨ c = '\t' then stripBackLoop s p.next! last
    else stripBackLoop s p.next! p.next!
  | none => last

private def stripOuter (s : String) : String :=
  let f := stripFrontLoop s s.startPos
  let b := stripBackLoop s f s.endPos
  s.extract f b

private def trimStr (s : String) : String := (s.trimAscii).copy

/-! Parser / stringifier ---------------------------------------------- -/

def parseBody (body : String) : OrdinalExpr :=
  let segs := splitTopLevelCommas body
  let comps : List (OrdinalExpr × OrdinalExpr) := segs.filterMap fun seg =>
    match findTopLevelAt seg with
    | none =>
      match OrdinalExpr.parse (trimStr seg) with
      | .ok a => some (a, OrdinalExpr.zero)
      | .error _ => none
    | some jPos =>
      let aStr := trimStr (seg.extract seg.startPos jPos)
      let bStr := trimStr (seg.extract jPos.next! seg.endPos)
      match OrdinalExpr.parse aStr, OrdinalExpr.parse bStr with
      | .ok a, .ok b => some (a, b)
      | _, _ => none
  .wv comps

def fromString (s : String) : WeakVeblen :=
  let t := stripOuter s
  if t.isEmpty then
    { ord := .wv [(.zero, .zero)] }
  else
    { ord := parseBody t }

def toString (w : WeakVeblen) : String := (repr w.ord).pretty
def toLatex := toString

/-! expand / operator[] ----------------------------------------------- -/

def expand (w : WeakVeblen) (n : GoogInt) : WeakVeblen :=
  match w.ord.expand n with
  | .ok o => { ord := o }
  | .error _ => w

def index (w : WeakVeblen) (n : GoogInt) : WeakVeblen := expand w n

/-! compare per article §1.3: primary = @b (second), secondary = @a (first).
    Closed (non-WV) ordinals fall back to Mathlib.Ordinal compare via
    OrdinalExpr.compare. The WV-structure recursion (`wvCmpLists` / `wvCmpOrd`)
    is computable and parameterized by the leaf compare, so the noncomputable
    `OrdinalExpr.compare` only enters through `compare` itself. -/

/-- Lexicographic compare of coordinate lists; `leaf` compares single coords. -/
private partial def wvCmpLists (leaf : OrdinalExpr → OrdinalExpr → Int)
    (Ca Cb : List (OrdinalExpr × OrdinalExpr)) : Int :=
  match Ca, Cb with
  | [], [] => 0
  | [], _ => -1
  | _, [] => 1
  | (a1, b1) :: Ca', (a2, b2) :: Cb' =>
    let c2 := leaf b1 b2
    if c2 ≠ 0 then c2
    else
      let c1 := leaf a1 a2
      if c1 ≠ 0 then c1
      else wvCmpLists leaf Ca' Cb'

/-- Coordinate compare: nested WV defers to `wvCmpLists`; closed coords use
    `base` (the semantic Mathlib.Ordinal compare). -/
private partial def wvCmpOrd (base : OrdinalExpr → OrdinalExpr → Int)
    (X Y : OrdinalExpr) : Int :=
  match X, Y with
  | .wv ca, .wv cb => wvCmpLists (wvCmpOrd base) ca cb
  | .wv _, _ => -1   -- undefined mixed ordering: consistent bias
  | _, .wv _ => 1
  | _, _ => base X Y

noncomputable def compare (a b : WeakVeblen) : Int :=
  match a.ord, b.ord with
  | .wv ca, .wv cb => wvCmpLists (wvCmpOrd OrdinalExpr.compare) ca cb
  | .wv _, _ => -1
  | _, .wv _ => 1
  | oa, ob => OrdinalExpr.compare oa ob

end WeakVeblen

noncomputable instance : Notation WeakVeblen where
  name _ := "weak_veblen"
  family _ := Family.ordinal
  subfamily _ := "veblen"
  style _ := "weak-like"
  creator _ := "zahin"
  version _ := "1"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.compare)
      |> (·.set Op.expand)

  stringToIt _ s := WeakVeblen.fromString s
  toLatex w := w.toLatex
  expand w n := w.expand n
  expandTo b _ := b   -- expand_to not in capability set
  compare a o := a.compare o
  reduce x := x   -- weak-Veblen expressions are already in reduced form

end Googology