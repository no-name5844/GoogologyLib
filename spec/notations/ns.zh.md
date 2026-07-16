# Ns（Nonlinear successo · 非线性后继）

> **Source / 源文章**: `study/notations/Ns.md`
> **Creator / 创造者**: 文章署名 `by1:送到本到(UTF-8) 2:nnc`——两位创造者：`送到本到`（含 `(UTF-8)` 备注，疑似编码标注）与 `nnc`。`creator()` 拟填 `"送到本到, nnc"`（多作者以逗号连接）；**确切格式待用户在计划模程确认**（design.md §3c）。
> **Version / 版本**: 文章未给显式版本号（待定）。
> **参考 / Reference**: 同一源文章定义辅助函数 $f$、集合 $\mathbb{NS}$、$\psi(x)$，以及参数化家族 `n,m-Ns`（Ns 为其 $(n,m)=(2,2)$ 特例）。

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

1. $1\,th,\mathbb{NS} = 0$
2. $(\alpha+1)\,th\,\mathbb{NS} = \alpha\,th\,\mathbb{NS} + \dfrac{1}{f(\alpha)}$
3. $\alpha\,th\,\mathbb{NS} = \lim_{i\to\infty} expand(\alpha,i)\,th\,\mathbb{NS}$，当 $\lnot\exists\beta\;(\alpha=\beta+1)$（$\alpha$ 为极限）

故 $\mathbb{NS} = \{a_\alpha \mid \alpha\in On,\ \alpha\ge 1\}$，其中记 $a_\alpha = \alpha\,th\,\mathbb{NS}$：
- $a_1 = 0$
- $a_{\alpha+1} = a_\alpha + 1/f(\alpha)$（$\alpha\ge 1$）
- $a_\lambda = \sup_i a_{expand(\lambda,i)}$（$\lambda$ 为极限）

$\mathbb{NS}$ 是一列**按序数索引的实数**（后继步长 $1/f(\alpha)$ 递减，故名「非线性后继」）。

### 1.4 $\psi(x)$

$$ \psi(x)\,th\,\mathbb{NS} = x $$

即 $\psi(x)$ 为实数 $x$（须属于 $\mathbb{NS}$）在 $\mathbb{NS}$ 序列中的序数下标（逆枚举 / 塌陷函数）。

### 1.5 参数化家族 $n,m\text{-}Ns$

源文章另给出带参数的推广 $n,m\text{-}Ns$（创造者同 Ns）。其辅助函数把 Ns 中的常数 $2$ 替换为参数 $n,m$：

$$ f(\alpha) = \begin{cases}
  1 & \alpha = 0 \\
  f(\beta)\cdot n & \alpha = \beta + 1 \\
  f(expand(\alpha,m)) & \lnot\exists\beta\;(\alpha=\beta+1)\quad(\alpha\text{ 为极限})
\end{cases} \qquad (\alpha\in On) $$

通用「$\alpha\,th\,A$」、$\mathbb{NS}$ 三条规则与 $\psi(x)$ 与 Ns **完全相同**，仅把 $f$ 换成上述参数化版本。两个参数分工：
- **$n$**：同时出现在后继步长（$\cdot n$）与 $\mathbb{NS}$ 极限行的基本列下标 $expand(\alpha,n)$；
- **$m$**：仅出现在辅助函数 $f$ 的极限分支 $expand(\alpha,m)$。

故 **Ns 即 $(n,m)=(2,2)$ 的特例**。两处基本列依赖（§2）在 $n,m\text{-}Ns$ 中变为：**$f$ 极限用 $expand(\alpha,m)$、$\mathbb{NS}$ 极限用 $expand(\alpha,n)$**，均须可插拔（§11）。

## 2. 跨记号基本列依赖（design.md §11）

`Ns` 与 `n,m-Ns` 均依赖其他序数记号的**基本列（fundamental sequence）**定义（design.md §11）：

- **Ns（$(n,m)=(2,2)$）**：两处依赖
  1. 辅助函数 $f$ 的极限分支使用 `expand(α,2)`；
  2. 极限元素的定义使用 `expand(α,i)\,th\,\mathbb{NS}`（整个基本列）。
- **n,m-Ns**：两处依赖，且各自使用不同参数
  1. 辅助函数 $f$ 的极限分支使用 `expand(α,m)`；
  2. 极限元素的定义使用 `expand(α,n)\,th\,\mathbb{NS}`（整个基本列）。

按本项目 §11 原则：此「被依赖部分」应**支持多种序数记号**（可插拔地传入基本列来源），而非写死单一记号。实现时 `expand(α,·)` 应作为参数 / 泛型约束，由调用方决定用哪种记号（如 `core/Ordinal` 的统一展开，或某个具体记号）；对 $n,m\text{-}Ns$ 还需把 $n,m$ 作为记号参数携带。

## 3. 实现要点（已落地）

- **模块归属**：已实现于 `ordinal/ns/`，同一模块含 `Ns` 与 `n,m-Ns`（`Ns` 即 $(n,m)=(2,2)$ 特例）。Taxonomy：`family()=Ordinal`、`subfamily()="ns"`、`style()="real_sequence"`。
- **实数算术（累加权重分数）**：新增 `core/Rational.hpp` 表达精确有理数，并新增 `WeightedFractionSum`（累加权重分数类型：`整数 + w₁/d₁ + w₂/d₂ + …`，如 `2 + 5/11 + 62/111`）。有限下标 $\alpha=k\ge 1$ 的精确值为
  $$a_k = \sum_{\beta=1}^{k-1}\frac{1}{n^{\beta}}$$
  （$n=2$ 即 Ns；$a_1=0,\;a_2=1/2,\;a_3=3/4,\;a_4=7/8\ldots$；此时整数部为 0、各权值 $w=1$）。极限下标需要基本列、**不自动计算**（见下）。
- **基本列来源参数化（§11）**：`expand` 委托 `core::Ordinal::expand`，即项目 §11 的「可插拔基本列来源」。有限 / 后继下标没有基本列，调用 `expand` 抛 `UnsupportedOperation`（正是用户所指「这玩意也不能自动计算」的情形）。
- **支持的运算**：`FromString` / `ToString`（LaTeX，输出 `α th \mathbb{NS}`）、`Expand`（如上）、`Compare`（直接比 $\alpha$，因 $a_\alpha$ 随 $\alpha$ 严格递增）；`Normalize` / `Successor` 文章未定义，不实现。
- **Creator / Version**：`creator()="送到本到, nnc"`，`version()="1"`（已按用户确认填入）。
- **累加权重分数视图（仅有限 $\alpha$）**：`value()` 返回精确 `Rational`；`accumulated_weight_fractions()` 返回 `WeightedFractionSum`（整数部 + 权值/分母列表）；`to_fraction_string()` 渲染为 `1/2 + 1/4 + …`（一般形式为 `2 + 5/11 + 62/111`）；极限下标抛 `std::domain_error`（不自动计算）。

## 4. 实现文件清单

- `include/googology/core/Rational.hpp` — 精确有理数（gcd 约分）+ `WeightedFractionSum`（累加权重分数：`整数 + w₁/d₁ + …`）。
- `include/googology/notations/real_sequence/ns/Ns.hpp` + `src/notations/real_sequence/ns/Ns.cpp` — `Ns` 类（`n_`,`m_` 双参数，`Ns` 为 `(2,2)`）；符号表达式 `to_string()` + 累加权重分数视图（`accumulated_weight_fractions()` / `to_fraction_string()`）+ 委托 `core::Ordinal` 的 `expand` / `compare`。注意：`Ns` 属于 `Family::RealSequence`（按序数索引的实数列），**不是**序数记号，故目录在 `real_sequence/` 而非 `ordinal/`。
- `tests/unit/test_ns.cpp` — 元数据、表达式往返、累加权重分数值（含 `2 + 5/11 + 62/111` 形式）、`expand`、`compare` 断言（全部通过）。
- `src/main.cpp` — 注册 `ns` 并演示。
