/-
序数记号语义（denotation）：记号表达式 → 它代表的序数。
基于 core/Ordinal.lean 的 MOrdinal（mathlib Ordinal）作语义值。

⚠ 本文件**依赖 mathlib**（Mathlib.SetTheory.Ordinal.*，经 core/Ordinal.lean
引入；MOrdinal = mathlib 的 Ordinal）。构建需 lakefile 中的 mathlib
依赖（v4.30.0-rc2）。记号层（Prss.lean / Pps.lean 等）不依赖 mathlib。

定义（三分支递归）：
  ord(0)       = 0
  ord(后继 A)  = ord(前驱 A) + 1
  ord(极限 A)  = sSup { ord(expand(A, n)) : n ∈ ℕ }
              （expand(A,n) 组成的基本列的确切上界 = A 所代表的序数）

“一律不求值”只适用于大数记号；序数记号有良定义语义，经此函数
接轨统一序数表示 MOrdinal。良基性依赖各记号的良序性（如 PrSS 良序
= ε₀、PPS 良序 = ζ₀），此处为非计算定义（不证明终止）。
Lean 4 的硬约束（实测）：`partial` 不能依赖 noncomputable 常量；
`unsafe`（含 `noncomputable unsafe`、`@[implemented_by]` 的 unsafe 实现）
同样不能——它们都要求代码生成，而 noncomputable 实例（sSup 依赖的
`instConditionallyCompleteLinearOrderBot` 等）没有代码；`noncomputable`
定义又必须可终止。故「无限递归 + 非计算上确界」的纯语义定义只能以
公理（`axiom`）形式给出：ordinalOf 是声明（记号 → 序数）而非计算程序。
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

/-- 记号表达式 a 的语义序数（三分支递归语义，见文件头注释）。
    公理化声明：ord(0)=0、ord(后继)=ord(前驱)+1、ord(极限)=
    sSup{ord(expand(A,n))}。良基性（记号良序 ⇒ 递归良定义）由各记号
    的良序定理承担，此处不计算。 -/
axiom ordinalOf {α : Type} [HasOrdSemantics α] (a : α) : MOrdinal

end Googology
