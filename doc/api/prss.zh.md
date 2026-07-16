# PrSS（原初序列系统 / Primitive Sequence System，阶差型）

> 一句话定义：阶差型（difference）自然数序列记号 `A=(a_1,...,a_n)`，满足 `a_{i+1}-a_i ≤ 1`、`a_1=0`、平台后非增。本库**只做符号化展开与 LaTeX 序列化，绝不求值**。

头文件：`include/googology/notations/ordinal/sequence/difference/prss/Prss.hpp`
命名空间：`googology::ordinal`

---

## 1. 分类（Taxonomy）

| 接口 | 返回值 |
|------|--------|
| `name()` | `"prss"` |
| `family()` | `Family::Ordinal` |
| `subfamily()` | `"sequence"` |
| `style()` | `"difference"` |

**类别**：**序数记号（ordinal）**，阶差型自然数序列子族。`Prss` 继承 `OrdinalNotation` → `Notation`，因此携带标准型逻辑。

---

## 2. 创建者 / 版本

| 接口 | 返回值 |
|------|--------|
| `creator()` | `""` （未覆盖基类默认空串；**待用户后续补**注明命名者） |
| `version()` | 未覆盖，基类默认 `""` |

> 注：PrSS 头文件未覆盖 `creator()` / `version()`，故二者返回空串。命名者信息以用户后续的"计划模程"为准。

---

## 3. 能力（Capabilities）

`capabilities()` 声明支持：

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Normalize` ✔
- `Op::Compare` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Successor` ✔（声明本记号具备 §12 后继条款；具体谓词见 `isSuccessor()`）

不支持的运算（未在能力集）调用相应接口会抛 `UnsupportedOperation`（本记号已覆盖上述全部，故基本实现均可用）。

---

## 4. 继承的基类接口

### 4.1 来自 `Notation`（通用）

`name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can(Op)` / `string_to_it` / `to_string` / `compare`（已覆盖）/ `expand`（已覆盖）/ `expand_to`（已覆盖）/ `comparable` / `print` / `operator<<` / `reduce`。

### 4.2 来自 `OrdinalNotation`（序数中间基类）

```cpp
virtual void normalize();                 // 规范化（标准型化）：就地把 *this 重写为标准型
bool isSuccessor() const;                 // 后继判定（与标准型无关）：序列以 baseVal_(=0) 结尾则为后继
bool is_standard() const;                 // 标准型检测谓词：运行 §12 通用判定引擎
virtual OrdinalNotation* clone() const = 0;   // 多态拷贝（Prss 已覆盖）
virtual std::vector<std::shared_ptr<OrdinalNotation>> roots() const = 0; // 顶层极限表达式（Prss 已覆盖）
```

> 标准型**统领定义**（适用于所有序数记号）："一个表达式是【标准表达式】⇔ 它可以通过某个【极限表达式】，经【有限次展开（expand）】并【取基本列前若干项】的方式得到。"
>
> - `normalize()` = **动词**：把表达式化归为标准的动作（逐记号重写算法，PrSS 的 `baseVal_=0`）。
> - `is_standard()` = **谓词**：判定"是否已处于标准型"。
> - `isSuccessor()` = 后继判定，**与标准型完全无关**（仅看序列末元素是否等于 `baseVal_`）。

---

## 5. 该记号自有 API（核心）

### 5.1 构造与解析

```cpp
Prss() = default;
explicit Prss(const std::string& s);      // 等价默认构造后调用 string_to_it(s)
```

### 5.2 覆盖的接口

```cpp
std::string name() const override;        // "prss"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "sequence"
std::string style() const override;       // "difference"

Capabilities capabilities() const override;

void string_to_it(const std::string& s) override;  // 解析序列文本
std::string to_string() const override;            // 输出 LaTeX

Prss& expand(BigInt n) override;          // 就地展开 n 步（基本列第 n 项），返回 *this
Prss& expand_to(BigInt len) override;     // 展开到长度 len，返回 *this
```

### 5.3 基本列下标（不改动 `*this`）

```cpp
Prss operator[](BigInt n) const;          // A[n] = 序数 A 所代表的基本列第 n 项
```

等价于 `expand(A, n)`，但**不修改** `*this`（返回副本），故重复 `A[1]`、`A[2]`、… 安全。

### 5.4 §12 极限表达式 API

```cpp
static Prss limit(BigInt n);              // 极限表达式 LIMIT=(0,1,2,3,...) 的第 n 项
static Prss master_limit();               // 主极限表达式 LIMIT 本身（标记 is_master_limit_）
```

`LIMIT = (0,1,2,3,...)`：`limit(0)=()`、`limit(1)=(0)`、`limit(2)=(0,1)`、…、`limit(n)=(0,1,...,n-1)`。满足 `LIMIT.expand(m) == limit(m)`；`is_standard()` 通过 `roots()` 把 BFS 播种于 `LIMIT`（主极限视为所有标准 Prss 表达式的上确界）。

### 5.5 比较

```cpp
int compare(const Notation& other) const override;   // 序列上的字典序（序数序）
```

- 返回 `-1/0/1`（`<` / `=` / `>`）。
- 非 `Prss` 实参（跨类型）抛 `NotComparable`。
- 主极限表达式比较为**上确界**（大于任何有限序列）。

### 5.6 §12 标准型判定支持

```cpp
OrdinalNotation* clone() const override;  // return new Prss(*this);
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;  // { master_limit() }
```

**行为要点**
- `expand` / `expand_to` `normalize`：均为**就地（in-place）**修改 `*this`，返回自身引用 / 无返回值。
- `operator[]`：返回副本，**不**改动 `*this`。
- 抛出的异常：`compare` 对跨类型实参抛 `NotComparable`；`normalize` 在卡住或非标准输入时**保持 `*this` 不变**（不抛异常，仅还原）；`string_to_it` 遇非法输入可能抛解析异常。

---

## 6. 范例（C++ 用法片段）

```cpp
#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// 例 1：构造、展开、序列化（不求值）
Prss a("(0,1,2)");
std::cout << a.to_string() << "\n";       // LaTeX 序列
a.expand(3);
std::cout << a.to_string() << "\n";       // 展开后形式

// 例 2：基本列下标（不改原值）
Prss b("(0,1,2)");
Prss b1 = b[1];                           // 基本列第 1 项，b 自身不变
std::cout << b1.to_string() << "\n";

// 例 3：规范化 + 标准型检测 + 比较
Prss c("(0,1,2,1,2)");                    // 可能非标准
c.normalize();                            // 就地化归标准型
bool ok = c.is_standard();                // 谓词：是否处于标准型
std::cout << std::boolalpha << ok << "\n";
Prss d("(0,1,2,3)");
int r = c.compare(d);                     // 字典序；跨类型会比较抛 NotComparable
```

---

## 7. 设计要点

- **记号即表达式，一律不求值**：`to_string()` 只打印 LaTeX；无 `Evaluate` / `ToOrdinal`。
- **标准型（noun）**：由 §12 统领定义统辖；`normalize()` 是逐记号的规范化动作，`is_standard()` 是标准型检测谓词，`isSuccessor()` 是与之无关的后继判定。`Prss` 的 `baseVal_=0`。
- **极限表达式 / roots**：`master_limit()` 标记 `is_master_limit_`，其 `expand(m)==limit(m)`，并被 `roots()` 用作 §12 BFS 种子（上确界）。
- `compare` 是**良定义的序数序**（序列字典序）；跨族/跨类型比较抛 `NotComparable`。大数记号（Knuth/Conway）无此能力。

> 脚注：本文档所有函数签名以 `include/googology/notations/ordinal/sequence/difference/prss/Prss.hpp` 头文件为准。`creator()` 返回空串为基类默认，命名者待用户后续补。
