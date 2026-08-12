/-
序数记号语义（denotation）：记号表达式 → 它代表的序数。
基于 core/Ordinal.lean 的 MOrdinal（mathlib Ordinal）作语义值。

定义（三分支递归）：
  ord(0)       = 0
  ord(后继 A)  = ord(前驱 A) + 1
  ord(极限 A)  = sSup { ord(expand(A, n)) : n ∈ ℕ }
              （expand(A,n) 组成的基本列的确切上界 = A 所代表的序数）

“一律不求值”只适用于大数记号；序数记号有良定义语义，经此函数
接轨统一序数表示 MOrdinal。良基性依赖各记号的良序性（如 PrSS 良序
= ε₀、PPS 良序 = ζ₀），此处为非计算定义（noncomputable partial，
不证明终止）。
-/

import Googology.Basic
import Googology.core.Ordinal
import Mathlib.SetTheory.Ordinal.Basic

namespace Googology

/-- 序数记号语义接口：0 判定 / 后继判定 / 前驱 / 基本列展开。
    满足此接口的类型可定义语义序数（见 ordinalOf）。 -/
class HasOrdSemantics (α : Type) where
  isZero : α → Bool
  isSuccessor : α → Bool
  predecessor : α → Option α
  expand : α → ℕ → α

/-- 记号表达式 a 的语义序数（三分支递归，见文件头注释）。 -/
noncomputable partial def ordinalOf {α : Type} [HasOrdSemantics α] (a : α) : MOrdinal :=
  if HasOrdSemantics.isZero a then (0 : MOrdinal)
  else if HasOrdSemantics.isSuccessor a then
    match HasOrdSemantics.predecessor a with
    | some p => ordinalOf p + (1 : MOrdinal)
    | none => (0 : MOrdinal)
  else
    sSup { o : MOrdinal | ∃ n : ℕ, o = ordinalOf (HasOrdSemantics.expand a n) }

end Googology
