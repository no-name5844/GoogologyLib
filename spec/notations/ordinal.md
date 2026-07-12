# Ordinal — 序数表达式基础层 / Unified ordinal expression layer

> 本文件对应源码 `include/googology/core/Ordinal.hpp`，是整个库在
> `study/notations/Ordinal.md` 基础上建立的**统一序数表达式类型**（value type）。
> 它是 `weak-Veblen-like` 等"非封闭序数记号"的支撑层，也可作为其它序数
> 算术记号的公共底座。
> This file backs `include/googology/core/Ordinal.hpp` — the unified
> symbolic ordinal expression type built on `study/notations/Ordinal.md`.
> It is the foundation layer for non-closed ordinal notations such as
> `weak-Veblen-like`, and may serve other ordinal-arithmetic notations.

---

## 1. 文章原文（逐字 / verbatim）

# Ordinal
对于任何一个不封闭的序数记号或非序列型号(若声明不封闭除外)

## 1. 展开与计算
对于 \(\alpha,\beta\in On \cup \Pi_1\) 有：

- 加法 / Addition：
  - \(\alpha+0=\alpha\)
  - \(\mathrm{expand}(\alpha+\beta,n)=\alpha+\mathrm{expand}(\beta,n)\)
- 乘法 / Multiplication：
  - \(\alpha\cdot 0=0\)
  - \(\mathrm{expand}(\alpha\beta,n)=\alpha\cdot\mathrm{expand}(\beta,n)\)
- 指数 / Exponentiation：
  - \(\alpha^0=1\)
  - \(\mathrm{expand}(\alpha^{\beta},n)=\alpha^{\mathrm{expand}(\beta,n)}\)

> 原文为简体中文，此处**逐字转录**，未改动任何符号 / transcribed verbatim.

---

## 2. 库中的语义 / Semantics in the library

`Ordinal` 是一个**表达式树（value type）**，变体（variant）为：

| 变体 Kind | 含义 | 构造工厂 |
|---|---|---|
| `Zero` | 0 | `zero()` / `fromInt(0)` |
| `Omega` | \(\omega\) | `omega()` |
| `Succ` | \(a+1\)（后继） | `succ(x)` |
| `Add` | \(a+b\) | `add(x,y)` / `x + y` |
| `Mul` | \(a\cdot b\) | `mul(x,y)` / `x * y` |
| `Pow` | \(a^b\) | `pow(x,y)` / `x.pow(y)` |
| `WV` | \((a_1@b_1,\dots,a_n@b_n)\) | `wv(comps)` |
| `Cnf` | Cantor 正规形（降幂、系数≥1） | `cnf(terms)` |

- `expand(n)` **重写表达式树并返回新 `Ordinal`**（绝不算值 / never evaluates）。
- 算术构造器（`+` `*` `pow` `successor`）只**建立节点**，不求值；求值只在
  `toCnf()`（见第 4 节）按需发生，且**仅用于闭序数**的比较/前驱/左减。

---

## 3. 文章沉默处的补约定 / Conventions where the article is silent

> 这些约定均为**最小补充**，已在代码注释与实现中以 `C2` / `C3` 标记。
> Minimal additions, flagged in code as `C2` / `C3`.

- **C1 — 统一值类型 / unified value type**：任何"不封闭序数记号"或
  "非序列型号"都以同一棵 `Ordinal` 表达式树表示；文章里的 \(+\)、\(\cdot\)、
  \(\hat{}\) 直接映射到 `Add` / `Mul` / `Pow` 节点。
- **C2 — \(\omega\) 的基础序列 / fundamental sequence of \(\omega\)**：文章未定义
  \(\omega\) 的 FS，采用标准定义 \(\mathrm{expand}(\omega,n)=n\)。
  0、1 与任何后继序数**没有** FS，`expand` 抛 `std::domain_error`。
- **C3 — 分量算术走 CNF / component arithmetic via CNF**：文章里
  "\(a=c+1\)"、"\(a_n=a_k+q\)"、"\(b_i>b_{i+1}\)" 等**条件判断**按各
  **分量**的 Cantor 正规形求值。分量必须是**闭序数**（0,1,\(\omega\),
  \(\omega+1\), …）。若某个分量是 `WV` 表达式（而非闭序数），则它无法化为
  CNF 值（文章未定义 weak-Veblen 表达式的序数取值）→ `toCnf()` 抛
  `std::domain_error`。

### 文章未定义（保持未定义）/ Left undefined by the article

- \(\omega\) 的展开（已由 C2 补）。
- 序数**相等 / 前驱 / 比较**：由 `toCnf()` 在**闭序数**上提供
  （`compare` / `equals` / `isSuccessor` / `predecessor` / `subtract`），
  但 `WV` 分量不进入这些谓词（见 C3）。

---

## 4. Cantor 正规形（CNF）/ Cantor normal form

CNF 仅用于**闭序数**的比较 / 相等 / 前驱 / 左减，由 `toCnf()` 按需生成；
存储节点始终是表达式树，以便 `expand()` 能在文章的 \(+\) / \(\cdot\) / \(\hat{}\)
形式上分派。

- `Zero` → `{}`，`Omega` → `{(1,1)}`，`Cnf` → 自身，
  `Succ/Add/Mul/Pow` → 递归化简，`WV` → **抛异常**（C3）。
- 算术：`addCnf` / `mulCnf` / `powCnf` / `subCnf` 实现标准 CNF 规则；
  \(\alpha^0=1\)、\(0^\beta=0\)（\(\beta>0\)）、\(1^\beta=1\) 均含于
  `powCnf`。
- `successor()` 对 `WV` 节点**合法**（用于 weak-Veblen 的
  "\((\#,a@0)+1\)" 等情形），它直接包成 `Succ` 节点，**不**走 CNF
  （因此 `succ` 用 kind 判断零，而非 `isZero()`，后者会对 WV 抛错）。

---

## 5. 解析器 / Parser

`Ordinal::parse(s)` 接受 ASCII：整数、`w` / `W` / `omega` 或 UTF-8 \(\omega\)
（CF 89）代表 \(\omega\)，`*` 代表乘法、`^` 右结合、圆括号。
仅用于解析**分量**（如 weak-Veblen 的 `a` / `b`）；输出的 LaTeX 用
\(\omega\) / \(\cdot\) 仅为显示，不影响解析。

---

## 6. 不求值 / No evaluation

`Ordinal` 与整个库一致：**从不**把任何记号坍缩成数值。
`expand` / `successor` 只重写符号树；需要 LaTeX 时再调用 `to_string()`。
