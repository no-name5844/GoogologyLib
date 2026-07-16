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
├─ hierarchy/                      (桥接 ordinal -> number)
│   └─ fgh/ hardy/ shg/            (规划 / future)
└─ real_sequence/                 (按序数索引的实数列 / ordinal-indexed real sequence; 非序数记号、非大数记号)
    └─ ns/   (Ns / n,m-Ns)        <- 已实现 / implemented
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

> **`normalize()` / `isSuccessor()` 是 `OrdinalNotation` 基类（继承 `Notation`）里的成员；`normalize()` 为 `virtual`（2026-07-16 改）。**
> 【标准型=定义】是**统领(universal)**概念，对所有序数记号通用；【规范化=动作】则是
> **逐记号(per-notation)**的——每个记号各自把自身表达式改写到那个统领定义。基类仅保留
> 一份供**序数序列**记号复用的**共享默认体**（仅 `baseVal_` 不同：PrSS=0，ε_pSS/ε_ωSS=1），
> 它并非"统领函数"。`isSuccessor()` 是**后继**判定（序列末项==`baseVal_`），与"标准型"无关，不改名。
> The *definition* of standard form is universal; the *action* of normalizing is per-notation.
> `normalize()` is `virtual`: the base class keeps only a shared default body (parameterized by
> `baseVal_`) for the ordinal-sequence family — it is NOT an umbrella function. `isSuccessor()`
> tests *successorship* (last term == `baseVal_`), unrelated to standard form, and is not renamed.

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

## 10. 使用政策与免责声明 / Usage Policy & Disclaimer

> **中文（政策）**
> 1. **大数记号拒绝求值型运算 / Refuse evaluation-style ops on large-number notations.**
>    对大数记号（如 Knuth ↑ / Conway →），一旦出现"减一"、"取前驱"、"左减"等
>    **依赖先求出数值**的运算，库**直接拒绝，不计算**。这些数太大，减一 ≈ 把整个数
>    算出来（见 §2 深层理由），在原则上不可行。接口中**不存在** `evaluate()` /
>    减一 / 前驱等运算；若未来误加，必须以抛异常方式拒绝，绝不偷偷求值。
> 2. **仅供研究参考，非取代计算器 / For research reference only, not a calculator.**
>    本库提供的展开 / 重写 / 比较等运算**仅供研究参考**，不能在"直接分析"时
>    **当作代替的计算器**使用。
> 3. **学习以定义为准，不单纯依赖本库 / Learn from definitions, not the library alone.**
>    研究学习应基于记号的**数值含义与相关定义**（记号定义、展开规则、标准型等）
>    本身来进行；本库是辅助工具，**不能单纯依靠它**来获得理解或结论。

> **English (policy)**
> 1. **Large-number notations refuse evaluation-style ops.** Any op that *presupposes
>    computing the value first* — subtract-one, predecessor, left-subtraction — on a
>    large-number notation is **refused outright, never computed**. These numbers are
>    so huge that "minus one" ≈ computing the whole value (see §2 rationale), which is
>    infeasible in principle. Such ops (`evaluate()`, predecessor, etc.) **do not exist**
>    in the interface; if ever mistakenly added, they must throw rather than silently
>    evaluate.
> 2. **Research reference only — not a substitute calculator.** The library's
>    expansion / rewriting / comparison are **for research reference only**; they must
>    not be used as a **substitute calculator** during direct analysis.
> 3. **Learn from definitions, not the library alone.** Study should rest on the
>    notation's **value meaning and related definitions** (its definition, expansion
>    rule, standard form, …). The library is an aid — **never rely on it alone** for
>    understanding or conclusions.

## 11. 跨记号依赖基本列的设计原则 / Cross-notation fundamental-sequence dependencies

> **中文（原则）**
> 若一个记号的展开 / 定义**需要依赖其他序数记号的基本列（fundamental
> sequence）定义**，那么它在"被依赖的那一部分"应当**支持多种序数记号**，
> 而不能写死成某一个具体记号。
> - 例：某记号在某个分支要"取下方序数的基本列第 n 项"，而那个下方序数
>   可能由不同记号（如 ε 系、Veblen、Buchholz 等）表示——这部分就应当
>   对多种序数记号**多态 / 可插拔**，而非硬编码成单一记号。
> - 实现落点：`expand` / 相关辅助应接受"被依赖记号的展开规则"作为参数或
>   泛型约束，由调用方决定用哪一种；库内部共享的 `core/Ordinal` 表达式树
>   （Zero/Omega/Succ/Add/Mul/Pow/WV/Cnf）正是为此类"统一序数表达"服务。
> - **实例**：源文章 `study/notations/Ns.md` 的辅助函数 $f$（极限分支用
>   `expand(α,2)`）与 $\mathbb{NS}$ 极限条款（用 `expand(α,i)`）即触发本原则；
>   其 spec 见 `spec/notations/ns.zh.md` / `ns.en.md`（待实现）。
> - 反之，若某记号的定义**不**依赖其他记号的基本列，则无需支持多种，
>   只实现自身即可。

> **English (principle)**
> If a notation's expansion / definition **depends on the fundamental sequence
> of other ordinal notations**, then the *dependent part* must **support multiple
> ordinal notations**, not be hardwired to a single one.
> - E.g. a notation whose some branch needs "the n-th term of the fundamental
>   sequence of the ordinal below" — that below-ordinal may be expressed in
>   different notations (ε-family, Veblen, Buchholz, …); that part should be
>   **polymorphic / pluggable** across notations, not hardcoded to one.
> - Implementation: `expand` / the relevant helper should take the *dependent
>   notation's expansion rule* as a parameter or generic constraint, chosen by the
>   caller. The shared `core/Ordinal` expression tree (Zero/Omega/Succ/Add/Mul/
>   Pow/WV/Cnf) exists precisely to serve this "unified ordinal representation".
> - Conversely, if a notation's definition does **not** depend on another's
>   fundamental sequence, it need not support multiple — just implement itself.

## 12. 标准表达式的判定 / Deciding whether an expression is standard

> **统领定义回顾 / Recap of the universal definition.** 一个表达式是【标准表达式】
> ⇔ 它可以通过某个【极限表达式】，经【有限次展开（expand）】并【取基本列前若干项】
> 的方式得到（见 `OrdinalNotation.hpp` 注释与 §4）。

**作用域 / Scope.** 标准型的判定**本质上只针对序数记号**：它唯一依赖的 `cmp`（序数全序）与
`fund_seq`（基本列）都是**序数记号**才有的结构。Knuth 上箭头、Conway 链、Ns（按序数索引的
实数列）等没有序数序 / 基本列，§12 对它们不适用。本库将算法实现为**泛型引擎**
（`core/StandardForm.hpp` 的模板 `StdSystem<Expr>`，只依赖 `cmp`+`fund_seq`+`clone`+`roots`），
每个序数记号系统只需提供这四个接口即可套用。`core::Ordinal`（统一序数值类型，自带
`compare` 与 `expand`＝基本列）是其中一个具体实例（`OrdinalSystem`）。**序数记号类
`OrdinalNotation`** 再提供一层适配器（`OrdinalNotationSystem`），把自己的 `compare()`/`expand()`
（即基本列）/`clone()`/`roots()` 接进引擎，并把判定暴露为**成员函数 `is_standard()`**——即
本节算法"在单一记号系统内、只依赖该系统自带的 cmp + fund_seq"的落地（见该头文件与
`tests/unit/test_standard_form.cpp`）。

下面给出在一个**自洽记号系统**内、仅依赖系统自带的比较规则 `cmp` 与基本列规则
`fund_seq` 的判定过程——**不借助任何外部语义**。

**已知 / Given**
- `cmp(X, Y)`：对全体标准表达式是全序，返回 `LESS` / `EQUAL` / `GREATER`。
- `fund_seq(L, n)`：当 `L` 是极限记号时返回第 `n` 项 `L[n]`，且 `cmp(L[n], L) == LESS`。
- 有限集 `Roots`：所有"最顶层"的极限记号；系统保证任何标准表达式都从某个
  `R ∈ Roots` 出发、经有限次 `fund_seq` 展开得到。

**算法一：带剪枝的广度优先搜索（自根向下）/ BFS-with-pruning (top-down)**
1. 初始化：队列 `Q ←` 所有 `Roots` 元素；`visited ← ∅`（可选，避免重复展开）。
2. 循环（`Q` 非空）：取队首 `X`；`c = cmp(X, E)`。
   · `c == EQUAL` → 找到，`E` 是标准表达式，返回 `True`。
   · `c == LESS` → `X` 已小于 `E`；后续展开严格下降，永不达 `E`，丢弃 `X`（剪枝）。
   · `c == GREATER` → `X` 仍大于 `E`，仍可能展开后等于 `E`：
      若 `X` 是极限记号，则生成候选直接后代加入 `Q`；否则丢弃。
3. 候选直接后代的生成（可计算的关键）：不盲目枚举全体 `n`，而是利用
   `cmp` 与基本列结构，只生成"有可能等于或大于 `E`"的 `X[n]`：
   · 若 `fund_seq` 对 `n` 单调（通常如此），`X[n]` 随 `n` 增大而增大；用比较规则找出
     最小 `n₀` 使 `cmp(X[n₀], E) != LESS`（即 `X[n₀] ≥ E`）与最大 `n₁` 使
     `cmp(X[n₁], E) != GREATER`（即 `X[n₁] ≤ E`），候选索引仅落在有限区间
     `n ∈ [n₀, n₁]`。对该区间内每个 `n` 计算 `Y = fund_seq(X, n)`，未访问则入队。
   · 若基本列非数值索引而是结构驱动（如 `X = ω^α` 仅当 `α` 是极限时
     `X[n] = ω^{α[n]}`），候选集由语法分析直接给出，索引继承自 `α` 的基本列，
     同样可用本算法递归确定。
   · 若无法构造有限候选索引集，则本判定在该系统上不可直接实现——系统须
     提供更强的标准形条件或额外可计算结构。
4. 终止：若 `Q` 变空，说明所有路径都掉到 `< E` 区域或无法展开，始终未遇等于 `E`
   的表达式，返回 `False`。

**为何终止？** 正向展开严格递减；一旦 `cmp(X,E)==LESS` 即永久不可逆，该分支
深度有限；`cmp` 全序把空间分为 `>E` / `=E` / `<E` 三区，算法只在 `>E` 区向下探索，该区
下降链有限（由系统所基于序数的良基性在记号上的投影保证），故搜索空间有穷。

**算法二：向上回溯法（更实用的变体）/ Upward backtracking (practical variant)**
1. 若 `E` 本身在 `Roots` 中，直接返回 `True`。
2. 否则寻找极限记号 `L` 与索引 `n` 使 `fund_seq(L, n) == E`（按 `cmp` 判等）。
3. 找到则递归判定 `L` 是否标准。
4. 找不到任何 `L` 则 `E` 非标准。
寻找 `L` 时同样用 `cmp` 压缩：必须 `cmp(L, E) == GREATER`，且由基本列结构反推
可能形式，把搜索空间压到极小。两法本质相同：用 `cmp` 做边界、用基本列做桥梁，
在 `>E` 的有限高度内完成全部搜索。

> **与库 API 的关系 / Relationship to the library API.**
> 这是"标准表达式判定"的概念骨架。本库**已**把 `is_standard()` 作为 **`OrdinalNotation`
> （序数记号基类）的成员函数**暴露（用户 2026-07-16 纠正："标准型的判定基本上针对都是
> 序数记号……它是属于序数记号这个类的一个成员函数"）。它在本记号系统内部、用本记号
> 自带的 `compare()`（序数序）与 `expand()`（即基本列）跑泛型 §12 引擎（`OrdinalNotationSystem`
> 适配器），不借任何外部语义——正是本节算法"单一系统、自带 cmp + fund_seq"的落地。
> `core::Ordinal` 上的 `is_standard_bfs(Ordinal)` / `is_standard_backtrack(Ordinal)` 便捷重载仍保留，
> 作为可独立使用的参考；本节同时作为各语言分支（C / Java / Python / Lean4）实现判定逻辑时的
> **形式化参考**。标准型判定**只依赖 `cmp` + `fund_seq`**，正是 §11 要求基本列来源
> 可插拔（pluggable）的同一理由——判定本身也是自洽系统内的事。
