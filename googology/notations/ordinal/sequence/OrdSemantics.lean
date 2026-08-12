/-
PrSS / PPS 的序数记号语义实例（接轨 core/OrdinalSemantics.lean 的 ordinalOf）。
每个实例提供：0 判定（非主极限的空序列）、后继判定（末项为 0）、
前驱（去末项）、基本列（expand）。 -/

import Googology.core.OrdinalSemantics
import Googology.notations.ordinal.sequence.Prss
import Googology.notations.ordinal.sequence.Pps

namespace Googology

/-- PrSS 的语义接口：空序列（非主极限）= 0；末项 0 = 后继（前驱 = 去末项）；
    极限 = 基本列展开。 -/
instance : HasOrdSemantics Prss where
  isZero b := ¬b.isMasterLimit ∧ b.seq.isEmpty
  isSuccessor b := Prss.isSuccessor b
  predecessor b :=
    if b.isSuccessor then some { b with seq := b.seq.dropLast } else none
  expand b n := Prss.expand b (n : GoogInt)

/-- PrSS 表达式的语义序数。 -/
noncomputable def Prss.ord (b : Prss) : MOrdinal := ordinalOf b

/-- PPS 的语义接口（全部变体共用同一类型）。 -/
instance : HasOrdSemantics Pps where
  isZero b := ¬b.isMasterLimit ∧ b.seq.isEmpty
  isSuccessor b := Pps.isSuccessor b
  predecessor b :=
    if b.isSuccessor then some { b with seq := b.seq.dropLast } else none
  expand b n := Pps.expand b (n : GoogInt)

/-- PPS 表达式的语义序数。 -/
noncomputable def Pps.ord (b : Pps) : MOrdinal := ordinalOf b

end Googology
