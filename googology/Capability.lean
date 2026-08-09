import Googology.Basic

namespace Googology

/-- Operations a notation may support (capability model). -/
inductive Op : Type where
  | fromString
  | toString
  | normalize
  | compare
  | expand
  | expandTo
  | successor
  deriving DecidableEq, Repr

/-- Bitset of supported operations. -/
structure Capabilities where
  bits : UInt8 := 0
  deriving Repr

namespace Capabilities

/-- Create empty capabilities. -/
def empty : Capabilities := { bits := 0 }

/-- Set a capability bit. -/
def set (c : Capabilities) (op : Op) : Capabilities :=
  let bit : UInt8 := 1 << match op with
    | Op.fromString => 0
    | Op.toString => 1
    | Op.normalize => 2
    | Op.compare => 3
    | Op.expand => 4
    | Op.expandTo => 5
    | Op.successor => 6
  { c with bits := c.bits ||| bit }

/-- Check if a capability is supported. -/
def has (c : Capabilities) (op : Op) : Bool :=
  let bit : UInt8 := 1 << match op with
    | Op.fromString => 0
    | Op.toString => 1
    | Op.normalize => 2
    | Op.compare => 3
    | Op.expand => 4
    | Op.expandTo => 5
    | Op.successor => 6
  (c.bits &&& bit) != 0

instance : ToString Capabilities where
  toString c := s!"Capabilities({c.bits})"

end Capabilities

end Googology