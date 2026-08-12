/-
PPS (Parented Predecessor Sequence) — marked-parent ordinal sequence notation.
Fully based on C++:
  include/googology/notations/ordinal/sequence/marked_parent/pps/PPS.hpp
  src/notations/ordinal/sequence/marked_parent/pps/PPS.cpp

Conventions (VERBATIM, article-faithful):
  - Sequence A = (a_1, …, a_n), 1-based columns. Limit expression 0,1,2,3,…
  - Bad root = the x-th term (x = last value). b = bad-root value, y = last
    column, L = y - x.
  - expandLen(A, M): replace last (per variant) + append M elements.
    Appended element p takes source i = p + y - L from the RUNNING sequence;
    copy rule: a_i ≥ x → a_i + L, else a_i.
  - 基本列[n] = expand to the (y + nL - 1)-th term (n ≥ 1). Library 0-index:
    expand(k) = 基本列[k+1] = expandLen((k+1)L - 1).
  - expand_to(M) -> expandLen(A, M).
  - limit(n) = (0,1,…,n-1); masterLimit marked; LIMIT.expand(m) = limit(m).
  - compare: lexicographic (ordinal order), same variant only (cross-variant
    returns -2 = NotComparable sentinel; C++ throws).
  - isSuccessor: sequence ends with 0.
  - PPS3 / ePPS4 / sPPS4 are N/A (no data) — not implemented.
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation

namespace Googology

open Capabilities

/-- PPS variants. -/
inductive PpsVariant where
  | pps1 | pps2 | pps4 | wpps4 | tpps4 | fpps4
  deriving DecidableEq, Repr

namespace PpsVariant

/-- Registry name of a variant. -/
def name : PpsVariant → String
  | .pps1 => "pps1"
  | .pps2 => "pps2"
  | .pps4 => "pps4"
  | .wpps4 => "wpps4"
  | .tpps4 => "tpps4"
  | .fpps4 => "fpps4"

/-- Version code of a variant. -/
def version : PpsVariant → String
  | .pps1 => "1"
  | .pps2 => "2"
  | .pps4 => "4"
  | .wpps4 => "4w"
  | .tpps4 => "4t"
  | .fpps4 => "4f"

end PpsVariant

/-- PPS state: variant + sequence terms + master limit flag. -/
structure Pps where
  variant : PpsVariant := .pps1
  seq : List GoogInt := []
  isMasterLimit : Bool := false
  deriving Repr

namespace Pps

/- Rightmost 1-based column j in the open interval (lo, hi) whose value = b;
   out-of-range columns are skipped; returns 0 if none. -/
def rightmostEq (seq : List GoogInt) (lo hi b : GoogInt) : Int :=
  let ys := seq.length
  let hiCap := min hi (ys + 1)   -- columns are < hi and ≤ ys
  let cands := (List.range hiCap.toNat).filter (fun j => Int.ofNat j > lo)
  let rec scan (cs : List Nat) : Int :=
    match cs with
    | [] => 0
    | j :: rest =>
      if seq.getD (j - 1) 0 = b then Int.ofNat j
      else scan rest
  scan cands.reverse

/- Lexicographic compare of a_{k+j} vs a_{c+j} (j = 0,1,2,…): first
   difference → 1 if the k-side is larger, -1 if smaller; one side exhausted
   first → shorter side is smaller; both exhausted → 0 (all-equal). -/
def tailCmp (seq : List GoogInt) (k c : GoogInt) : Int :=
  let y : Int := seq.length
  let rec scan (js : List Nat) : Int :=
    match js with
    | [] => 0
    | jj :: rest =>
      let j : Int := Int.ofNat jj
      let ik := k + j; let ic := c + j
      let okK := ik ≥ 1 ∧ ik ≤ y
      let okC := ic ≥ 1 ∧ ic ≤ y
      if ¬okK ∧ ¬okC then 0
      else if ¬okK then -1
      else if ¬okC then 1
      else
        let vk := seq.getD (ik.toNat - 1) 0
        let vc := seq.getD (ic.toNat - 1) 0
        if vk ≠ vc then if vk > vc then 1 else -1
        else scan rest
  scan (List.range (y.toNat + 1))

/- Per-variant last-term replacement. Returns (new last value, strong flag).
   strong = entered the strong-expansion branch (PPS4 family only).
   - pps1: term = b between root and last → b, else x - 1.
   - pps2: term = b AND a_{k+j} > a_{c+j} at first difference → b, else x - 1.
   - pps4/wpps4/tpps4/fpps4: weak if term = b exists → b; else strong:
     rightmost term = b in columns (b, x) → its column, else b (as weak). -/
def replaceLast (v : PpsVariant) (seq : List GoogInt) (x bval y : GoogInt) : GoogInt × Bool :=
  match v with
  | .pps1 =>
    let k := rightmostEq seq x y bval
    (if k ≠ 0 then bval else x - 1, false)
  | .pps2 =>
    let k := rightmostEq seq x y bval
    (if k ≠ 0 ∧ tailCmp seq k x > 0 then bval else x - 1, false)
  | .pps4 | .wpps4 | .tpps4 | .fpps4 =>
    let k := rightmostEq seq x y bval
    if k ≠ 0 then (bval, false)
    else
      let j := rightmostEq seq bval x bval
      (if j ≠ 0 then j else bval, true)

/- Copy rule for appended elements. src = source value at 1-based column i.
   Base: src ≥ x → src + L, else src. tpps4/fpps4 on the strong branch add
   the difference L to the last-term chain (i ≡ y mod L); fpps4 sets the
   first last-term copy (i = y) to the bad-root column x. -/
def copied (v : PpsVariant) (src x L y i : GoogInt) (strong : Bool) : GoogInt :=
  let base := if src ≥ x then src + L else src
  if ¬strong then base
  else
    match v with
    | .tpps4 | .fpps4 =>
      if i % L = y % L then
        if v = .fpps4 ∧ i = y then x else src + L
      else base
    | _ => base

/- §12 limits. limit(v, n) = (0,1,…,n-1); masterLimit(v) marked. -/
def limit (v : PpsVariant) (n : GoogInt) : Pps :=
  { variant := v, seq := (List.range n.toNat).map (fun k => Int.ofNat k) }

def masterLimit (v : PpsVariant) : Pps := { variant := v, isMasterLimit := true }

/-- expandLen(A, M): replace last (per variant) + append M elements from the
    running sequence. Defensive: out-of-range bad root (x ≥ y) leaves the
    sequence unchanged (C++ throws out_of_range; here panic-free). -/
def expandLen (b : Pps) (M : GoogInt) : Pps :=
  if b.seq.isEmpty then b
  else
    let y : Int := b.seq.length
    let x := b.seq.getLast!
    if x = 0 then
      -- ends with 0 => successor, strip last (for any M).
      { b with seq := b.seq.dropLast }
    else if x ≥ y then b  -- bad root out of range (C++ throws)
    else
      let bval := b.seq.getD (x.toNat - 1) 0
      let L := y - x
      let (newLast, strong) := replaceLast b.variant b.seq x bval y
      let seq0 := b.seq.set (y.toNat - 1) newLast
      let seq1 := (List.range M.toNat).foldl (fun running s =>
        let i : Int := ((s + 1 : Nat) : Int) + y - L  -- appended position p = s+1
        if i < 1 ∨ i > running.length then running
        else
          let src := running.getD (i.toNat - 1) 0
          running ++ [copied b.variant src x L y i strong]) seq0
      { b with seq := seq1 }

/- Parsing (same as Prss). -/
private def stripOuter (s : String) : String :=
  let cs := s.toList.filter (· ≠ ' ')
  let cs1 := cs.dropWhile (fun c => c = '(' ∨ c = '[' ∨ c = '{')
  let tail := cs1.reverse.dropWhile (fun c => c = ')' ∨ c = ']' ∨ c = '}')
  String.ofList tail.reverse

def fromString (s : String) : Pps :=
  let t := stripOuter s
  if t.isEmpty then { seq := [] }
  else
    let parts := t.splitOn ","
    let seq : List GoogInt := parts.filterMap fun p =>
      let trimmed := p.trimAscii
      if trimmed.isEmpty then none
      else some ((trimmed.toInt?).getD 0)
    { seq }

def toString (b : Pps) : String :=
  if b.isMasterLimit then "(0, 1, 2, …)"
  else
    let parts := b.seq.map (fun v : GoogInt => v.repr)
    s!"({String.intercalate ", " parts})"

/-! Expand. Library 0-index: expand(k) = 基本列[k+1] = expandLen((k+1)L - 1). -/
def expand (b : Pps) (n : GoogInt) : Pps :=
  if b.isMasterLimit then limit b.variant n
  else if b.seq.isEmpty then b
  else
    let x := b.seq.getLast!
    if x = 0 then { b with seq := b.seq.dropLast }
    else
      let y : Int := b.seq.length
      if x ≥ y then b  -- bad root out of range (C++ throws)
      else
        let L := y - x
        expandLen b ((n + 1) * L - 1)

/-- expand_to(M) = expandLen(A, M). -/
def expandTo (b : Pps) (len : GoogInt) : Pps := expandLen b len

/-- A[n] operator (non-mutating) — equivalent to expand. -/
def index (b : Pps) (n : GoogInt) : Pps := expand b n

/- isSuccessor: sequence ends with 0 (master limit -> false). -/
def isSuccessor (b : Pps) : Bool :=
  if b.isMasterLimit then false
  else match b.seq.reverse.head? with | some 0 => true | _ => false

/- Lexicographic compare (ordinal order). Master limit is the supremum.
   Cross-variant comparison returns -2 (NotComparable sentinel; C++ throws). -/
def compare (a b : Pps) : Int :=
  if a.variant ≠ b.variant then -2
  else if a.isMasterLimit ∨ b.isMasterLimit then
    match a.isMasterLimit, b.isMasterLimit with
    | true, true => 0
    | true, false => 1
    | false, true => -1
    | false, false => 0  -- unreachable (guarded by the if above)
  else
    let n1 := a.seq.length; let n2 := b.seq.length
    let m := min n1 n2
    let rec scan (i : Nat) : Int :=
      if i ≥ m then
        if n1 > n2 then 1
        else if n1 < n2 then -1
        else 0
      else
        let x := a.seq.getD i 0; let y := b.seq.getD i 0
        if x > y then 1
        else if x < y then -1
        else scan (i + 1)
    scan 0

/-- normalize(): placeholder idempotence (same as Prss.lean). -/
def normalize (b : Pps) : Pps := b

end Pps

instance : Notation Pps where
  name b := b.variant.name
  family _ := Family.ordinal
  subfamily _ := "sequence"
  style _ := "marked_parent"
  creator _ := "318`4"
  version b := b.variant.version

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.normalize)
      |> (·.set Op.compare)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)
      |> (·.set Op.successor)

  stringToIt _ s := Pps.fromString s
  toLatex b := b.toString
  expand b n := b.expand n
  expandTo b len := b.expandTo len
  compare a o := a.compare o
  reduce a := Pps.normalize a  -- placeholder idempotence (loop-free, like normalize)

end Googology
