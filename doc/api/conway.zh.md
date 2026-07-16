# Conway 链式箭头记号（Conway chained-arrow）

> 一句话定义：用 `→` 链表示超指数迭代的大数记号（规则 `[a->b]=a^b`，`X->1->Y=X`，`X->a->b = X->(X->a-1->b)->b-1`）。本库**只做符号化展开与 LaTeX 序列化，绝不求值**。

头文件：`include/googology/notations/number/conway/Conway.hpp`
命名空间：`googology::number`

---

## 1. 分类（Taxonomy）

| 接口 | 返回值 |
|------|--------|
| `name()` | `"conway"` |
| `family()` | `Family::Number` |
| `subfamily()` | `""` |
| `style()` | `""` |

**类别**：**大数记号（number）**。Conway 直承 `Notation`，**不**携带序数记号的标准型逻辑。

---

## 2. 创建者 / 版本

| 接口 | 返回值 |
|------|--------|
| `creator()` | `"John Conway"` |
| `version()` | `"1"` |

---

## 3. 能力（Capabilities）

`capabilities()` 声明支持：

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Compare` ✘（大数记号比较未定义）

不支持 `Compare`：调用 `compare` 抛 `NotComparable`。无 `Normalize` / `Successor`。

---

## 4. 继承的基类接口（`Notation`）

与 Knuth 完全相同（见 Knuth 文档 §4）：`name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare`（基类抛 `NotComparable`）/ `expand` / `expand_to` / `comparable` / `print` / `operator<<` / `reduce`。

> 同样：运算返回记号**自身**，永不返回字符串；没有任何数值求值接口。

---

## 5. 该记号自有 API（核心）

### 5.1 内部节点结构 `CNode`（公有嵌套结构体）

```cpp
struct CNode {
    bool isInt = true;                    // true=整数叶子；false=嵌套子链
    BigInt val = 0;                       // 整数值
    std::vector<CNode> sub;               // 子链元素

    static CNode value(BigInt v);                       // 构造整数叶子
    static CNode subchain(std::vector<CNode> s);        // 构造嵌套子链
};
```

### 5.2 构造与解析

```cpp
Conway() = default;
explicit Conway(const std::string& s);    // 等价默认构造后调用 string_to_it(s)
```

### 5.3 覆盖的接口

```cpp
std::string name() const override;        // "conway"
Family      family() const override;      // Family::Number
std::string creator() const override;     // "John Conway"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void        string_to_it(const std::string& s) override;
std::string to_string() const override;            // 输出 LaTeX，如 "3 \rightarrow 3 \rightarrow 2"

Conway& expand(BigInt n) override;        // 就地重写链 n 步（规则 2/3），返回 *this
Conway& expand_to(BigInt len) override;   // 展开到长度 len，返回 *this
```

### 5.4 流输入

```cpp
friend std::istream& operator>>(std::istream& is, Conway& c);
```

**行为要点**
- `expand` / `expand_to`：in-place 修改 `*this`，返回自身引用。单步 `stepOnce` 在链的最外层可应用处应用规则 2/3；若顶层无可应用规则但某嵌套子链有，则在子链内应用（`recurseSub` 驱动嵌套链的完全规约）。
- `to_string()`：只打印 LaTeX，绝不求值。
- 抛出的异常：`compare` 抛 `NotComparable`；解析失败抛 `std::invalid_argument`（由 `string_to_it`）。

---

## 6. 范例（C++ 用法片段）

```cpp
#include "googology/notations/number/conway/Conway.hpp"
#include <iostream>
using namespace googology;
using namespace googology::number;

// 例 1：构造与展开（不求值）
Conway c("3 -> 3 -> 2");
std::cout << c.to_string() << "\n";      // LaTeX: 3 \rightarrow 3 \rightarrow 2
c.expand(1);
std::cout << c.to_string() << "\n";      // 一次展开后重写形式

// 例 2：expand_to 到指定长度
Conway c2("4 -> 4 -> 4");
c2.expand_to(2);
std::cout << c2 << "\n";

// 例 3：能力查询
Conway c3("2 -> 3");
std::cout << std::boolalpha << c3.can(Op::Expand) << "\n";   // true
std::cout << c3.can(Op::Compare) << "\n";                     // false
```

---

## 7. 设计要点

- **记号即表达式，一律不求值**：`to_string()` 只打印 LaTeX；无 `Evaluate` / `ToOrdinal`。
- **大数记号 ≠ 序数记号**：Conway 表示整数/函数，大小在库内不可比较，跨族或同族比较均抛 `NotComparable`。
- `expand` / `expand_to` 是就地重写，返回 `*this`；要保留原值需先拷贝（默认拷贝对 `std::vector<CNode>` 做深拷贝）。
- 展开规则（`spec/notations/conway.md`）：`[a->b]=a^b`、`X->1->Y=X`、`X->a->b = X->(X->a-1->b)->b-1`。

> 脚注：本文档所有函数签名以 `include/googology/notations/number/conway/Conway.hpp` 头文件为准。
