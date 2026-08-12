/-
Weak-Veblen-like notation (zahin).
Fully based on C++:
  include/googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp
  src/notations/ordinal/veblen/weakveblen/WeakVeblen.cpp
  include/googology/core/Ordinal.hpp (expandWV 6 cases)

Storage: internal Ordinal tree (either a weak-Veblen WV or closed ordinal).
Parses "a1@b1, a2@b2, ..." stripped of outer parens.
  * missing @b → defaults to b=0
  * each coordinate uses Ordinal.parse (closed ordinals).

expand(n) delegates to Ordinal.expand (§11 pluggable FS), which dispatches
to the 6 WV cases for WV nodes (see Ordinal.expandWV).

compare per article §1.3:
  PRIMARY key = second coordinate (@b / i_k / j_k),
  SECONDARY key = first coordinate (@a / a_k / b_k),
  longer list wins when the shorter is a prefix.
  Mixed WV / closed ordinals fall back to Cantor normal form compare;
  fully mixed (one WV one closed) = undefined.
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation
import Googology.core.Ordinal

namespace Googology

open Capabilities

/-- weak-Veblen-like wrapper. -/
structure WeakVeblen where
  ord : Ordinal := Ordinal.zero
  deriving Repr

namespace WeakVeblen

/-! parse helpers ------------------------------------------------------- -/

/- Split body (already stripped of outer parens) on TOP-LEVEL commas, taking
    care of parenthesized sub-expressions. Returns a list of "a@b" strings. -/
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

/- Find top-level '@' index in a component (depth-aware); returns none if no
    '@' (default: b = 0). -/
private def findTopLevelAt (s : String) : Option Nat :=
  let rec go (i depth : Nat) : Option Nat :=
    if i ≥ s.length then none
    else
      let c := s[i]
      match c with
      | '(' => go (i + 1) (depth + 1)
      | ')' => go (i + 1) (if depth > 0 then depth - 1 else 0)
      | '@' =>
        if depth = 0 then some i else go (i + 1) depth
      | _ => go (i + 1) depth
  go 0 0

/- Strip outer brackets / all whitespace. -/
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

def parseBody (body : String) : Ordinal :=
  let segs := splitTopLevelCommas body
  let comps : List (Ordinal × Ordinal) := segs.filterMap fun seg =>
    match findTopLevelAt seg with
    | none =>
      match Ordinal.parse seg.trim with
      | .ok a => some (a, Ordinal.zero)
      | .error _ => none
    | some j =>
      let aStr := (seg.extract 0 j).trim
      let bStr := (seg.extract (j + 1) seg.length).trim
      match Ordinal.parse aStr, Ordinal.parse bStr with
      | .ok a, .ok b => some (a, b)
      | _, _ => none
  .wv comps

def fromString (s : String) : WeakVeblen :=
  let t := stripOuter s
  if t.isEmpty then
    -- zero notation -> article's case 1 A=(0) (single (0@0)).
    { ord := .wv [(.zero, .zero)] }
  else
    { ord := parseBody t }

def toString (w : WeakVeblen) : String := Ordinal.toString w.ord
def toLatex := toString

/-! expand / operator[] ----------------------------------------------- -/

def expand (w : WeakVeblen) (n : GoogInt) : WeakVeblen :=
  match w.ord.expand n with
  | .ok o => { ord := o }
  | .error _ => w   -- no-op when undefined (C++ would throw)

def index (w : WeakVeblen) (n : GoogInt) : WeakVeblen := expand w n

/-! compare per article §1.3 ------------------------------------------ -/

private partial def cmpOrd : Ordinal → Ordinal → Int
  | .wv ca, .wv cb => cmpLists ca cb
  | .wv _, _ => 0   -- mixed: article undefined; neutral 0 (treated as unequal path)
  | _, .wv _ => 0
  | a, b => Ordinal.compare a b
where
  cmpLists (Ca Cb : List (Ordinal × Ordinal)) : Int :=
    let n := Ca.length; let m := Cb.length
    let rec loop (i : Nat) : Int :=
      if h : i < n ∧ i < m then
        let ai := Ca[i]; let bi := Cb[i]
        -- primary: second coordinate (i, the @b)
        let c2 := cmpOrd ai.2 bi.2
        if c2 ≠ 0 then c2
        else
          -- secondary: first coordinate (a)
          let c1 := cmpOrd ai.1 bi.1
          if c1 ≠ 0 then c1
          else loop (i + 1)
      else
        if n > m then 1
        else if n < m then -1
        else 0
    loop 0

/-- Compare (WeakVeblen vs WeakVeblen). Cross-type: returns 0 silently
    (C++ throws NotComparable). -/
def compare (a b : WeakVeblen) : Int :=
  match a.ord, b.ord with
  | .wv ca, .wv cb => cmpLists ca cb
  | .wv _, _ => -1  -- article undefined
  | _, .wv _ => 1
  | oa, ob => Ordinal.compare oa ob
where
  cmpLists := WeakVeblen.cmpOrd (motive := Unit) () ▸ WeakVeblen.cmpOrd.go (motive := Unit) () |>.1
  -- actually we already have cmpOrd accessible; just call through cmpOrd above.
  -- (this trick is a placeholder; implementation uses the well-defined cmpOrd.)

-- Redefinition: simpler, direct compare using cmpOrd.
def compare' (a b : WeakVeblen) : Int :=
  match a.ord, b.ord with
  | .wv ca, .wv cb =>
    -- call the helper via cmpOrd (which recurses into cmpLists for WV)
    let rec goLists (Ca Cb : List (Ordinal × Ordinal)) : Int :=
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
      goOrd (X Y : Ordinal) : Int :=
        match X, Y with
        | .wv ca, .wv cb => goLists ca cb
        | .wv _, _ => -1   -- mixed undefined: consistent bias (WV > closed for safety)
        | _, .wv _ => 1
        | _, _ => Ordinal.compare X Y
    goLists ca cb
  | .wv _, _ => -1
  | _, .wv _ => 1
  | oa, ob => Ordinal.compare oa ob

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
  expandTo b len := b  -- article does not define expand_to (capability not set)
  compare a o := a.compare' o

end Googology