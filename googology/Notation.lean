import Googology.Basic
import Googology.Capability

namespace Googology

/-- Top-level taxonomy of notations. -/
inductive Family : Type where
  | ordinal
  | number
  | hierarchy
  | realSequence
  deriving DecidableEq, Repr

/-- Exception for unsupported operations. -/
structure UnsupportedOpException where
  name : String
  op : Op
  deriving Repr

instance : ToString UnsupportedOpException where
  toString e := s!"Notation '{e.name}' does not implement the requested operation"

/-- Exception for non-comparable notations (large-number notations). -/
structure NotComparableException where
  name : String
  deriving Repr

instance : ToString NotComparableException where
  toString e := s!"Notation '{e.name}': comparison is undefined for large-number notations"

/-- Generic notation interface. Every notation implements this trait. -/
class Notation (α : Type) where
  -- taxonomy
  name : α → String
  family : α → Family
  subfamily : α → String := λ _ => ""
  style : α → String := λ _ => ""

  -- attribution
  creator : α → String := λ _ => ""
  version : α → String := λ _ => ""

  -- capability
  capabilities : α → Capabilities
  can (a : α) (op : Op) : Bool := (capabilities a).has op

  -- string conversion
  stringToIt : α → String → α
  toLatex : α → String

  -- operations
  compare : α → α → Int := λ _ _ => -1
  expand : α → GoogInt → α
  expandTo : α → GoogInt → α

  comparable (a : α) : Bool := (capabilities a).has Op.compare

  reduce (a : α) : α :=
    let rec go (n : Nat) (cur : α) (prev : String) : α :=
      match n with
      | 0 => cur
      | n + 1 =>
        let next := expand cur 1
        let curS := toLatex next
        if curS = prev then next
        else go n next curS
    go 1000 a (toLatex a)

end Googology