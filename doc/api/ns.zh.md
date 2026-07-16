# Ns / n,m-Ns（非线性后继，实序列记号）

> 一句话定义：把实数 `x = α th NS` 表达为**按序数 α 索引的实序列**（Ns 即参数 `(n,m)=(2,2)` 的特例）。本库保留符号表达式 `α th NS`，并用**累加权重分数**（整数 + w₁/d₁ + …）给出有限下标 α 的精确值。本库**绝不把表达式自动求值成极限下标的数值**。

头文件：`include/googology/notations/real_sequence/ns/Ns.hpp`
命名空间：`googology::real_sequence`

---

## 1. 分类（Taxonomy）

| 接口 | 返回值 |
|------|--------|
| `name()` | `"ns"` |
| `family()` | `Family::RealSequence` |
| `subfamily()` | `"ns"` |
| `style()` | `"real_sequence"` |

**类别**：**实序列记号（real_sequence）**——按序数索引的实序列（"α th NS"，一个实数）。它**不是序数记号**，也**不是大数记号**。已于 2026-07-16 依用户指令从 `ordinal` 移出到 `real_sequence`。因此**没有** `normalize` / `is_standard` / `isSuccessor` / `limit` / `roots` 这类序数方法。

---

## 2. 创建者 / 版本

| 接口 | 返回值 |
|------|--------|
| `creator()` | `"送到本到, nnc"`（`ns.zh.md` / `ns.en.md` 的 Creator 元数据） |
| `version()` | `"1"` |

---

## 3. 能力（Capabilities）

`capabilities()` 声明支持：

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Expand` ✔（委托 `core::Ordinal::expand`，见 §5.3）
- `Op::Compare` ✔（归约为比较序数下标 α，因 `a_α` 随 α 严格递增）

**不支持**（调用对应接口会抛异常或 `UnsupportedOperation`）：
- `Op::Normalize` / `Op::Successor` ✘ —— 非序数记号，无标准型/后继概念，`normalize`/`isSuccessor` 等方法**不存在**。
- 有限/后继下标无基本列，`expand` 对此类下标抛 `UnsupportedOperation`（用户所述"不能自动计算"的情形）。

---

## 4. 继承的基类接口（`Notation`）

Ns 直接继承 `googology::Notation`（**不是** `OrdinalNotation`），因此：

- 拥有：`name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare`（已覆盖）/ `expand`（已覆盖）/ `comparable` / `print` / `operator<<` / `reduce`。
- **不拥有**（`OrdinalNotation` 才有）：`normalize` / `is_standard` / `isSuccessor` / `clone` / `roots` / `master_limit` / `limit`。

> 内部把序数下标 α 存为 `core::Ordinal`（`core/Ordinal.hpp`，值类型，**非** `Notation` 分类成员）。`expand(k)` 委托 `α.expand(k)`——项目 §11 可插拔基本列来源。

---

## 5. 该记号自有 API（核心）

### 5.1 成员

```cpp
Ordinal alpha_;     // 序数下标 α（"α th NS"）
BigInt  n_ = 2;     // 后继步长因子 & NS-极限基本列下标
BigInt  m_ = 2;     // f-极限基本列下标
```

### 5.2 构造与参数

```cpp
Ns();                                     // alpha_ = Ordinal::fromInt(1)（默认 n_=m_=2）
explicit Ns(const std::string& s);        // 解析，默认 n_=m_=2
Ns(BigInt n, BigInt m);                   // 设 n,m，alpha_ = fromInt(1)
Ns(BigInt n, BigInt m, const std::string& s); // 设 n,m 并解析

void setParams(BigInt n, BigInt m);       // 设置 n, m
std::pair<BigInt, BigInt> params() const; // 读取 (n, m)
bool isDefaultParams() const;             // 是否 n_==2 && m_==2（即纯 Ns 特例）
```

### 5.3 覆盖的接口

```cpp
std::string name() const override;        // "ns"
Family      family() const override;      // Family::RealSequence
std::string subfamily() const override;   // "ns"
std::string style() const override;       // "real_sequence"
std::string creator() const override;     // "送到本到, nnc"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void string_to_it(const std::string& s) override;  // 解析表达式
std::string to_string() const override;            // 表达式 "α th NS"（LaTeX 风格）

Ns& expand(BigInt k) override;            // 委托 core::Ordinal::expand（§11 可插拔 FS）
                                          // 有限/后继下标无基本列 -> 抛 UnsupportedOperation
```

### 5.4 比较

```cpp
int compare(const Notation& other) const override;  // 归约成比较下标 α；非 Ns 实参抛 NotComparable
```

因 `a_α` 随 α 严格递增（每步加正项 `1/f(β)`），比较两个 Ns 等价于比较其序数下标 α。非 `Ns` 实参抛 `NotComparable`。

### 5.5 累加权重分数视图（仅对**有限** α 有意义）

```cpp
Rational value() const;                   // 有限 α 的精确有理值 x = α th NS
                                           // 极限下标抛 std::domain_error（不自动计算）
WeightedFractionSum accumulated_weight_fractions() const; // 累加权重分数：int + w1/d1 + w2/d2 + ...
std::string to_fraction_string() const;   // 渲染累加和，如 "1/2 + 1/4"；首项为 "0"
```

- `value()`：返回精确 `Rational`。对**极限下标**抛 `std::domain_error`（需要基本列，库不自动计算）。详见 `Rational.hpp`：`Rational(num, den)`，自动约分，`to_string()` 输出 `"num/den"` 或整数。
- `accumulated_weight_fractions()`：返回 `WeightedFractionSum{ intPart, terms }`（每项是 `(weight, denominator)` 对，分母 >0）。对有限 α，整数部为 0、每权重为 1（即项 `1/n^β`）。其 `.value()` 汇总为 `Rational`。
- `to_fraction_string()`：渲染为 `int + w1/d1 + ...` 形式（如 `"2 + 5/11 + 62/111"`）；`α=1` 首项输出 `"0"`。

**行为要点**
- `expand`：委托 `core::Ordinal::expand`，是**就地（in-place）**修改 `*this` 返回自身引用；但有限/后继下标无基本列，故抛 `UnsupportedOperation`。
- `value()` / `accumulated_weight_fractions()` / `to_fraction_string()`：**只读**，不改动 `*this`；仅对有限 α 有定义，极限下标 `value()` 抛 `std::domain_error`。
- 抛出的异常：`compare` 跨类型抛 `NotComparable`；`value()` 对极限下标抛 `std::domain_error`；`string_to_it` 遇非法输入抛解析异常。

---

## 6. 范例（C++ 用法片段）

```cpp
#include "googology/notations/real_sequence/ns/Ns.hpp"
#include <iostream>
using namespace googology;
using namespace googology::real_sequence;

// 例 1：有限下标的值（累加权重分数）
Ns x("2");                                // α = 2（默认 n=m=2 -> 纯 Ns）
std::cout << x.to_string() << "\n";       // 表达式 "2 th NS"（LaTeX 风格）
std::cout << x.to_fraction_string() << "\n"; // 如 "1/2 + 1/4"
Rational v = x.value();                   // 精确有理数
std::cout << v.to_string() << "\n";

// 例 2：参数化 n,m-Ns
Ns y(3, 2, "2");                          // n=3, m=2, α=2
std::cout << y.params().first << "," << y.params().second << "\n"; // 3,2
std::cout << y.to_fraction_string() << "\n";

// 例 3：比较（归约成比较 α）；极限下标 value() 抛 domain_error
Ns a("3");
Ns b("5");
int r = a.compare(b);                     // 比较下标 -> a < b 返回 -1
// Ns lim("ω"); lim.value();             // 极限下标 -> 抛 std::domain_error
```

---

## 7. 设计要点

- **记号即表达式，一律不求值**：`to_string()` 只打印 `α th NS` 符号；本库**没有** `Evaluate` / `ToOrdinal` 数值求值接口。
- **Ns 是"按序数索引的实序列"，不是序数记号**：故**没有** `normalize` / `is_standard` / `isSuccessor` / `limit` / `roots`。标准型统领定义只适用于序数记号。
- **累加权重分数表达有限下标值**：用户指定用 `int + w1/d1 + ...` 精确表达有限 α 的 `x = α th NS`（如 `"2 + 5/11 + 62/111"`）。Ns 中整数部为 0、每权重为 1（项 `1/n^β`）。
- **极限下标不自动计算**：`value()` 对极限下标抛 `std::domain_error`；`expand` 对有限/后继下标（无基本列）抛 `UnsupportedOperation`，二者都对应"用户所述不能自动计算"的情形。`expand` 委托 `core::Ordinal::expand`（§11 可插拔基本列来源）。
- `compare` 归约为比较序数下标 α（序列严格递增），跨类型抛 `NotComparable`。
- **同一模块两种类型**：Ns 与 n,m-Ns 同 `ns` 模块；Ns 即 `(n,m)=(2,2)` 特例（由 `isDefaultParams()` 检测）。

> 脚注：本文档所有函数签名以 `include/googology/notations/real_sequence/ns/Ns.hpp` 头文件为准。`Rational` / `WeightedFractionSum` 的定义见 `include/googology/core/Rational.hpp`。
