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
- `expand(n)` / `reduce()` 返回的是**记号自身对象**（可继续 `expand` / `to_string`），不是字符串、也不是数字。
  `expand(n)` / `reduce()` return the notation's **own object** (which can be expanded further or
  serialized), never a string and never a number.
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
enum class Op { FromString, ToString, Normalize, Compare, Expand, ExpandTo, Successor };
class Capabilities { void set(Op); bool has(Op) const; };
```

每个记号重写 `capabilities()`。基类提供默认实现（抛异常），因此部分实现的记号仍是合法的
`Notation`，接口始终完整。
Each notation overrides `capabilities()`. The base supplies default (throwing) implementations, so
a partial notation is still a valid `Notation` and the interface is complete.

> **`normalize()` / `isSuccessor()` 是 `OrdinalNotation` 基类（继承 `Notation`）里的 NON-VIRTUAL 函数 / non-virtual functions in `OrdinalNotation` (which derives from `Notation`).**
> 标准型的**定义**对所有序数记号通用，而序数**序列**记号的**算法**也完全相同——
> 唯一差异是极限表达式的首项 `baseVal_`（PrSS=0，ε_pSS/ε_ωSS=1）。因此二者只写
> 一份（序数基类里的非虚函数），由各记号在构造时设定 `baseVal_`；不再三处复制。
> The standard-form *definition* is universal, and — for the ordinal **sequence** notations — the
> *algorithm* is byte-identical too; the only per-notation input is the limit expression's first
> element `baseVal_` (PrSS=0, ε_pSS/ε_ωSS=1). So both live as ONE non-virtual function in
> `OrdinalNotation` (parameterized by `baseVal_`, set per notation in its constructor) — no 3-way duplication.

## 3b. 类层级 / Class hierarchy

```
Notation                 (generic root — all notations)
├─ OrdinalNotation : Notation        (base of ALL ordinal notations)
│   ├─ prss / eps_p_ss / eps_omega_ss   (ordinal SEQUENCE; carry seq_ + standard-form)
│   └─ (future) cnf / veblen / buchholz / taranovsky / ...
└─ Knuth / Conway : Notation         (large-number; root directly, no seq_ / no std-form)
```

`OrdinalNotation` 承载所有序数记号共有的标准型检测（`normalize()` / `isSuccessor()`，非虚共享实现，仅以 `baseVal_` 区分）；大数记号直接继承 `Notation`，不携带 `seq_`、也不进入标准型逻辑。
`OrdinalNotation` carries the standard-form detection shared by every ordinal notation (`normalize()` / `isSuccessor()`, non-virtual, distinguished only by `baseVal_`); large-number notations derive straight from `Notation` and never see `seq_` or standard-form logic。

> **`core/Ordinal` 与 `WeakVeblen` 的层级说明 / hierarchy note.**
> `core/Ordinal` 是一个**独立的 value type**（序数表达式树），**不是** `Notation`
> 的子类——它是支撑层，被 WeakVeblen 等"非封闭序数记号"持有。
> `omega/Ordinal` is a **separate value type** (ordinal expression tree), **not** a
> subclass of `Notation`; it is a foundation layer held *by* notations like WeakVeblen.
> `ordinal/veblen/weakveblen` 直承 `Notation`（**不**经 `OrdinalNotation`，因
> 非自然数序列、文章无标准型 / see spec `weak_veblen_like.md` C4），其内部的
> 序数算术经 `core/Ordinal` 完成。`weak_veblen` derives straight from `Notation`
> (not `OrdinalNotation`; see spec `weak_veblen_like.md` C4) and delegates its
> ordinal arithmetic to `core/Ordinal`.

## 3c. 元数据：创造者 / 版本 / Metadata: creator / version

每个记号可携带两项**可选元数据**（不是运算，故不在能力位里）：
Every notation may carry two **optional metadata** fields (not operations, so not in the capability bitset):

- **`creator()`** — 创造者（命名者）/ the creator (who named/introduced it).
  约定（记住 / convention）：**创造者的名字直接作为类名前缀的一部分**（类名即隐含创造者），
  并在本方法中返回该名字。例如 `WeakVeblen` 的创造者元数据为 `"zahin"`
  （文章标题即 *zahin's weak-Veblen-like notation*）。各类在自己的类里覆写此方法；
  默认空串表示未知，待日后在"计划模程"中标明。
  Convention: **the creator's name is part of the CLASS name (prefixed)**, and this
  method returns that name. Each notation overrides it in its OWN class; default
  empty = unknown, to be annotated later in the planning template.
- **`version()`** — 版本号 / version. 可为**字符串或一串数字代码**
  （如 `"1"`、`"2.3"`、`"a17"`、`"1.2.0"`）。各类在自己的类里覆写；
  默认空串。每个记号及其各分支小版本各自持有自己的版本。
  A **string or a group of numeric codes** (e.g. `"1"`, `"2.3"`, `"a17"`,
  `"1.2.0"`). Each notation / variant overrides it in its own class; default empty.

当前已填 / currently populated: `weak_veblen`→`creator="zahin", version="1"`；
`knuth`→`creator="Donald Knuth", version="1"`；`conway`→`creator="John Conway", version="1"`；
`epsilon_ss` 模块中 `EpspSS`→`creator="zahin", version="1"`，`EpsOmegaSS`→`creator="zahin", version="1"`
（ε_pSS 与 ε_ωSS 均由 zahin 命名，同出一篇源文章）。
其余记号默认空串，待你在计划模程中写好后交来编写（含各分支小版本及其创造者/版本）。
The rest default to empty until you supply them (with each variant's creator/version) via the planning template.

## 5. 运算语义 / Operation semantics (per family)

- **FromString / ToString** — 字符串 ↔ 内部形式 / string <-> internal form.
- **Compare** — 序数 → 序数序；大数 → **抛 `NotComparable`** / ordinal → ordinal order;
  number → **throws `NotComparable`**.
- **Expand(n)** — 序数 → 基础序列第 n 项 α[n]；大数 → 一次重写步，应用 n 次 / ordinal → nth
  fundamental term α[n]; number → one rewrite step, n times.
- **ExpandTo(len)** — 展开到目标规模 / 长度（数组记号用）/ expand to a target size / length.

（`normalize()` / `isSuccessor()` 是 `OrdinalNotation` 基类（继承 `Notation`）里的非虚函数，所有序数序列记号共用同一份实现，仅由 `baseVal_` 区分。）


所有运算均**不产生数值**：`expand` / `reduce` 返回记号自身（仍是未求值的符号对象），需要 LaTeX 时再调用 `to_string()`。
None of these operations produce a numeric value: `expand` / `reduce` return the notation object
itself (still unevaluated); call `to_string()` when LaTeX is needed.

## 6. 跨族比较不可定义 / Cross-family comparison undefined

一般不可判定；仅允许"同记号 / 兼容族内"比较。跨族不提供桥接（原 `ToOrdinal` 已移除）。
Generally undecidable; only "same notation / compatible family" comparison is allowed. No
cross-family bridge is provided (the former `ToOrdinal` is removed).

## 7. 本分支状态 / Status of this C++ branch

已实现（真实、可审计）/ Implemented (real, auditable):
- `number/knuth` — Knuth up-arrow (高德纳箭头)
- `number/conway` — Conway chained arrow (康威链式箭头)
- `ordinal/sequence/difference/prss` — PrSS (阶差型, 继承 `OrdinalNotation`)
- `ordinal/sequence/difference/epsilon_ss` — ε_pSS + ε_ωSS **合并模块** (同一类型;
  同源于 `study/notations/"epsilon_nSS & epsilon_omegaSS.md"`，均继承 `OrdinalNotation`)
- `core/Ordinal` — **统一序数表达式层 / unified ordinal expression layer**
  （value type，非 Notation）。对应 `study/notations/Ordinal.md`：变体
  `Zero/Omega/Succ/Add/Mul/Pow/WV/Cnf`；`expand` 重写表达式树；
  Cantor 正规形（CNF）仅用于闭序数的比较/前驱/左减；含 C2（\(\omega\) 的
  FS=\(n\)）、C3（分量算术走 CNF，WV 分量不可化归则抛错）约定。
- `ordinal/veblen/weakveblen` — **WeakVeblen (zahin 类弱 Veblen 记号)**
  对应 `study/notations/weak-Veblen-like notation.md`：6 个 case 的 `expand`
  **逐字**实现，序数算术由 `core/Ordinal` 支撑。⚠ **直承 `Notation`**
  （C4），**不**继承 `OrdinalNotation`：它不是自然数序列记号、文章
  **未给标准型 / `normalize` / `successor` 算法**（但 §1.3 已给
  `compare`），故暴露 FromString/ToString/Expand/**Compare**。

大数记号（knuth/conway）刻意**不**支持 `Compare`；序数序列记号通过 `OrdinalNotation`
继承 `normalize()` / `isSuccessor()` 与 `Compare`（`prss` 已实现 `compare`；
`epsilon_ss` 模块（ε_pSS + ε_ωSS 合并）现亦按"标准型下字典序"实现 `compare`，跨类型抛 `NotComparable`）。
Number notations deliberately do **not** support `Compare`; ordinal sequence notations get
`normalize()` / `isSuccessor()` + `Compare` via `OrdinalNotation` (`prss` and the merged
ε_pSS/ε_ωSS module implement `compare` as lexicographic order under standard form; cross-type throws
`NotComparable`). Other notations are future slots.

> 代码对齐说明 / Code-alignment note: 已移除 `evaluate()`，`expand` / `reduce` 返回记号自身的对象（非字符串、非数值），
> 字符串转换仅由 `to_string()` / `string_to_it()` 负责。`evaluate()` has been removed; `expand` /
> `reduce` return the notation object itself (neither a string nor a value), and string conversion
> is handled solely by `to_string()` / `string_to_it()`.

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

> **中英双语文档分开存储 / Bilingual docs are stored SEPARATELY.**
> 每个记号的 spec 用**后缀区分语言**的两个文件：`name.zh.md`（中文）与 `name.en.md`（英文），
> 放在同一目录下（如 `spec/notations/ordinal/veblen/weakveblen/weak_veblen.zh.md`）。
> **已有双语文件暂不迁移**（如 `spec/notations/weak_veblen_like.md` 仍保留双语）；
> 新记号 / 新文档一律按 `name.zh.md` + `name.en.md` 分写。
> Each notation's spec is TWO files distinguished by language suffix: `name.zh.md` (Chinese)
> and `name.en.md` (English), in the same directory. **Existing bilingual files are NOT
> migrated** (e.g. `weak_veblen_like.md` stays bilingual); new notations/docs always use
> the split `name.zh.md` + `name.en.md`.

> **同一类型记号写在同一模块 / Same-type notations live in ONE module.**
> 判定规则（用户约定）：**源文章里写在同一个文件（或同一节）的记号 = 同一类型**，
> 应在库里放进同一个模块（同一个头文件 + 同一个实现文件），spec 也合并为
> `name.zh.md` + `name.en.md`。例：ε_pSS 与 ε_ωSS 同源于
> `study/notations/"epsilon_nSS & epsilon_omegaSS.md"`，故已合并为 `epsilon_ss` 模块
> （`include/.../epsilon_ss/EpsilonSS.hpp` 内含 `EpspSS` 与 `EpsOmegaSS` 两类的原样实现）。
> Rule (user convention): notations that appear together in the SAME source article/file are
> the SAME type and live in ONE module (one header + one .cpp); the merged spec is
> `name.zh.md` + `name.en.md`. E.g. ε_pSS and ε_ωSS share the article
> `"epsilon_nSS & epsilon_omegaSS.md"`, so they are merged into the `epsilon_ss` module.
