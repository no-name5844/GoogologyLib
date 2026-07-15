# Ns（Nonlinear successo · 非线性后继）

> **Source / 源文章**: `study/notations/Ns.md`
> **Creator / 创造者**: 文章署名 `by 送到本到(UTF-8)`，疑似乱码 / 编码错误；**待用户在计划模程中确认**（暂留空，符合 design.md §3c）。
> **Version / 版本**: 文章未给显式版本号（待定）。
> **参考 / Reference**: 同一源文章定义辅助函数 $f$、集合 $\mathbb{NS}$ 与 $\psi(x)$。

## 1. 定义 / Definition

### 1.1 辅助函数 $f$

$$ f(\alpha) = \begin{cases}
1 & \alpha = 0 \\
f(\beta)\cdot 2 & \alpha = \beta + 1 \\
f(expand(\alpha,2)) & \lnot\exists\beta\;(\alpha = \beta + 1)\quad(\alpha\text{ 为极限})
\end{cases} \qquad (\alpha\in On) $$

> 注：`expand(\alpha,2)` 即 $\alpha$ 的基本列第 2 项 $\alpha[2]$。
> **此函数依赖其他序数记号的基本列定义**（见下文 §2 与 design.md §11）。

### 1.2 通用「$\alpha\,th\,A$」

对任一满足 $|\!A\!|\ge\aleph_0$ 的序数集 $A$：
1. $1st, A = \min A$
2. $\alpha\,th\,A = \min\{x \mid x > \max\{\beta\,th\,A \mid \beta < \alpha\}\;\land\; x\in A\}$

即 $\alpha\,th\,A$ 为 $A$ 按递增良序的第 $\alpha$ 个元素。

### 1.3 $\mathbb{NS}$ 集合

1. $1st,\mathbb{NS} = 0$
2. $(\alpha+1)\,th\,\mathbb{NS} = \alpha\,th\,\mathbb{NS} + \dfrac{1}{f(\alpha)}$
3. $\alpha\,th\,\mathbb{NS} = \lim_{i\to\infty} expand(\alpha,i)\,th\,\mathbb{NS}$，当 $\lnot\exists\beta\;(\alpha=\beta+1)$（$\alpha$ 为极限；原文 `th A` 应为 `th NS` 笔误）

故 $\mathbb{NS} = \{a_\alpha \mid \alpha\in On\}$，其中
- $a_0 = 0$
- $a_{\alpha+1} = a_\alpha + 1/f(\alpha)$
- $a_\lambda = \sup_i a_{expand(\lambda,i)}$（$\lambda$ 为极限）

$\mathbb{NS}$ 是一列**按序数索引的实数**（后继步长 $1/f(\alpha)$ 递减，故名「非线性后继」）。

### 1.4 $\psi(x)$

$$ \psi(x)\,th\,\mathbb{NS} = x $$

即 $\psi(x)$ 为实数 $x$（须属于 $\mathbb{NS}$）在 $\mathbb{NS}$ 序列中的序数下标（逆枚举 / 塌陷函数）。

## 2. 跨记号基本列依赖（design.md §11）

`Ns` 在**两处**依赖其他序数记号的**基本列（fundamental sequence）**定义：
1. 辅助函数 $f$ 的极限分支使用 `expand(α,2)`；
2. 极限元素的定义使用 `expand(α,i)\,th\,\mathbb{NS}`（整个基本列）。

按本项目 §11 原则：此「被依赖部分」应**支持多种序数记号**（可插拔地传入基本列来源），而非写死单一记号。实现时 `expand(α,·)` 应作为参数 / 泛型约束，由调用方决定用哪种记号（如 `core/Ordinal` 的统一展开，或某个具体记号）。

## 3. 实现要点 / 待定（pending）

- **模块归属**：`Ns` 产生「按序数索引的实数列」，既不属 `number/` 也不属纯 `ordinal/`。建议置于 `ordinal/ns/`（新子族），或 taxonomy 新开一类（如 `ordinal_sequence`）；**待用户确认**。
- **实数算术**：元素为实数（有理数），需引入有理数 / 实数表示（当前库仅用 `BigInt`）。
- **基本列来源参数化**：`expand` 依赖须做成可插拔（§11）。
- **支持的运算（提议，未定）**：`FromString` / `ToString`（LaTeX）、`Expand`（枚举 $a_\alpha$）、`Compare`（实数可比较）；`Normalize` / `Successor` 文章未定义，不实现。
- **Creator / Version**：文章署名疑似乱码，待用户确认为 `送到本到` 或真名后再填（design.md §3c）。
