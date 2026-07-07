# GoogologyLib — 设计契约 / Design Contract

> 本文是整库契约，语言无关；C++ 分支为参考实现，其余分支镜像同一逻辑骨架。
> This is the contract for the whole library, language-neutral in spirit; the C++ branch is the
> reference implementation, other branches mirror the same skeleton.

---

## 1. 序数记号 ≠ 大数记号 / Ordinal notations ≠ large-number notations

二者是根本不同的对象，不可混淆 / The two are fundamentally different objects and must not be confused:

| | 序数记号 (ordinal) | 大数记号 (large-number) |
|---|---|---|
| 表示 represents | 序数（良序）ordinals (well-ordered) | 整数 / 函数 integers / functions |
| `compare` | **定义良好** well-defined (ordinal order) | **通常未定义 UNDEFINED (不明确)** |
| `expand` | 基础序列 α[n] fundamental sequence | 一次重写 / 数组步 one rewrite step |
| 例 example | Cantor, Veblen, Buchholz, Y, PPS | Knuth ↑, Conway →, BEAF, Hyper-E |

架构落实 / Consequences baked into the architecture:
- 通用基类 `Notation` 提供 `compare()`；对**大数记号**它抛 `NotComparable` 且 `comparable()`
  返回 `false`。跨族比较永不定义。
  The generic base `Notation` provides `compare()`; for **large-number notations** it throws
  `NotComparable` and `comparable()` returns `false`. Cross-family comparison is never defined.
- 记号"包含其他运算"（其他运算也要包含进去）= 通过能力位在接口中**声明**它们，即便未全部实现；
  未实现者抛 `UnsupportedOperation`。
  A notation "includes other operations" by **declaring** them in its interface via the capability
  bitset, even if not all are implemented; unsupported ones throw `UnsupportedOperation`.

## 2. 不计算数值，只提供"如何计算" / No numeric evaluation — only "how to compute"

**原则 / Principle:** 本库**绝不**把一个记号坍缩成裸整数或任何数值对象。所谓"计算"，只通过
`expand` / `reduce` 把记号按其自身规则逐步重写，结果仍停留在记号自己的符号形式。即使是普通
算术（如 `3^3`、`2+2`）也保留符号，不求值。
The library **never** collapses a notation to a bare integer or any numeric object. "Computation"
means only that `expand` / `reduce` rewrite the notation step by step under its own rules; the
result stays in the notation's own symbolic form. Even ordinary arithmetic (e.g. `3^3`, `2+2`) is
kept symbolic, not evaluated.

这意味着 / This means:
- 能力位 `Evaluate`（返回数值）与 `ToOrdinal`（映到序数对象）**已从契约中移除**。
  The capabilities `Evaluate` (returns a value) and `ToOrdinal` (maps to an ordinal object) are
  **removed from the contract**.
- `expand(n)` / `reduce()` 返回的是**记号自身的符号形式**（字符串或归一化记号项），不是数字。
  `expand(n)` / `reduce()` return the notation's **own symbolic form** (a string or a normalized
  notation term), not a number.
- 普通算术表达式仅作为展开的终点符号，库不会进一步算出 `27` 之类的终值。
  An ordinary-arithmetic expression is only the symbolic endpoint of expansion; the library does
  not further compute a final value such as `27`.

## 3. 分类树 / Taxonomy (hierarchical)

```
notations/
├─ ordinal/                         (compare 定义良好 / compare well-defined)
│   ├─ sequence/                    (自然数序列记号 / natural-number sequence: Y, PPS)
│   │   ├─ difference/              (阶差型 / difference)   -> pps
│   │   └─ marked_parent/          (标记父向型 / marked-parent) -> y_sequence
│   └─ cnf/ veblen/ buchholz/ taranovsky/ bocf/ mountain/   (经典序数 / classic ordinal)
├─ number/                         (compare 未定义 / compare UNDEFINED)
│   ├─ knuth/     (高德纳箭头 / Knuth ↑)        <- 已实现 / implemented
│   ├─ conway/    (康威链式箭头 / Conway →)      <- 已实现 / implemented
│   └─ beaf/ hyper_e/ extensible_e/ bird/       (规划 / future)
└─ hierarchy/                      (桥接 ordinal -> number)
    └─ fgh/ hardy/ shg/            (规划 / future)
```

每个记号暴露 `name()` / `family()` / `subfamily()` / `style()`，使注册表可不依赖目录布局回答
"列出所有 ordinal/sequence/marked_parent 记号"之类查询。
Every notation exposes `name()` / `family()` / `subfamily()` / `style()` so the registry can answer
queries like "list all ordinal/sequence/marked_parent notations" without relying on layout.

## 4. 能力位模型 / Capability model

```cpp
enum class Op { Parse, Serialize, Normalize, Compare, Expand, ExpandTo, Successor };
class Capabilities { void set(Op); bool has(Op) const; };
```

每个记号重写 `capabilities()`。基类提供默认实现（抛异常），因此部分实现的记号仍是合法的
`Notation`，接口始终完整。
Each notation overrides `capabilities()`. The base supplies default (throwing) implementations, so
a partial notation is still a valid `Notation` and the interface is complete.

## 5. 运算语义 / Operation semantics (per family)

- **Parse / Serialize** — 字符串 ↔ 内部形式 / string <-> internal form.
- **Normalize** — 规范 / 标准形式（序数：标准型；大数：化简）/ canonical / standard form.
- **Compare** — 序数 → 序数序；大数 → **抛 `NotComparable`** / ordinal → ordinal order;
  number → **throws `NotComparable`**.
- **Expand(n)** — 序数 → 基础序列第 n 项 α[n]；大数 → 一次重写步，应用 n 次 / ordinal → nth
  fundamental term α[n]; number → one rewrite step, n times.
- **ExpandTo(len)** — 展开到目标规模 / 长度（数组记号用）/ expand to a target size / length.
- **Successor** — 主要用于序数 / mainly ordinal.

所有运算均**不产生数值**：`expand` / `reduce` 的输出是记号自身的符号形式。
None of these operations produce a numeric value: the output of `expand` / `reduce` is the
notation's own symbolic form.

## 6. 跨族比较不可定义 / Cross-family comparison undefined

一般不可判定；仅允许"同记号 / 兼容族内"比较。跨族不提供桥接（原 `ToOrdinal` 已移除）。
Generally undecidable; only "same notation / compatible family" comparison is allowed. No
cross-family bridge is provided (the former `ToOrdinal` is removed).

## 7. 本分支状态 / Status of this C++ branch

已实现（真实、可审计）/ Implemented (real, auditable):
- `number/knuth` — Knuth up-arrow (高德纳箭头)
- `number/conway` — Conway chained arrow (康威链式箭头)

两者刻意**不**支持 `Compare`（大数记号比较未定义）。其余记号均为规划槽位。
Both deliberately do **not** support `Compare` (large-number comparison undefined). All other
notations above are future slots.

> 代码对齐说明 / Code-alignment note: 当前 C++ 代码仍含 `evaluate()`，将于下一步移除并改为符号化
> `expand` / `reduce`。This C++ code still contains `evaluate()`; the next step removes it and
> switches to symbolic `expand` / `reduce`.

## 8. 整数类型 / Integer type

仅用 `int64_t` 表示解析参数与展开索引（如高度 c、链长），**不作数值计算**。`BigInt` 仅作索引
别名，不引入 boost 等重依赖。
Only `int64_t` is used for parse parameters and expansion indices (e.g. height c, chain length);
**no numeric evaluation**. `BigInt` is merely an index alias; no heavy deps (boost) are pulled in.

## 9. 多分支布局 / Multi-branch layout

`master` 存放语言无关的 `spec/` 与 `tests/vectors/`（黄金测试向量）。各语言分支实现同一逻辑骨架。
详见顶层 README。
`master` holds language-neutral `spec/` and `tests/vectors/` (golden vectors). Each language branch
implements the same logical skeleton. See the top-level README.
