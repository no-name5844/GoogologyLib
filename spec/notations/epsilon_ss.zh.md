# ε_pSS / ε_ωSS（合并模块 · 同一类型）

> 两记号出自**同一篇源文章** `study/notations/"epsilon_nSS & epsilon_omegaSS.md"`
> （§1 = ε_pSS，§2 = ε_ωSS）。按项目约定——**源文章里写在同一个文件的记号即
> 同一类型，应在库里放进同一个模块**——故合并于此。
> 代码：`include/googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp`
> （内含 `EpspSS` 与 `EpsOmegaSS` 两个类，原样共置于同一文件）。

## 共同属性 / Common

- **族 / Family**: `ordinal / sequence / difference`（序数 · 自然数序列 · 阶差型）
- **参考 / Reference**: `study/notations/"epsilon_nSS & epsilon_omegaSS.md"`
- 两记号均继承 `OrdinalNotation`，`baseVal_ = 1`，共用 `rightmostLess_` /
  `expandLen_` / `compare` / `normalize` 算法。
- **唯一差别在每步追加量**：
  - ε_pSS 有参数 `p`（封顶）：case 4 把追加量封顶在 `p`。
  - ε_ωSS 无 `p`、无 case 4：`gap q = a_n - a_br` 全量追加（无界），
    即 ε_pSS 的 `p→ω` 极限。

> **记号约定（用户澄清）**：两种下标不要混淆：
> - `a_i`（即"第 i 个 A"）= 序列 A 自身的**第 i 个元素**（元素访问），代码 `seq_[i-1]`。
> - `A[n]`（方括号）= A 所表示序数的**基本列第 n 项**，约定等于
>   `expand(A, n)`，代码 `expand(n)` 返回 `A[n]`。
> 元素访问（`a_n`）与基本列项访问（`A[n]`）是**不同**操作。

### 接口映射（两者通用）
- `expand(m)` → 文章 `expand(A, m)`；`expand_to(M)` → `expandLen(A, M)`。
- 两者都**重写内部序列并返回记号自身对象**，库一律不求值。
- `operator[]` 重载：`A[n]` 返回基本列第 n 项（即 `expand(A,n)`），返回副本、
  不改动 `*this`，故 `A[1]`、`A[2]`… 互相独立。
- `isSuccessor()`：序列以 1 结尾即后继，`expand`/`expandLen` 剥去尾 1。
- `normalize()`：实现项目**通用标准型定义**（表达式可由极限表达式经有限次展开 +
  取前若干项得到）。用户指定两记号的极限表达式均为 **(1, n)**，故规范起点为
  `(1, a_2)`。非标准输入保持不变。
- `compare()`：文章未定义，但用户指定标准型下满足**字典序序数比较**（与 PrSS 一致）；
  跨类型比较抛 `NotComparable`。

## ε_pSS（ε_p Sequence System）

- **Creator / 创造者**: `zahin`（ε_pSS 由 zahin 命名；见代码 `EpspSS::creator()`）。
- **Version / 版本**: `1`（基础版本）。

### 定义 / Definition
ε_pSS 是*阶差型*自然数**序列**记号。其极限表达式为 `(1, n)` —— 即每个合法序列以
`1` 开头。对序列 `A = (a_1, …, a_n)` 唯一要求：
1. `a_i ∈ ℕ`
2. `a_1 = 1`

> 与 **PrSS** 不同，此处放宽了 PrSS 的额外约束（`a_{i+1}-a_i ≤ 1` 及"平台—非升"
> 规则）——允许任意跳跃。ε_pSS 是 PrSS 的*参数化推广*；适当设封顶可恢复 PrSS 在
> gap-1 序列上的行为。

### 展开 / Expansion
辅助函数：
1. `(a_1,…,a_n) ⊕ (b_1,…,b_n) = (a_1,…,a_n, b_1,…,b_n)`
2. `A ⊕^n_{i=a} f(i) = (A ⊕^{n-1}_{i=a} f(i)) ⊕ f(n)`

`expand` 函数：
- 空序列 `A=()`：`expand(A,m) = expandLen(A,m) = ()`。
- 以 1 结尾：`A=(a_1,…,a_n,1)`：`expand(A,m)=expandLen(A,m)=(a_1,…,a_n)`（剥尾 1）。
- 否则令 `br = max{n−i | a_{n−i} < a_n}`（最右值严格小于 `a_n` 的列标，1-based）；
  `L = n−br`；`q = a_n − a_br`（因 `a_br < a_n` 故 `q ≥ 1`）。

```
expandLen(A,m)=
  (a_1,…,a_n−1)                                     m=0
  expandLen(A,k) ⊕ ((m+n−L) th expandLen(A,k))         m=k+1 ∧ a_n = a_k+1
  expandLen(A,k) ⊕ ((n+m−1) th expandLen(A,k) + q−1)  m=k+1 ∧ a_n = a_k+q (1<q≤p)
  expandLen(A,k) ⊕ ((n+m−1) th expandLen(A,k) + p)     m=k+1 ∧ a_n − a_k > p

expand(A,m)=
  expandLen(A,0)          m=0
  expandLen(A, m·L−1)     m>0
```

两种下标形式**严格照文章保留**：case 2 用 `(m+n−L) th expandLen(A,k)`
（gap-1 情形），case 3/4 用 `(n+m−1) th expandLen(A,k) + q−1 / +p`。
被索引元素取自**运行序列** `expandLen(A,k)` 在第 `k=m−1` 阶段的值，*不是* 原序列 `A`。
不做任何重解释或统一。

### 实现说明（C++ 参考分支）
- 类 `googology::ordinal::EpspSS` **仅**逐字实现上述公式。第 `m=k+1` 步追加的
  元素是**运行序列** `expandLen(A,k)` 的第 `X` 项（`X = m+n−L` 或 `n+m−1`），
  *不是* 原 `A`。代码中即此前追加后的 `seq_`，追加值 `seq[X−1] + add`。
- **不引入循环闭合**：文章未定义回绕。当文章的索引 `X` 超出运行序列（输入落在文章
  字面定义域外），代码抛 `std::out_of_range`——防御性守卫，不是对公式的重解释。
- **`compare` 文章未定义**，但依用户指定满足标准型下字典序序数比较（与 PrSS 一致）；
  跨类型抛 `NotComparable`。这是用户指定属性，非文章主张。
- **`isSuccessor()` 依文章后继条款**；`normalize()` 依项目通用标准型定义 + 用户指定
  极限表达式 `(1,n)`。均由用户澄清驱动，非个人见解。

## ε_ωSS（ε_ω Sequence System）

### 定义 / Definition
ε_ωSS 是*阶差型*自然数**序列**记号，是 ε_pSS 的 **ω-极限**（追加量无封顶）。
- 族同 ε_pSS；**参数**：无（无 `p`、无 case 4）。
- 对序列 `A = (a_1, …, a_n)`：`a_i ∈ ℕ`、`a_1 = 1`，PrSS 额外约束同样放宽。
- ε_ωSS = **去掉参数 `p`**（及其 case 4）的 ε_pSS：`gap q = a_n − a_br`
  **全量追加**（无界），即 `p→ω` 极限。

### 展开 / Expansion
公式与 ε_pSS 相同，但**无 case 4**（无封顶）：

```
expandLen(A,m)=
  (a_1,…,a_n−1)                           m=0
  expandLen(A,k) ⊕ ((m+n−L) th expandLen(A,k))      m=k+1 ∧ a_n = a_k+1
  expandLen(A,k) ⊕ ((n+m−1) th expandLen(A,k) + q−1)  m=k+1 ∧ a_n = a_k+q
expand(A,m)= (同 ε_pSS)
```

case 3 的 `q−1` 全量追加（无界）。其余说明（运行序列、不引入闭合、
`compare`/`isSuccessor`/`normalize` 约定）与 ε_pSS 一致。

### 实现说明（C++ 参考分支）
- 类 `googology::ordinal::EpsOmegaSS` 逐字实现上述（无 `p`、无 case 4）。
  其余同 ε_pSS 实现说明。

## 多语言 / Multi-language
按项目约定，两记号最终须移植到 **C / Java / Python / Lean4**（各自独立 git 分支），
均以本 spec 与 `master` 上的黄金向量为准。C++ 实现为当前参考实现。
