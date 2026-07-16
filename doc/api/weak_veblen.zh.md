# WeakVeblen（弱 Veblen 型记号，zahin）

> 一句话定义：弱 Veblen 型序数记号 `A=(a_1@b_1, ..., a_n@b_n)`（`a_i,b_i∈On`、`b_i>b_{i+1}`），严格按文章 `weak-Veblen-like notation.md` 的 6 例展开规则实现。本库**只做符号化展开与 LaTeX 序列化，绝不求值**。

头文件：`include/googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp`
命名空间：`googology::ordinal`

---

## 1. 分类（Taxonomy）

| 接口 | 返回值 |
|------|--------|
| `name()` | `"weak_veblen"` |
| `family()` | `Family::Ordinal` |
| `subfamily()` | `"veblen"` |
| `style()` | `"weak-like"` |

**类别**：**序数记号（ordinal）**，Veblen 子族。但 WeakVeblen **直承 `Notation`**（**不**继承 `OrdinalNotation`）：因为其条目是**序数对 `(a@b)`**，文章**未定义标准型**，故按"照文章写"原则不添加 `normalize` / `is_standard` / `isSuccessor` / `roots` / `limit` 等序数序列方法。是否引入标准型概念由用户决定。

---

## 2. 创建者 / 版本

| 接口 | 返回值 |
|------|--------|
| `creator()` | `"zahin"`（文章标题即 "zahin's weak-Veblen-like notation"） |
| `version()` | `"1"`（文章未给显式版本号） |

---

## 3. 能力（Capabilities）

`capabilities()` 声明支持：

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Compare` ✔（文章 §1.3 给出 `is_equal` / `is_greater` / `compare`）
- `Op::Expand` ✔

**不支持**（调用对应基类接口会抛 `UnsupportedOperation`）：
- `Op::ExpandTo` ✘ —— 头文件未覆盖 `expand_to`，落在基类抛 `UnsupportedOperation`。
- `Op::Normalize` / `Op::Successor` ✘ —— 文章未定义标准型/后继条款，故不声明；`normalize` / `isSuccessor` 等方法在本记号中**不存在**。

---

## 4. 继承的基类接口（`Notation`）

WeakVeblen 直接继承 `googology::Notation`（**不是** `OrdinalNotation`），因此：

- 拥有：`name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare`（已覆盖）/ `expand`（已覆盖）/ `comparable` / `print` / `operator<<` / `reduce`。
- **不拥有**（`OrdinalNotation` 才有）：`normalize` / `is_standard` / `isSuccessor` / `clone` / `roots` / `master_limit` / `limit`。

> 注意：本记号内部的 `Ordinal ord_` 是项目统一序数表达式类型（`core/Ordinal.hpp`，值类型，**非** `Notation` 分类成员）。`expand` 在内部重写该 `Ordinal` 并返回 `*this`；其中 `(a@b)` 表达式的展开（6 例）由 `Ordinal::expandWV` 实现，序算术（`+1`、`^`、`expand(b,n)`）由 `Ordinal` 核心提供。

---

## 5. 该记号自有 API（核心）

### 5.1 构造

```cpp
WeakVeblen() = default;                                  // 默认（未初始化表达式）
explicit WeakVeblen(const std::string& s);               // 解析 "a1@b1, a2@b2, ..."
explicit WeakVeblen(const std::vector<std::pair<Ordinal, Ordinal>>& c); // 直接由 (a@b) 对构造
```

### 5.2 覆盖的接口

```cpp
std::string name() const override;        // "weak_veblen"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "veblen"
std::string style() const override;       // "weak-like"
std::string creator() const override;     // "zahin"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void string_to_it(const std::string& s) override;   // 解析 "a1@b1, a2@b2, ..."
std::string to_string() const override;             // 渲染 (a@b) 表达式

WeakVeblen& expand(BigInt n) override;   // 按文章 6 例重写内部 Ordinal，返回 *this
```

### 5.3 比较

```cpp
int compare(const Notation& other) const override;   // 文章 §1.3：返回 1(>)、0(=)、-1(<)
```

- 定义在 WV **标准型**表达式上；非 `WeakVeblen` 实参抛 `NotComparable`。
- 实现为字典序（主键第二坐标 `@b`，次键 `@a`），见 `WeakVeblen.cpp` 的逐字对应实现。

### 5.4 基本列下标（不改动 `*this`）

```cpp
WeakVeblen operator[](BigInt n) const;   // A[n] = 序数 A 所代表的基本列第 n 项；等价于 expand(A,n)
```

返回副本，**不**修改 `*this`；重复下标安全。注意 `operator[]` 不是 `Capability` 位，它只是 `expand` 的语法糖。

**行为要点**
- `expand`：就地在 `*this` 上重写内部 `Ordinal` 并返回自身引用。
- `expand_to`：未覆盖，调用抛 `UnsupportedOperation`。
- 抛出的异常：`compare` 跨类型抛 `NotComparable`；`string_to_it` 遇非法输入抛解析异常。

---

## 6. 范例（C++ 用法片段）

```cpp
#include "googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// 例 1：构造并展开（不求值）
WeakVeblen a("(2@3, 0@1)");
std::cout << a.to_string() << "\n";       // 渲染 (a@b) 表达式
a.expand(1);
std::cout << a.to_string() << "\n";       // 一次展开后重写形式

// 例 2：基本列下标（不改原值）
WeakVeblen b("(0@1)");
WeakVeblen b1 = b[1];                      // 基本列第 1 项，b 不变
std::cout << b1.to_string() << "\n";

// 例 3：同类型比较；跨类型抛 NotComparable
WeakVeblen c("(1@2)");
WeakVeblen d("(1@3)");
int r = c.compare(d);                      // 字典序（主键 @b）
// c.compare(somePrss);   // 抛 NotComparable
```

---

## 7. 设计要点

- **记号即表达式，一律不求值**：`to_string()` 仅打印 LaTeX/符号；无 `Evaluate` / `ToOrdinal`。
- **直承 `Notation`，非 `OrdinalNotation`**：弱 Veblen 条目是序数对 `(a@b)`，文章未定义标准型，故**无** `normalize` / `is_standard` / `isSuccessor` / `limit` / `roots`。这与 PrSS / ε_pSS / ε_ωSS 不同（后三者是自然数**序列**记号，继承 `OrdinalNotation`）。
- **忠实实现（no added interpretation）**：展开严格照文章 6 例；序算术由 `core::Ordinal` 提供。`compare` 由文章 §1.3 给出故声明 `Compare`；`expand_to` 文章未定义故抛 `UnsupportedOperation`。
- `compare` 是良定义的序数序（标准型表达式上）；跨类型抛 `NotComparable`。

> 脚注：本文档所有函数签名以 `include/googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp` 头文件为准。
