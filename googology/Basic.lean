/-
GoogologyLib — Multi-language Notation Collection Library (Lean4 branch)
-/

def googologyVersion : String := "1"

namespace Googology

/-- Integer type used throughout the library. Never used for numeric evaluation
of notations — only for parse parameters and expansion indices. -/
abbrev GoogInt := Int

end Googology