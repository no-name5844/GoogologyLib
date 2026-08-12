/-
BMS (Bashicu Matrix System) — fully based on the C++ implementation in
include/googology/notations/ordinal/matrix/bms/BMS.hpp and
src/notations/ordinal/matrix/bms/BMS.cpp.

Storage
-------
Column-major. S_{x,y} = column x, row y.

struct BMS (virtual hook based):
  cols_ : List (List GoogInt)   — cols[x] = column x values, top row y=0 first
  is_master_limit_ : Bool       — for the shared (0)(1,1,1,…) limit expression

Subclasses (BM4 default, BM1 / BM3_3 override hooks):
  parentOf        (k m : Nat) : Int   — greatest p < m with S[p,k] < S[m,k]
                                             (BM4 / BM3_3: AND ascension check)
  ascensionDegree (k m t : Nat) : Bool — ∃ c ≥ 0 : (p_k)^c (m) = t

Expand skeleton (single step only; matches C++ `expand(n)` = FS_n(S) =
G + B^(0) + … + B^(n)):
  expand((), n)   = ()                → value 0
  expand(S+(0),n) = drop last column  → successor's predecessor
  expand(S, n)    when last column has no LNZ-parent (r<0):
                    decrement LNZ n times in place (and peel column if 0)
  expand(S, n)    otherwise: G + B^(0) ++ … ++ B^(n)
                    B^(i)_m,k = B_m,k + a_{k,r+m}(r) * Δ_k * i

Parsing: "(a,b,c)(d,e,f)…" — each (…) is one COLUMN, comma-separated rows.

Output to_string uses "valHeight" (trim trailing zeros per column, keep at
least 1 so the zero vector renders as "(0)").

Limit expressions (shared):
  limit(0) = ""                        = ()
  limit(1) = "(0)"                     = (0)
  limit(2) = "(0)(1)"                  = (0)(1)
  limit(3) = "(0)(1,1)"                = (0)(1,1)
  …
  master_limit = (0)(1,1,1,…) marked by is_master_limit flag;
                 master_limit.expand(m) == limit(m).
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation

namespace Googology

open Capabilities

/-! Variants (dispatch tag, same role as C++ virtual overrides). -/
inductive BmsKind where
  | bm4
  | bm1
  | bm33
  deriving DecidableEq, Repr

/-! Column-major BMS state. -/
structure BMS where
  cols : List (List GoogInt)          -- column-major matrix
  isMasterLimit : Bool := false
  kind : BmsKind := BmsKind.bm4       -- default BM4, overridden per subtype
  deriving Repr

/-! C++ helpers -------------------------------------------------------- -/
namespace BMS

/- Total number of columns. -/
@[inline] def numCols (b : BMS) : Nat := b.cols.length

/- Raw height of column x; out-of-bounds -> 0. -/
def colHeight (b : BMS) (x : Nat) : Nat :=
  match b.cols.get? x with | some c => c.length | none => 0

/- S_{x,y}; out-of-bounds -> 0. -/
def get (b : BMS) (x y : Nat) : GoogInt :=
  match b.cols.get? x with
  | none => 0
  | some col => match col.get? y with | some v => v | none => 0

/- Value height: drop trailing zeros, minimum 1 so a pure zero column still
    renders as "(0)". -/
def valHeight (b : BMS) (x : Nat) : Nat :=
  let h := b.colHeight x
  if h = 0 then 0
  else
    let rec loop (j : Nat) : Nat :=
      if j ≥ h then 1
      else
        let idx := h - 1 - j
        if b.get x idx ≠ 0 then idx + 1
        else loop (j + 1)
    loop 0

/- Last non-zero row y = max { y | S_{X-1, y} > 0 } in last column.
    Returns -1 if none. -/
def lnzRow (b : BMS) : Int :=
  let X := b.numCols
  if X = 0 then -1
  else
    let h := b.colHeight (X - 1)
    let rec loop (y : Nat) (best : Int) : Int :=
      if y ≥ h then best
      else
        if b.get (X - 1) y > 0 then loop (y + 1) y
        else loop (y + 1) best
    loop 0 (-1)

/- Does the last column consist entirely of zeros? -/
def lastColAllZero (b : BMS) : Bool := b.lnzRow < 0

/-! ===== Variant hooks (BM4 default, BM1/BM3_3 override) ===== -/

/-- Generic parentOf implementation. BM4 rule:
      p_k(m) = max p<m with S[p,k] < S[m,k] AND (k=0 OR ascensionDegree(k-1,m,p))
    BM1: drop the "ascensionDegree(k-1,m,p)" guard.
    BM3_3: same as BM4 (parentOf not overridden there; only ascensionDegree is).
-/
def parentOf (b : BMS) (k m : Nat) : Int :=
  let rec go (p : Nat) (best : Int) : Int :=
    if p ≥ m then best
    else
      if b.get p k < b.get m k then
        match b.kind with
        | BmsKind.bm1 => go (p + 1) p  -- BM1 ignores ancestor check
        | _ =>  -- BM4 / BM3_3 both do the standard ancestor check
          if k = 0 ∨ b.ascensionDegree (k - 1) m p then
            go (p + 1) p
          else go (p + 1) best
      else go (p + 1) best
  go 0 (-1)

/-- Generic ascensionDegree: does column m have t as an ancestor under its
    row-k parent chain (∃ c : (p_k)^c (m) = t)?
    BM4 default: iterate parentOf(k,·) walking the chain.
    BM1: ignore k, use row 0 uniformly (C++ line: `ascensionDegree(0,m,t)`).
    BM3_3: recursive clause a_{k,m}=1 ⇔ a_{k,parent(m)}=1 ∧ parent(m) > t,
           plus base case m==t → 1.
-/
partial def ascensionDegree (b : BMS) (k m t : Nat) : Bool :=
  match b.kind with
  | BmsKind.bm1 => b.ascensionDegreeBM4 0 m t   -- unified by row 0
  | BmsKind.bm33 =>
    if m = t then true
    else
      let p := b.parentOf k m
      if p < 0 ∨ p.toNat = m then false
      else
        let pn := p.toNat
        b.ascensionDegreeBM4 k pn t ∧ pn > t   -- note: t here is bad root r
  | BmsKind.bm4 => b.ascensionDegreeBM4 k m t
where
  ascensionDegreeBM4 (k m t : Nat) : Bool :=
    if m = t then true
    else
      let rec loop (cur : Nat) (guard : Nat) : Bool :=
        if guard = 0 then false
        else
          let p := b.parentOf k cur
          if p < 0 ∨ p.toNat = cur then false
          else
            let pn := p.toNat
            if pn = t then true else loop pn (guard - 1)
      loop m 100000

/- Bad root r = p_z(X-1) where z = lnzRow_; -1 when no lnz. -/
def badRoot (b : BMS) : Int :=
  let z := b.lnzRow
  if z < 0 then -1
  else
    let X := b.numCols
    if X = 0 then -1
    else b.parentOf z.toNat (X - 1)

/- Δ_k = S_{X-1,k} - S_{r,k} for k<z; 0 otherwise. -/
def delta (b : BMS) (k : Nat) : GoogInt :=
  let z := b.lnzRow
  if z < 0 ∨ k ≥ z.toNat then 0
  else
    let r := b.badRoot
    let X := b.numCols
    if r < 0 then 0
    else b.get (X - 1) k - b.get r.toNat k

/-- The i-th copy of the m-th bad part column (0-indexed).
    B^(i)_{m,k} = B_{m,k} + a_{k, r+m}(r) * Δ_k * i.
    Here m ∈ [0, Bcols), original column index = r + m, r = badRoot.
-/
def copyColumn (b : BMS) (r : Nat) (m i : Nat) : List GoogInt :=
  let orig := r + m
  let hh := b.colHeight orig
  let rec loop (k : Nat) (acc : List GoogInt) : List GoogInt :=
    if k ≥ hh then acc.reverse
    else
      let a : GoogInt := if b.ascensionDegree k orig r then 1 else 0
      let v := b.get orig k + a * (b.delta k) * (i : GoogInt)
      loop (k + 1) (v :: acc)
  loop 0 []

/-- Resize every column to the same height = max row count, padding bottom
    with zeros. Mirror of C++ `for (auto& c : cols_) c.resize(maxH, 0);`. -/
def padToRect (cols : List (List GoogInt)) : List (List GoogInt) :=
  let maxH := cols.foldl (fun mx c => max mx c.length) 0
  cols.map fun c => c ++ List.replicate (maxH - c.length) 0

/-! Expand skeleton (C++ §0.2.2): produce FS_n(S). -/
private def expandToFS (b : BMS) (n : GoogInt) : BMS :=
  let X := b.numCols
  if X = 0 then
    -- expand((), n) = () treated as 0 value; cols cleared.
    { b with cols := [] }
  else if b.lastColAllZero then
    -- expand(S+(0), n) = predecessor = drop last column, stop
    { b with cols := b.cols.dropLast }
  else
    let z := b.lnzRow.toNat  -- safe: since lastColAllZero=false, lnzRow ≥0
    let r := b.badRoot
    if r < 0 then
      -- No parent (e.g. single column (5)): decrement LNZ entry n times in
      -- place, and peel once the last column becomes all zero.
      let rec loop (cols : List (List GoogInt)) (remaining : GoogInt)
                  : List (List GoogInt) :=
        if remaining ≤ 0 then cols
        else
          let X' := cols.length
          if X' = 0 then cols
          else
            -- recalc local lnzRow
            let lastCol := cols.get! (X' - 1)
            let findLnz (c : List GoogInt) : Int :=
              c.enum.foldl (init := -1) fun best (y, v) =>
                if v > 0 then y else best
            let z' := findLnz lastCol
            if z' < 0 then cols
            else
              let newLastCol : List GoogInt :=
                lastCol.modifyN z'.toNat (fun v => v - 1)
              -- check column now all zeros?
              let nowZero := newLastCol.all (· = 0)
              let cols2 :=
                if nowZero then
                  cols.take (X' - 1)
                else
                  cols.set (X' - 1) newLastCol
              loop cols2 (remaining - 1)
      { b with cols := loop b.cols n }
    else
      -- Proper FS_n(S) = G ++ concat_i=0..n (B^(i)), n+1 bad blocks.
      let rN := r.toNat
      let G : List (List GoogInt) := b.cols.take rN
      let Bcols : Nat := (X - 1) - rN
      let nb : GoogInt := n + 1
      let rec buildBad (i : GoogInt) (acc : List (List GoogInt))
                     : List (List GoogInt) :=
        if i ≥ nb then acc.reverse
        else
          let iN : Nat := i.toNat
          let rec buildCol (m : Nat) (colsAcc : List (List GoogInt))
                          : List (List GoogInt) :=
            if m ≥ Bcols then colsAcc
            else
              let col := b.copyColumn rN m iN
              buildCol (m + 1) (col :: colsAcc)
          let newBlock := buildCol 0 []
          buildBad (i + 1) (newBlock.reverse ++ acc)
      let newCols := G ++ buildBad 0 []
      { b with cols := padToRect newCols }

/-! Limit expressions (§0.3, version-agnostic, BM4-kind under the hood).-/
def limit (n : GoogInt) : BMS :=
  let mk : BMS := { kind := BmsKind.bm4, isMasterLimit := false, cols := [] }
  if n ≤ 0 then mk
  else
    let col0 : List GoogInt := [0]
    if n = 1 then { mk with cols := [col0] }
    else
      -- col1: (n-1) copies of 1
      let rec buildCol1 (j : GoogInt) (acc : List GoogInt) : List GoogInt :=
        if j ≥ n - 1 then acc.reverse
        else buildCol1 (j + 1) (1 :: acc)
      { mk with cols := padToRect [col0, buildCol1 0 []] }

def masterLimit : BMS :=
  { kind := BmsKind.bm4, isMasterLimit := true, cols := [] }

/-! to_string / LaTeX (value height trimmed per column). -/
def toString (b : BMS) : String :=
  if b.isMasterLimit then "(0)(1,1,1,…)"
  else
    let rec colStr (x : Nat) : String :=
      match b.cols.get? x with
      | none => ""
      | some c =>
        let h := b.valHeight x
        let rec rowsStr (y : Nat) (acc : String) : String :=
          if y ≥ h then acc
          else
            let sep := if y = 0 then "" else ","
            rowsStr (y + 1) (acc ++ sep ++ toString (b.get x y))
        s!"({rowsStr 0 ""})"
    let rec loop (x : Nat) (acc : String) : String :=
      if x ≥ b.numCols then acc
      else loop (x + 1) (acc ++ colStr x)
    loop 0 ""

def toLatex (b : BMS) : String :=
  if b.numCols = 0 then "0"
  else
    -- Render as pmatrix: every column of the matrix is rendered as a pmatrix
    -- column, using rows y=0..h-1 top to bottom. This matches C++ to_string
    -- semantics but wrapped in math-mode pmatrix cells for each column.
    let rec renderCol (col : List GoogInt) : String :=
      match col with
      | [] => ""
      | [v] => toString v
      | v :: vs => s!"{toString v} \\\\ {renderCol vs}"
    let colLatex (c : List GoogInt) : String :=
      if c.length = 1 then renderCol c
      else s!"\\begin{{pmatrix}} {renderCol c} \\end{{pmatrix}}"
    let rec loop (x : Nat) (acc : String) : String :=
      if x ≥ b.numCols then acc
      else
        match b.cols.get? x with
        | none => acc
        | some cRaw =>
          let h := b.valHeight x
          let c := cRaw.take h
          loop (x + 1) (acc ++ colLatex c)
    loop 0 ""

/-! Parsing: "(a,b)(c,d,e)…"  -> columns, resize to rect. -/
def stringToIt (s : String) : BMS :=
  let rec parseCol (inner : Substring) : List GoogInt :=
    if inner.isEmpty then []
    else
      let parts := inner.toString.splitOn ","
      parts.filterMap fun p =>
        let trimmed := p.trim
        if trimmed.isEmpty then none
        else some (trimmed.toInt!.getD 0)
  let rec parse (t : Substring) (acc : List (List GoogInt)) : List (List GoogInt) :=
    if t.isEmpty then acc.reverse
    else
      let t' := t.dropWhile (· = ' ')
      if not (t'.startsWith "(") then acc.reverse
      else
        let rest := (t'.drop 1)
        match rest.find (· = ')') with
        | none => acc.reverse
        | some j =>
          let inner := rest.extract 0 j
          let after := rest.extract (j + 1) rest.endPos
          let col := parseCol inner
          parse after (col :: acc)
  let t := (s.replace " " "").toSubstring
  { cols := padToRect (parse t []), isMasterLimit := false, kind := BmsKind.bm4 }

/-! Compare (column-major lex order on valHeight-trimmed). -/
def compare (a b : BMS) : Int :=
  if a.isMasterLimit ∨ b.isMasterLimit then
    match a.isMasterLimit, b.isMasterLimit with
    | true, true => 0
    | true, false => 1
    | false, true => -1
  else
    let X := max a.numCols b.numCols
    let rec colLoop (x : Nat) : Int :=
      if x ≥ X then
        if a.numCols ≠ b.numCols
        then (if a.numCols < b.numCols then -1 else 1)
        else 0
      else
        let h1 := a.valHeight x
        let h2 := b.valHeight x
        let h := max h1 h2
        let rec rowLoop (y : Nat) : Int :=
          if y ≥ h then colLoop (x + 1)
          else
            let av := if y < h1 then a.get x y else 0
            let bv := if y < h2 then b.get x y else 0
            if av ≠ bv then (if av < bv then -1 else 1)
            else rowLoop (y + 1)
        rowLoop 0
    colLoop 0

/-! Core expand(n). -/
def expand (b : BMS) (n : GoogInt) : BMS :=
  if b.isMasterLimit then
    -- master_limit.expand(m) == limit(m)
    limit n
  else
    expandToFS b n

/-! expandTo (repeat step until length ≥ len; matches other notations). -/
def expandTo (b : BMS) (len : GoogInt) : BMS :=
  let rec go (cur : BMS) (prev : String) : BMS :=
    if prev.length ≥ len.toNat then cur
    else
      let next := expandToFS cur 0
      let curS := next.toString
      if curS = prev then next
      else go next curS
  go b b.toString

/-! §0.1 legality check (purely syntactic; legal ≠ standard). -/
def isLegal (b : BMS) : Bool :=
  let X := b.numCols
  if X = 0 then true
  else
    -- 1) column 0 all zero
    let h0 := b.colHeight 0
    let cond1 : Bool := Id.run do
      for y in List.range h0 do
        if b.get 0 y ≠ 0 then return false
      return true
    if not cond1 then false
    else
      -- 2) each column non-increasing top to bottom
      let cond2 : Bool := Id.run do
        for x in List.range X do
          let h := b.colHeight x
          for y in List.range (h - 1) do
            if b.get x y < b.get x (y + 1) then return false
        return true
      if not cond2 then false
      else
        -- 3) S[x,y] <= max_{p<x} S[p,y] + 1
        let cond3 : Bool := Id.run do
          for x in List.range (X - 1) do
            let xp1 := x + 1
            let h := b.colHeight xp1
            for y in List.range h do
              let mut mx : GoogInt := 0
              for p in List.range xp1 do
                let v := b.get p y
                if v > mx then mx := v
              if b.get xp1 y > mx + 1 then return false
          return true
        cond3

def normalize (b : BMS) : BMS :=
  if b.isMasterLimit then b
  else if b.isLegal then b
  else b  -- keep unchanged per library convention (C++ line 290)

def isSuccessor (_b : BMS) : Bool := false

/-! Create a BMS of a specific kind by casting from BM4-parsed form. -/
def withKind (b : BMS) (k : BmsKind) : BMS :=
  { b with kind := k }

end BMS

/-! Concrete notation wrappers: BM4, BM1, BM3_3 ------------------------ -/

/-- BM4 (2018-09-01 standard). -/
def BM4 := BMS
/-- BM1 (2014 original). -/
def BM1 := BMS
/-- BM3.3 (2019 community). -/
def BM3_3 := BMS

/-! Public constructors with correct kind tag. -/
namespace BM4
  def ofString (s : String) : BM4 := (BMS.stringToIt s).withKind BmsKind.bm4
end BM4
namespace BM1
  def ofString (s : String) : BM1 := (BMS.stringToIt s).withKind BmsKind.bm1
end BM1
namespace BM3_3
  def ofString (s : String) : BM3_3 := (BMS.stringToIt s).withKind BmsKind.bm33
end BM3_3

/-! Notation instances (BM4 default; BM1 and BM3_3 override name). -/
instance : Notation BMS where
  name _ := "bm4"
  family _ := Family.ordinal
  subfamily _ := "bms"
  style _ := "matrix"
  creator _ := "Bashicu"
  version _ := "2018-09-01"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)
      |> (·.set Op.compare)
      |> (·.set Op.normalize)

  stringToIt b s := BMS.stringToIt s
  toLatex b := b.toLatex
  expand b n := b.expand n
  expandTo b len := b.expandTo len
  compare a other := a.compare other

instance : Notation BM1 where
  name _ := "bm1"
  family _ := Family.ordinal
  subfamily _ := "bms"
  style _ := "matrix"
  creator _ := "Bashicu"
  version _ := "2014"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)
      |> (·.set Op.compare)
      |> (·.set Op.normalize)

  stringToIt _ s := BM1.ofString s
  toLatex b := b.toLatex
  expand b n := b.expand n
  expandTo b len := b.expandTo len
  compare a other := a.compare other

instance : Notation BM3_3 where
  name _ := "bm3.3"
  family _ := Family.ordinal
  subfamily _ := "bms"
  style _ := "matrix"
  creator _ := "Rpakr + ecl1psed"
  version _ := "2019-03"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)
      |> (·.set Op.compare)
      |> (·.set Op.normalize)

  stringToIt _ s := BM3_3.ofString s
  toLatex b := b.toLatex
  expand b n := b.expand n
  expandTo b len := b.expandTo len
  compare a other := a.compare other

end Googology
