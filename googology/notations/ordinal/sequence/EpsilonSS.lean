/-
ε_pSS (EpspSS) / ε_ωSS (EpsOmegaSS) — merged module (C++ project convention:
same type because both share the article "epsilon_nSS & epsilon_omegaSS.md").

Fully based on C++:
  include/googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp
  src/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.cpp

The only structural difference is the per-step addition:
  * EpspSS    has parameter p (cap on added amount, case 4).
  * EpsOmegaSS has no p, case 3 applies for all q > 1 (unbounded).
Both share:
  - expandLen(A, M):
      m=0 -> decrement last
      append:
        q == 1       -> append (s + n - L)th running (case 2)
        1 < q <= p   -> append (n + s - 1)th running + (q-1) (case 3)
        q > p        -> append (n + s - 1)th running + p     (case 4, EpspSS only)
  - expand(A, m>0) = expandLen(A, m*L - 1); m=0 = expandLen(A,0).
  - expand_to(M) = expandLen(A, M).
  - limit(n) = (1, n); master_limit marked.
  - compare: lexicographic over the sequence.
  - isSuccessor: sequence ends with 1.
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation

namespace Googology

open Capabilities

/- EpspSS (ε_pSS): sequence + master limit + cap parameter p. -/
structure EpspSS where
  seq : List GoogInt := []
  p : GoogInt := 1
  isMasterLimit : Bool := false
  deriving Repr

/- EpsOmegaSS (ε_ωSS): no p, no case 4. -/
structure EpsOmegaSS where
  seq : List GoogInt := []
  isMasterLimit : Bool := false
  deriving Repr

namespace CommonEps

/- rightmost index (1-based) whose value < an. 0 if none. -/
def rightmostLess (seq : List GoogInt) (an : GoogInt) : Int :=
  let rec loop (i : Nat) (best : Int) : Int :=
    if i ≥ seq.length then best
    else
      if seq.getD i 0 < an then loop (i + 1) (i + 1)
      else loop (i + 1) best
  loop 0 0

/- Strip outer brackets / whitespace for parsing. -/
def stripOuter (s : String) : String :=
  let noSpace := s.filter (· ≠ ' ')
  let rec dropFront (t : String) : String :=
    if !t.isEmpty ∧ (t.front = '(' ∨ t.front = '[' ∨ t.front = '{')
    then dropFront (t.drop 1) else t
  let front := dropFront noSpace
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
  String.mk (dropBack front.data)

def parseSeq (s : String) : List GoogInt :=
  let t := stripOuter s
  if t.isEmpty then []
  else
    let parts := t.splitOn ","
    parts.filterMap fun p =>
      let trimmed := p.trim
      if trimmed.isEmpty then none
      else some (trimmed.toInt!.getD 0)

def toString (seq : List GoogInt) : String :=
  let parts := seq.map toString
  s!"({String.intercalate ", " parts})"

/- Expand helper — EpspSS case split with `pC` cap. -/
def expandLenEpsp (seq : List GoogInt) (pC : GoogInt) (M : GoogInt) : List GoogInt :=
  if seq.isEmpty then seq
  else
    let an := seq.getLast!
    if an = 1 then seq.dropLast   -- ends with 1 => successor
    else
      let n : Int := seq.length
      let br1 := rightmostLess seq an
      if br1 = 0 then seq
      else if M ≤ 0 then
        if an > 0 then
          seq.set (seq.length - 1) (an - 1)
        else seq
      else
        let L := n - br1
        let a_br := seq.getD (br1.toNat - 1) 0
        let q := an - a_br
        -- step 0: decrement last
        let seq0 := seq.set (seq.length - 1) (an - 1)
        let rec loop (s : Int) (running : List GoogInt) : List GoogInt :=
          if s > M then running
          else
            let pos1 : Int; let add : GoogInt
            if q = 1 then
              pos1 := s + n - L; add := 0
            else if q ≤ pC then
              pos1 := n + s - 1; add := q - 1
            else
              pos1 := n + s - 1; add := pC
            if pos1 < 1 ∨ pos1 > running.length then running
            else
              let elem := running.get! (pos1.toNat - 1)
              loop (s + 1) (running ++ [elem + add])
        loop 1 seq0

/- Expand helper — EpsOmegaSS (no p, no case 4). -/
def expandLenOmega (seq : List GoogInt) (M : GoogInt) : List GoogInt :=
  if seq.isEmpty then seq
  else
    let an := seq.getLast!
    if an = 1 then seq.dropLast
    else
      let n : Int := seq.length
      let br1 := rightmostLess seq an
      if br1 = 0 then seq
      else if M ≤ 0 then
        if an > 0 then seq.set (seq.length - 1) (an - 1) else seq
      else
        let L := n - br1
        let a_br := seq.getD (br1.toNat - 1) 0
        let q := an - a_br
        let seq0 := seq.set (seq.length - 1) (an - 1)
        let rec loop (s : Int) (running : List GoogInt) : List GoogInt :=
          if s > M then running
          else
            let pos1 : Int; let add : GoogInt
            if q = 1 then
              pos1 := s + n - L; add := 0
            else
              pos1 := n + s - 1; add := q - 1
            if pos1 < 1 ∨ pos1 > running.length then running
            else
              let elem := running.get! (pos1.toNat - 1)
              loop (s + 1) (running ++ [elem + add])
        loop 1 seq0

/- Common expand (master_limit redirects). -/
def expandEpsp (b : EpspSS) (m : GoogInt) : EpspSS :=
  if b.isMasterLimit then
    -- limit(m): (1, m) for m > 0, () for m = 0
    let seq := if m > 0 then [1, m] else []
    { b with seq, isMasterLimit := false }
  else if b.seq.isEmpty then b
  else
    let an := b.seq.getLast!
    if an = 1 then { b with seq := b.seq.dropLast }
    else
      let n : Int := b.seq.length
      let br1 := rightmostLess b.seq an
      if br1 = 0 then b
      else
        let L := n - br1
        if m = 0 then
          if an > 0 then
            { b with seq := b.seq.set (b.seq.length - 1) (an - 1) }
          else b
        else
          let M := m * L - 1
          { b with seq := expandLenEpsp b.seq b.p M }

def expandOmega (b : EpsOmegaSS) (m : GoogInt) : EpsOmegaSS :=
  if b.isMasterLimit then
    let seq := if m > 0 then [1, m] else []
    { b with seq, isMasterLimit := false }
  else if b.seq.isEmpty then b
  else
    let an := b.seq.getLast!
    if an = 1 then { b with seq := b.seq.dropLast }
    else
      let n : Int := b.seq.length
      let br1 := rightmostLess b.seq an
      if br1 = 0 then b
      else
        let L := n - br1
        if m = 0 then
          if an > 0 then
            { b with seq := b.seq.set (b.seq.length - 1) (an - 1) }
          else b
        else
          let M := m * L - 1
          { b with seq := expandLenOmega b.seq M }

/- Lexicographic sequence compare, master_limit is supremum. -/
def compareSeq (seqa seqb : List GoogInt) (mla mlb : Bool) : Int :=
  if mla ∨ mlb then
    match mla, mlb with | true, true => 0 | true, false => 1 | false, true => -1
  else
    let n1 := seqa.length; let n2 := seqb.length; let m := min n1 n2
    let rec loop (i : Nat) : Int :=
      if i ≥ m then
        if n1 > n2 then 1 else if n1 < n2 then -1 else 0
      else
        let x := seqa.getD i 0; let y := seqb.getD i 0
        if x > y then 1 else if x < y then -1 else loop (i + 1)
    loop 0

end CommonEps

/-! EpspSS API + Notation instance. -/
namespace EpspSS

def fromString (p : GoogInt) (s : String) : EpspSS :=
  { seq := CommonEps.parseSeq s, p }

def toString (b : EpspSS) : String :=
  if b.isMasterLimit then "(1, ω)" else CommonEps.toString b.seq

def toLatex := toString

def expand (b : EpspSS) (m : GoogInt) : EpspSS := CommonEps.expandEpsp b m
def expandTo (b : EpspSS) (M : GoogInt) : EpspSS :=
  { b with seq := CommonEps.expandLenEpsp b.seq b.p M }

def index (b : EpspSS) (n : GoogInt) : EpspSS := expand b n

def isSuccessor (b : EpspSS) : Bool :=
  if b.isMasterLimit then false
  else match b.seq.last? with | some 1 => true | _ => false

def compare (a b : EpspSS) : Int :=
  CommonEps.compareSeq a.seq b.seq a.isMasterLimit b.isMasterLimit

def normalize (b : EpspSS) : EpspSS := b

def limit (p : GoogInt) (n : GoogInt) : EpspSS :=
  let seq := if n > 0 then [1, n] else []
  { seq, p }

def masterLimit (p : GoogInt := 1) : EpspSS := { p, isMasterLimit := true }

end EpspSS

instance : Notation EpspSS where
  name _ := "epsilon_p_ss"
  family _ := Family.ordinal
  subfamily _ := "sequence"
  style _ := "difference"
  creator _ := "zahin"
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

  stringToIt b s := EpspSS.fromString b.p s
  toLatex b := b.toLatex
  expand b n := b.expand n
  expandTo b len := b.expandTo len
  compare a o := a.compare o

/-! EpsOmegaSS API + Notation instance. -/
namespace EpsOmegaSS

def fromString (s : String) : EpsOmegaSS :=
  { seq := CommonEps.parseSeq s }

def toString (b : EpsOmegaSS) : String :=
  if b.isMasterLimit then "(1, ω)" else CommonEps.toString b.seq

def toLatex := toString

def expand (b : EpsOmegaSS) (m : GoogInt) : EpsOmegaSS := CommonEps.expandOmega b m
def expandTo (b : EpsOmegaSS) (M : GoogInt) : EpsOmegaSS :=
  { b with seq := CommonEps.expandLenOmega b.seq M }

def index (b : EpsOmegaSS) (n : GoogInt) : EpsOmegaSS := expand b n

def isSuccessor (b : EpsOmegaSS) : Bool :=
  if b.isMasterLimit then false
  else match b.seq.last? with | some 1 => true | _ => false

def compare (a b : EpsOmegaSS) : Int :=
  CommonEps.compareSeq a.seq b.seq a.isMasterLimit b.isMasterLimit

def normalize (b : EpsOmegaSS) : EpsOmegaSS := b

def limit (n : GoogInt) : EpsOmegaSS :=
  let seq := if n > 0 then [1, n] else []
  { seq }

def masterLimit : EpsOmegaSS := { isMasterLimit := true }

end EpsOmegaSS

instance : Notation EpsOmegaSS where
  name _ := "epsilon_omega_ss"
  family _ := Family.ordinal
  subfamily _ := "sequence"
  style _ := "difference"
  creator _ := "zahin"
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

  stringToIt _ s := EpsOmegaSS.fromString s
  toLatex b := b.toLatex
  expand b n := b.expand n
  expandTo b len := b.expandTo len
  compare a o := a.compare o

end Googology