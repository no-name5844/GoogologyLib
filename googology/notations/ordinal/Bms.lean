/-
Bashicu Matrix System (BMS, 波希久矩阵系统).

A BMS term is either:
  - a non-negative integer (level 0)
  - a matrix `(row0; row1; ...)` where each row is a list of column values

We implement the standard binary BMS (2-column) expansion:
  1. Empty matrix -> 0.
  2. Last row has col-0 = 0 -> drop last row.
  3. Otherwise:
       new_a = last_a - 1
       find rightmost parent row i < k with rows[i][0] < new_a (else [0,0])
       replace rows[k] with [new_a, last_b + 1]
       append parent before the new last row
       (copy rule: rowsWithoutLast ++ [parent] ++ [newLast])
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation

namespace Googology

open Capabilities

/-- A single matrix row = list of integer column values. -/
abbrev BmsRow := List GoogInt

/-- BMS AST node: integer or matrix. -/
inductive BmsNode : Type where
  | value  : GoogInt → BmsNode
  | matrix : List BmsRow → BmsNode
  deriving Repr

namespace BmsNode

/-- LaTeX for one row (columns separated by `&` for pmatrix). -/
def rowTex : BmsRow → String
  | [] => ""
  | [v] => toString v
  | v :: vs => s!"{toString v} & {rowTex vs}"

/-- LaTeX for rows separated by `\\`. -/
private def bodyTex : List BmsRow → String
  | [] => ""
  | [r] => rowTex r
  | r :: rs => s!"{rowTex r} \\\\ {bodyTex rs}"

/-- Top-level BMS LaTeX render. -/
def tex : BmsNode → String
  | .value v => toString v
  | .matrix [] => "0"
  | .matrix [[v]] => s!"({toString v})"
  | .matrix rows => s!"\\begin{{pmatrix}} {bodyTex rows} \\end{{pmatrix}}"

/- Expansion helpers --------------------------------------------------- -/

/-- Number of columns = max row length. -/
def colCount : List BmsRow → Nat
  | [] => 0
  | r :: rs => max r.length (colCount rs)

/-- Zero-pad row to `n` columns. -/
private def padTo (row : BmsRow) (n : Nat) : BmsRow :=
  row ++ List.replicate (n - row.length) 0

/-- Get j-th column value of a row (padded). -/
private def colAt (row : BmsRow) (n : Nat) (j : Nat) : GoogInt :=
  match (padTo row n).get? j with | some v => v | none => 0

/-- Find rightmost parent index i < lastIdx such that rows[i][0] < target.
    Returns `none` if none → use [0,0] as the default parent. -/
private def findParent (rows : List BmsRow) (nCols : Nat) (lastIdx : Nat) (target : GoogInt)
    : Option Nat :=
  let rec go (i : Nat) (best : Option Nat) : Option Nat :=
    if i ≥ lastIdx then best
    else
      if colAt (rows.getD i []) nCols 0 < target
        then go (i + 1) (some i)
        else go (i + 1) best
  go 0 none

/-- Build a new last row using the parent and new (a-1, b+1) values for the
    first two columns, copying parent values for any remaining columns. -/
private def buildNewLast (parent : BmsRow) (last : BmsRow) (newA : GoogInt) (newB : GoogInt)
    (nCols : Nat) : BmsRow :=
  let p := padTo parent nCols
  let rec loop (j : Nat) (acc : BmsRow) : BmsRow :=
    if j ≥ nCols then acc.reverse
    else
      let v : GoogInt :=
        if j = 0 then newA
        else if j = 1 then newB
        else colAt p nCols j
      loop (j + 1) (v :: acc)
  loop 0 []

/-- Core step: expand a list of rows by one rule application. -/
def stepRows (rows : List BmsRow) : List BmsRow :=
  match rows with
  | [] => []
  | [_] =>
    -- Single row: if col 0 > 0 we just decrement it; if col 0 = 0 and has
    -- col 1 > 0 we also decrement it; otherwise -> empty
    let nCols := colCount rows
    let r := padTo (rows.getD 0 []) nCols
    let a0 := colAt r nCols 0
    let b0 := colAt r nCols 1
    if a0 > 0 then
      [buildNewLast [] r (a0 - 1) b0 nCols]
    else if b0 > 0 then
      [buildNewLast [] r 0 (b0 - 1) nCols]
    else
      []
  | _ =>
    let nCols := colCount rows
    let lastIdx := rows.length - 1
    let lastRow := rows.getD lastIdx []
    let a_k := colAt lastRow nCols 0
    if a_k = 0 then
      -- Rule 2: last row's a is 0 → drop
      rows.dropLast
    else
      let newA := a_k - 1
      let b_k := colAt lastRow nCols 1
      let parentIdx := findParent rows nCols lastIdx newA
      let parent : BmsRow :=
        match parentIdx with
        | some i => rows.getD i []
        | none => []
      let newLast := buildNewLast parent lastRow newA (b_k + 1) nCols
      rows.dropLast ++ [parent] ++ [newLast]

/-- One expansion step at the node level. -/
def step : BmsNode → BmsNode
  | .value v => .value v
  | .matrix rows =>
    match stepRows rows with
    | [] => .value 0
    | rs => .matrix rs

end BmsNode

/-- BMS notation wrapper. -/
structure Bms where
  root : BmsNode := .value 0
  deriving Repr

namespace Bms

/--
  Parse a BMS string. Accepted forms:
    - "0", "123"          → integer value node
    - "(1,2)"             → single row
    - "(0,0; 1,2; 3,4)"   → rows separated by `;`, cols by `,`
-/
def fromString (s : String) : Bms :=
  let t := s.trim
  if t.isEmpty then { root := .value 0 }
  else if t.startsWith "(" && t.endsWith ")" then
    let inner := t.drop 1 |>.dropRight 1
    let rowsStr := inner.splitOn ";"
    let rows : List BmsRow :=
      rowsStr.filterMap fun rs =>
        let cols := (rs.splitOn "," |>.map fun c => (c.trim.toInt!.getD 0))
        if cols.isEmpty then none else some cols
    { root := .matrix rows }
  else
    { root := .value (t.toInt!.getD 0) }

/-- Create an empty BMS notation. -/
def new : Bms := { root := .value 0 }

end Bms

instance : Notation Bms where
  name _ := "bms"
  family _ := Family.ordinal
  creator _ := "Bashicu"
  version _ := "1"
  subfamily _ := "binary"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)

  stringToIt _ s := Bms.fromString s
  toLatex b := b.root.tex

  expand b n :=
    let rec go (node : BmsNode) (i : GoogInt) : BmsNode :=
      if i ≤ 0 then node
      else go (node.step) (i - 1)
    { b with root := go b.root n }

  expandTo b len :=
    let rec go (cur : Bms) (prev : String) : Bms :=
      if prev.length ≥ len then cur
      else
        let next : Bms := { root := cur.root.step }
        let curS := next.root.tex
        if curS = prev then next
        else go next curS
    go b (b.root.tex)

end Googology