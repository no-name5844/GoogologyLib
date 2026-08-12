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

/-- Single-bit mask for an operation. -/
def bitOf (op : Op) : UInt8 :=
  match op with
  | Op.fromString => 1
  | Op.toString => 2
  | Op.normalize => 4
  | Op.compare => 8
  | Op.expand => 16
  | Op.expandTo => 32
  | Op.successor => 64

/-- Set a capability bit. -/
def set (c : Capabilities) (op : Op) : Capabilities :=
  { c with bits := c.bits ||| bitOf op }

/-- Check if a capability is supported. -/
def has (c : Capabilities) (op : Op) : Bool :=
  (c.bits &&& bitOf op) != 0

instance : ToString Capabilities where
  toString c := s!"Capabilities({c.bits})"

end Capabilities

end Googology