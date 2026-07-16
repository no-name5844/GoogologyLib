# Knuth 上箭头记号（Knuth up-arrow）

> 一句话定义：用 `↑`（上箭头）表示超幂次迭代的大数记号；层数越高，增长速度越快。本库**只做符号化展开与 LaTeX 序列化，绝不把表达式坍缩成数值**。

头文件：`include/googology/notations/number/knuth/Knuth.hpp`
命名空间：`googology::number`

---

## 1. 分类（Taxonomy）

| 接口 | 返回值 |
|------|--------|
| `name()` | `"knuth"` |
| `family()` | `Family::Number` |
| `subfamily()` | `""`（未覆盖，使用基类默认空串） |
| `style()` | `""`（未覆盖，使用基类默认空串） |

**类别**：**大数记号（number）**。Knuth 直承通用基类 `Notation`，**不**携带序数记号的标准型逻辑（`normalize` / `is_standard` / `isSuccessor` / `limit` / `roots` 一概不存在）。

---

## 2. 创建者 / 版本

| 接口 | 返回值 |
|------|--------|
| `creator()` | `"Donald Knuth"` |
| `version()` | `"1"` |

---

## 3. 能力（Capabilities）

`capabilities()` 声明支持的操作（可用 `can(Op::X)` 查询）：

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Compare` ✘（大数记号的比较在库内**未定义**）

**不支持 `Compare`**：大数记号之间没有良定义的序关系。`Notation::compare()` 在基类中对任意 `Notation` 实参默认抛 `NotComparable`；Knuth 未覆盖 `compare`，因此调用即抛 `NotComparable`。也没有 `Op::Normalize` / `Op::Successor`——本记号无标准型、无后继概念。

---

## 4. 继承的基类接口（`Notation`）

Knuth 直接继承 `googology::Notation`，因此拥有以下全部基类成员（部分被本记号覆盖）：

- 分类：`name()` / `family()` / `subfamily()` / `style()`
- 元数据：`creator()` / `version()`
- 能力：`Capabilities capabilities() const` / `bool can(Op op) const`
- 字符串：`void string_to_it(const std::string&)` / `std::string to_string() const`
- 运算：`int compare(const Notation&) const`（基类默认抛 `NotComparable`）/ `Notation& expand(BigInt)` / `Notation& expand_to(BigInt)`
- 工具：`bool comparable() const`（返回 `can(Op::Compare)`，此处为 `false`）
- 输出：`std::ostream& print(std::ostream& = std::cout) const` / `friend std::ostream& operator<<(std::ostream&, const Notation&)`
- 规约：`Notation& reduce()`——反复 `expand(1)` 直到形态稳定（最多 1000 步），返回 `*this`

> 注意：`expand` / `expand_to` / `reduce` 返回记号**自身**（可继续展开或序列化），**永不**返回字符串。本库没有任何 `Evaluate` / `ToOrdinal` 之类的数值求值接口。

---

## 5. 该记号自有 API（核心）

### 5.1 内部节点结构 `KNode`（公有嵌套结构体）

```cpp
struct KNode {
    bool isVal = true;                                  // true=叶子常量；false=箭头节点
    BigInt val = 0;                                     // 叶子值
    BigInt baseVal = 0;                                 // 箭头：左操作数（常量 a）
    BigInt height = 1;                                  // 箭头：高度 c
    std::unique_ptr<KNode> exp;                         // 箭头：右操作数（子树）

    static KNode value(BigInt v);                       // 构造叶子节点
    static KNode arrow(BigInt base, BigInt h, KNode e); // 构造 a ↑^h e 节点
};
```

说明：Knuth 形式中左操作数恒为常量，因此只有指数（右操作数）需要是递归子树。展开后的形如 `a ↑^{c-1} (a ↑^c (b-1))` 也能存回**同一** `KNode` 类型，而非压平成字符串。

### 5.2 构造与解析

```cpp
Knuth() = default;                          // 默认空（未初始化表达式）
explicit Knuth(const std::string& s);       // 等价于默认构造后调用 string_to_it(s)
```

### 5.3 覆盖的接口

```cpp
std::string name() const override;          // 返回 "knuth"
Family      family() const override;        // 返回 Family::Number
std::string creator() const override;       // 返回 "Donald Knuth"
std::string version() const override;       // 返回 "1"

Capabilities capabilities() const override; // 见 §3

void        string_to_it(const std::string& s) override; // 解析为内部 AST
std::string to_string() const override;                  // 输出 LaTeX，如 "2 \uparrow\uparrow 3"

Knuth& expand(BigInt n) override;    // 就地（in-place）对 AST 做 n 步重写，返回 *this
Knuth& expand_to(BigInt len) override; // 展开到长度 len（等价地多次 expand），返回 *this
```

### 5.4 流输入

```cpp
friend std::istream& operator>>(std::istream& is, Knuth& k);  // 调用 string_to_it
```

**行为要点**
- `expand` / `expand_to`：in-place 修改 `*this`，返回自身引用（可链式继续展开）。
- `to_string()`：只打印 LaTeX 形式，**绝不**把表达式求值成数。
- 抛出的异常：`string_to_it` 遇非法输入抛 `std::invalid_argument`（递归下降解析器，右结合 `^` 塔）；`compare` 抛 `NotComparable`；`reduce` 内部依赖 `expand` / `to_string`，不会自行抛数值类异常。

---

## 6. 范例（C++ 用法片段）

```cpp
#include "googology/notations/number/knuth/Knuth.hpp"
#include <iostream>
using namespace googology;
using namespace googology::number;

// 例 1：构造并展开（不求值，仅重写 AST）
Knuth k("2 ^^ 3");                 // 解析自 ASCII 形式；内部为 2 ↑↑ 3
std::cout << k.to_string() << "\n"; // 打印 LaTeX: 2 \uparrow\uparrow 3
k.expand(1);
std::cout << k.to_string() << "\n"; // 一次展开后的重写形式（仍是符号）

// 例 2：expand_to 展开到指定长度
Knuth k2("3 ^^ 2 ^^ 2");
k2.expand_to(3);
std::cout << k2 << "\n";           // operator<< 调用 to_string()

// 例 3：能力查询与跨族比较
Knuth k3("2 ^ 3");
if (!k3.can(Op::Compare))
    std::cout << "Knuth is not comparable\n"; // 大数记号无比较
// k3.compare(other);  // 任意调用都会抛 NotComparable
```

---

## 7. 设计要点

- **记号即表达式，一律不求值**：序列化 `to_string()` 只打印 LaTeX/符号，绝不坍缩成数。本库没有 `Evaluate` / `ToOrdinal` 这类数值求值接口（见 `core/Capability.hpp` 注释）。
- **大数记号 ≠ 序数记号**：Knuth 表示整数（或函数），其大小在库内**不可比较**——跨族或同族比较都抛 `NotComparable`。
- `expand` / `expand_to` 是**就地重写**：它们修改 `*this` 并返回自身引用；如需保留原值，请先拷贝（默认拷贝构造对 `unique_ptr` 子树做深拷贝）。
- 解析语法（右结合箭头塔）：`expr := INT ( '^'+ expr )?`，故 `a ^^ b ^^ c` 解析为 `a ↑^{2} (b ↑^{2} c)`。

> 脚注：本文档所有函数签名以 `include/googology/notations/number/knuth/Knuth.hpp` 头文件为准。
