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

private def splitTopLevelCommas (body : String) : List String :=
  let rec go (i depth : Nat) (cur : List Char) (acc : List String) : List String :=
    if h : i < body.length then
      let c := body[i]
      match c with
      | '(' => go (i + 1) (depth + 1) (c :: cur) acc
      | ')' => go (i + 1) (if depth > 0 then depth - 1 else 0) (c :: cur) acc
      | ',' =>
        if depth = 0 then
          let seg := String.mk cur.reverse
          go (i + 1) 0 [] (acc ++ [seg])
        else go (i + 1) depth (c :: cur) acc
      | _ => go (i + 1) depth (c :: cur) acc
    else
      if cur.isEmpty then acc
      else acc ++ [String.mk cur.reverse]
  go 0 0 [] []

private def findTopLevelAt (s : String) : Option Nat :=
  let rec go (i depth : Nat) : Option Nat :=
    if i ≥ s.length then none
    else
      let c := s[i]
      match c with
      | '(' => go (i + 1) (depth + 1)
      | ')' => go (i + 1) (if depth > 0 then depth - 1 else 0)
      | '@' => if depth = 0 then some i else go (i + 1) depth
      | _ => go (i + 1) depth
  go 0 0

private def stripOuter (s : String) : String :=
  let noSpace := s.filter (· ≠ ' ')
  let rec dropFront (t : String) : String :=
    if !t.isEmpty ∧ (t.front = '(' ∨ t.front = '[' ∨ t.front = '{')
    then dropFront (t.drop 1) else t
  let rec dropBack (cs : List Char) : List Char :=
    match cs with
    | [] => []
    | cs =>
      match cs.reverse with
      | [] => []
      | last :: rinit =>
        if last = ')' ∨ last = ']' ∨ last = '}'
        then dropBack rinit.reverse
        else cs
  String.mk (dropBack (dropFront noSpace).data)

/-! Parser / stringifier ---------------------------------------------- -/

def parseBody (body : String) : OrdinalExpr :=
  let segs := splitTopLevelCommas body
  let comps : List (OrdinalExpr × OrdinalExpr) := segs.filterMap fun seg =>
    match findTopLevelAt seg with
    | none =>
      match OrdinalExpr.parse seg.trim with
      | .ok a => some (a, OrdinalExpr.zero)
      | .error _ => none
    | some j =>
      let aStr := (seg.extract 0 j).trim
      let bStr := (seg.extract (j + 1) seg.length).trim
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

def toString (w : WeakVeblen) : String := OrdinalExpr.toString w.ord
def toLatex := toString

/-! expand / operator[] ----------------------------------------------- -/

def expand (w : WeakVeblen) (n : GoogInt) : WeakVeblen :=
  match w.ord.expand n with
  | .ok o => { ord := o }
  | .error _ => w

def index (w : WeakVeblen) (n : GoogInt) : WeakVeblen := expand w n

/-! compare per article §1.3: primary = @b (second), secondary = @a (first).
    Closed (non-WV) ordinals fall back to Mathlib.Ordinal compare via
    OrdinalExpr.compare. -/

def compare (a b : WeakVeblen) : Int :=
  let rec goLists (Ca Cb : List (OrdinalExpr × OrdinalExpr)) : Int :=
    match Ca, Cb with
    | [], [] => 0
    | [], _ => -1
    | _, [] => 1
    | (a1, b1) :: Ca', (a2, b2) :: Cb' =>
      let c2 := goOrd b1 b2
      if c2 ≠ 0 then c2
      else
        let c1 := goOrd a1 a2
        if c1 ≠ 0 then c1
        else goLists Ca' Cb'
  where
    goOrd (X Y : OrdinalExpr) : Int :=
      match X, Y with
      | .wv ca, .wv cb => goLists ca cb
      | .wv _, _ => -1   -- undefined mixed ordering: consistent bias
      | _, .wv _ => 1
      | _, _ => OrdinalExpr.compare X Y
  match a.ord, b.ord with
  | .wv ca, .wv cb => goLists ca cb
  | .wv _, _ => -1
  | _, .wv _ => 1
  | oa, ob => OrdinalExpr.compare oa ob

end WeakVeblen

instance : Notation WeakVeblen where
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

end Googology