/-
Knuth up-arrow notation (高德纳箭头).

  a ^c b = a ^ b                 (c = 1)
  a ^c b = a                     (b = 1)
  a ^c b = a ^(c-1) (a ^c (b-1)) (c > 1, b > 1)
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation

namespace Googology

open Capabilities

/-- One node of a Knuth up-arrow expression AST. -/
inductive KNode : Type where
  | value : GoogInt → KNode
  | arrow : (base : GoogInt) → (height : GoogInt) → (exp : KNode) → KNode
  deriving Repr

instance : Inhabited KNode := ⟨KNode.value 0⟩

namespace KNode

/-- Render a Knuth AST node as LaTeX. -/
def tex : KNode → String
  | .value v => toString v
  | .arrow base height exp =>
    let bt := toString base
    let et := tex exp
    let arr := if height = 1 then "\\uparrow" else "\\uparrow^{" ++ height.repr ++ "}"
    match exp with
    | .arrow _ _ _ => s!"{bt} {arr} ({et})"
    | _ => s!"{bt} {arr} {et}"

/-- Single expansion step on a Knuth AST node. -/
def step : KNode → KNode
  | .value v => .value v
  | .arrow base height (.value b) =>
    if b = 1 then .value base
    else if height = 1 then .arrow base height (.value b)
    else
      let inner : KNode := .arrow base height (.value (b - 1))
      .arrow base (height - 1) inner
  | .arrow base height exp => .arrow base height (step exp)

/-- Parse a string into a Knuth AST node. Partial: recursion over a
    strictly shorter suffix terminates at runtime (no termination proof). -/
partial def parse (s : String) : KNode :=
  let s := (String.trimAscii s).toString
  let posIdx := s.toList.findIdx (· = '^')
  if posIdx < s.length then
    let a := (s.take posIdx).toInt?.getD 0
    let rest := (s.drop posIdx).toString
    let (c, rest') := countArrows rest
    let rest'' := (String.trimAscii rest').toString
    if rest''.isEmpty then .value 0 -- fallback
    else .arrow a c (parse rest'')
  else
    .value (s.toInt?.getD 0)
where
  countArrows (s : String) : GoogInt × String :=
    let rec go (cnt : GoogInt) (cs : List Char) : GoogInt × String :=
      match cs with
      | [] => (cnt, "")
      | c :: rest =>
        if c = '^' then go (cnt + 1) rest
        else (cnt, String.ofList rest)
    go 0 s.toList

end KNode

/-- Knuth up-arrow notation. -/
structure Knuth where
  root : KNode := .value 0
  deriving Repr

namespace Knuth

/-- Create a Knuth notation from a string. -/
def fromString (s : String) : Knuth :=
  let t := s.replace "↑" "^" |>.replace " " ""
  if t.isEmpty then { root := .value 0 }
  else { root := KNode.parse t }

/-- Create an empty Knuth notation. -/
def new : Knuth := { root := .value 0 }

end Knuth

instance : Notation Knuth where
  name _ := "knuth"
  family _ := Family.number
  creator _ := "Donald Knuth"
  version _ := "1"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)

  stringToIt k s := Knuth.fromString s
  toLatex k := k.root.tex

  expand k n :=
    let rec expandGo (node : KNode) (i : Nat) : KNode :=
      if i = 0 then node
      else expandGo (node.step) (i - 1)
    { k with root := expandGo k.root n.toNat }

  expandTo k len :=
    let rec expandToGo (n : Nat) (cur : Knuth) (prev : String) : Knuth :=
      if prev.length ≥ len then cur
      else if n = 0 then cur
      else
        let next : Knuth := { root := cur.root.step }
        let curS := next.root.tex
        if curS = prev then next
        else expandToGo (n - 1) next curS
    expandToGo 1000 k (k.root.tex)

  reduce k := k  -- placeholder (Knuth step may not stabilize; no-op)

end Googology