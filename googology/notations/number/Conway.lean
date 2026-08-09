/-
Conway chained-arrow notation (康威链式箭头).

Rules:
  1. [a -> b]                = a ^ b
  2. X -> 1 -> Y             = X
  3. X -> a -> b             = X -> (X -> a-1 -> b) -> b-1
-/

import Googology.Basic
import Googology.Capability
import Googology.Notation

namespace Googology

open Capabilities

/-- A chain element: either an integer leaf, or a nested sub-chain. -/
inductive CNode : Type where
  | value : GoogInt → CNode
  | subChain : List CNode → CNode
  deriving Repr

/-- Conway chained-arrow notation. -/
structure Conway where
  chain : List CNode := []
  deriving Repr

namespace Conway

/-- Render a chain as LaTeX. -/
def ser (ch : List CNode) : String :=
  match ch with
  | [.value a, .value b] => s!"{a}^{{{b}}}"
  | _ =>
    let parts := ch.map fun node =>
      match node with
      | .value v => toString v
      | .subChain s => s!"({ser s})"
    String.intercalate " \\rightarrow " parts

/-- Single expansion step on a chain. -/
def stepOnce (ch : List CNode) : List CNode :=
  match ch with
  | [] => []
  | [_] => ch
  | [a, b] =>
    match a, b with
    | .value _, .value _ => ch  -- terminal: a -> b
    | _, _ => recurseSub ch
  | _ =>
    -- rule 2a: trailing 1
    if let some (.value 1) := ch.getLast? then
      ch.dropLast
    -- rule 2b: middle 1
    else if ch.length ≥ 2 then
      let secondLast := ch.get! (ch.length - 2)
      match secondLast with
      | .value 1 => ch.dropLast 2
      | _ =>
        -- rule 3: X -> a -> b = X -> (X -> a-1 -> b) -> b-1
        match ch.getLast?, secondLast with
        | some (.value b), .value a =>
          let x := ch.dropLast 2
          let inner := x ++ [.value (a - 1), .value b]
          x ++ [.subChain inner, .value (b - 1)]
        | _, _ => recurseSub ch
    else recurseSub ch

/-- Recurse into sub-chains looking for something to expand. -/
def recurseSub (ch : List CNode) : List CNode :=
  let rec go (acc : List CNode) (remaining : List CNode) (changed : Bool) : List CNode :=
    match remaining with
    | [] => if changed then acc.reverse else ch
    | (.subChain sub) :: rest =>
      let before := ser sub
      let s := stepOnce sub
      let after := ser s
      if after ≠ before then
        let newNode : CNode :=
          match s with
          | [.value v] => .value v
          | _ => .subChain s
        go (newNode :: acc) rest true
      else
        go (.subChain sub :: acc) rest changed
    | node :: rest => go (node :: acc) rest changed
  go [] ch false

/-- Create a Conway notation from a string. -/
def fromString (s : String) : Conway :=
  let t := s.replace "→" "->" |>.replace " " ""
  let parts := t.splitOn "->"
  { chain := parts.map (λ p => .value (p.toInt!.getD 0)) }

/-- Create an empty Conway notation. -/
def new : Conway := { chain := [] }

end Conway

instance : Notation Conway where
  name _ := "conway"
  family _ := Family.number
  creator _ := "John Conway"
  version _ := "1"

  capabilities _ :=
    Capabilities.empty
      |> (·.set Op.fromString)
      |> (·.set Op.toString)
      |> (·.set Op.expand)
      |> (·.set Op.expandTo)

  stringToIt c s := Conway.fromString s
  toLatex c := Conway.ser c.chain

  expand c n :=
    let rec go (ch : List CNode) (i : GoogInt) : List CNode :=
      if i ≤ 0 then ch
      else go (Conway.stepOnce ch) (i - 1)
    { c with chain := go c.chain n }

  expandTo c len :=
    let rec go (cur : Conway) (prev : String) : Conway :=
      if prev.length ≥ len then cur
      else
        let next : Conway := { chain := Conway.stepOnce cur.chain }
        let curS := Conway.ser next.chain
        if curS = prev then next
        else go next curS
    go c (Conway.ser c.chain)

end Googology