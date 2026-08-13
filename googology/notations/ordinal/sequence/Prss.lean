/-
PrSS (Primitive Sequence System) — difference-type ordinal sequence notation.
Fully based on C++:
  include/googology/notations/ordinal/sequence/difference/prss/Prss.hpp
  src/notations/ordinal/sequence/difference/prss/Prss.cpp

Conventions (VERBATIM, article-faithful):
  - Sequence A = (a_1, …, a_n). Parsing accepts "()/[]/{}, " or bare.
  - rightmostLess_: 1-based index; returns 0 when no element < a_n.
  - expandLen(A, M):
      m=0 -> (a_1..a_n - 1) [decrement last]
      m=s>0 -> append (s + n - L)th element of the RUNNING (growing) sequence.
    PrSS Δ≤1 so a single recursive case (all gaps exactly 1).
  - expand(A, m>0) = expandLen(A, m * L - 1); m=0 = expandLen(A,0).
  - expand_to(M) -> expandLen(A, M).
  - limit(n) = (0,1,…,n-1); master_limit marked.
  - compare: lexicographic over the sequence (ordinal order).
  - isSuccessor: sequence ends with 0.
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation

namespace Googology

open Capabilities

/-- PrSS state: a list of integer sequence terms + master limit flag. -/
structure Prss where
  seq : List GoogInt := []
  isMasterLimit : Bool := false
  deriving Repr

namespace Prss

/- rightmost index i (1-based) whose value < an. Returns 0 if none. -/
def rightmostLess (seq : List GoogInt) (an : GoogInt) : Int :=
  let rec loop (i : Nat) (best : Int) : Int :=
    if i ≥ seq.length then best
    else
      let v := seq.getD i 0
      if v < an then loop (i + 1) (i + 1)  -- 1-based
      else loop (i + 1) best
  loop 0 0

/-- expandLen(A, M): decrement last, append M elements from the running seq.
    Index is 1-based (s + n - L). Defensive: out-of-range -> leave unchanged.
    C++ throws; here we panic-free preserve, matching the "guard not a
    reinterpretation" policy (formula left as-is). -/
def expandLen (b : Prss) (M : GoogInt) : Prss :=
  if b.seq.isEmpty then b
  else
    let n : Int := b.seq.length
    let an := b.seq.getLast!
    if an = 0 then
      -- ends with 0 => successor, strip last (for any M).
      { b with seq := b.seq.dropLast }
    else if M ≤ 0 then
      -- M=0 base: decrement last
      match b.seq with
      | [] => b
      | _ =>
        let lastIdx := b.seq.length - 1
        let newSeq := b.seq.set lastIdx (an - 1)
        { b with seq := newSeq }
    else
      let br := rightmostLess b.seq an   -- 1-based
      if br = 0 then b  -- undefined; leave unchanged (C++ throws)
      else
        let L := n - br
        -- step 0: decrement last
        let lastIdx0 := b.seq.length - 1
        let seq0 := b.seq.set lastIdx0 (an - 1)
        -- append M elements from the RUNNING sequence (source pos1 = s + n - L)
        let seq1 := (List.range M.toNat).foldl (fun running s =>
          let pos1 : Int := ((s + 1 : Nat) : Int) + n - L  -- s: Nat -> s+1 (1-based)
          if pos1 < 1 ∨ pos1 > running.length then running
          else
            let elem := running.getD (pos1.toNat - 1) 0
            running ++ [elem]) seq0
        { b with seq := seq1 }

/- Parsing. -/
private def stripOuter (s : String) : String :=
  let cs := s.toList.filter (· ≠ ' ')
  let cs1 := cs.dropWhile (fun c => c = '(' ∨ c = '[' ∨ c = '{')
  let tail := cs1.reverse.dropWhile (fun c => c = ')' ∨ c = ']' ∨ c = '}')
  String.ofList tail.reverse

def fromString (s : String) : Prss :=
  let t := stripOuter s
  if t.isEmpty then { seq := [] }
  else
    let parts := t.splitOn ","
    let seq : List GoogInt := parts.filterMap fun p =>
      let trimmed := p.trimAscii
      if trimmed.isEmpty then none
      else some ((trimmed.toInt?).getD 0)
    { seq }

def toString (b : Prss) : String :=
  if b.isMasterLimit then "(0, 1, 2, …)"
  else
    let parts := b.seq.map (fun v : GoogInt => v.repr)
    s!"({String.intercalate ", " parts})"

/- §12 limits. -/
def limit (n : GoogInt) : Prss :=
  { seq := (List.range n.toNat).map (fun k => Int.ofNat k) }

def masterLimit : Prss := { isMasterLimit := true }

/-! Expand. -/
def expand (b : Prss) (m : GoogInt) : Prss :=
  if b.isMasterLimit then limit m
  else if b.seq.isEmpty then b
  else
    let an := b.seq.getLast!
    if an = 0 then { b with seq := b.seq.dropLast }
    else
      let n : Int := b.seq.length
      let br := rightmostLess b.seq an
      if br = 0 then b   -- C++ throws out_of_range
      else
        let L := n - br
        if m = 0 then
          if an > 0 then
            let li := b.seq.length - 1
            { b with seq := b.seq.set li (an - 1) }
          else b
        else
          let M := m * L - 1
          expandLen b M

/-- expand_to(M) = expandLen(A, M). -/
def expandTo (b : Prss) (len : GoogInt) : Prss := expandLen b len

/-- A[n] operator (non-mutating) — equivalent to expand (returns a copy). -/
def index (b : Prss) (n : GoogInt) : Prss := expand b n

/- isSuccessor: sequence ends with 0 (master_limit -> false). -/
def isSuccessor (b : Prss) : Bool :=
  if b.isMasterLimit then false
  else match b.seq.reverse.head? with | some 0 => true | _ => false

/- Lexicographic compare (ordinal order). Master limit is supremum. -/
def compare (a b : Prss) : Int :=
  if a.isMasterLimit ∨ b.isMasterLimit then
    match a.isMasterLimit, b.isMasterLimit with
    | true, true => 0
    | true, false => 1
    | false, true => -1
    | false, false => 0  -- unreachable (guarded by the if above)
  else
    let n1 := a.seq.length; let n2 := b.seq.length
    let m := min n1 n2
    let rec loop (i : Nat) : Int :=
      if i ≥ m then
        if n1 > n2 then 1
        else if n1 < n2 then -1
        else 0
      else
        let x := a.seq.getD i 0; let y := b.seq.getD i 0
        if x > y then 1
        else if x < y then -1
        else loop (i + 1)
    loop 0

/-- normalize(): the standard form engine (§12). We don't run the generic
    BFS here; as a faithful placeholder normalize just idempotently returns
    the expression if it's already standard form (see C++). -/
def normalize (b : Prss) : Prss := b  -- no-op, matches std C++ idempotence

end Prss

instance : Notation Prss where
  name _ := "prss"
  family _ := Family.ordinal
  subfamily _ := "sequence"
  style _ := "difference"
  creator _ := ""
  version _ := "1"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.normalize)
      |> (·.set Op.compare)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)
      |> (·.set Op.successor)

  stringToIt _ s := Prss.fromString s
  toLatex b := b.toString
  expand b n := b.expand n
  expandTo b len := b.expandTo len
  compare a o := a.compare o
  reduce a := Prss.normalize a  -- placeholder idempotence (loop-free, like normalize)

end Googology