# ε_pSS / ε_ωSS（阶差型序数序列记号，同一模块两种类型）

> 一句话定义：二者均出自同一篇文章（"epsilon_nSS & epsilon_omegaSS.md"），是**阶差型（difference）自然数序列**序数记号，继承 `OrdinalNotation`，共享 `baseVal_=1` 与同一套 `expandLen_` / `compare` / `normalize` 算法。唯一区别在每步追加量：ε_pSS 带参数 `p`（上限封顶）；ε_ωSS 无 `p`、把间隔 `q` 全量追加（无界，即 ε_pSS 的 ω-极限）。本库**只做符号化展开与 LaTeX 序列化，绝不求值**。

头文件：`include/googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp`
命名空间：`googology::ordinal`
同一模块两种类型：`EpspSS`（ε_pSS）/ `EpsOmegaSS`（ε_ωSS）。

---

## 1. 分类（Taxonomy）

| 接口 | `EpspSS` 返回 | `EpsOmegaSS` 返回 |
|------|---------------|-------------------|
| `name()` | `"epsilon_p_ss"` | `"epsilon_omega_ss"` |
| `family()` | `Family::Ordinal` | `Family::Ordinal` |
| `subfamily()` | `"sequence"` | `"sequence"` |
| `style()` | `"difference"` | `"difference"` |

**类别**：**序数记号（ordinal）**，阶差型自然数序列子族。两类型同模块、同基类 `OrdinalNotation` → `Notation`，携带标准型逻辑。

---

## 2. 创建者 / 版本

| 接口 | `EpspSS` | `EpsOmegaSS` |
|------|----------|--------------|
| `creator()` | `"zahin"` | `"zahin"` |
| `version()` | `"1"` | `"1"` |

---

## 3. 能力（Capabilities）

两个类的 `capabilities()` 相同，均声明支持：

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Normalize` ✔
- `Op::Compare` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Successor` ✔（声明具备 §12 后继条款；谓词见 `isSuccessor()`）

> `compare` 在文章未定义，但用户规定标准型下二者可按序列字典序比较（与 PrSS 一致，仅当两操作数均处于标准型时有效）；跨类型比较抛 `NotComparable`。

---

## 4. 继承的基类接口（与 PrSS 同，见 PrSS 文档 §4）

- `Notation`：`name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare`（已覆盖）/ `expand`（已覆盖）/ `expand_to`（已覆盖）/ `comparable` / `print` / `operator<<` / `reduce`。
- `OrdinalNotation`：`virtual void normalize()` / `bool isSuccessor() const`（`baseVal_=1`，序列以 1 结尾即后继）/ `bool is_standard() const` / `clone()`（已覆盖）/ `roots()`（已覆盖）。

> 标准型**统领定义**同 PrSS：`"标准表达式 ⇔ 可由某个极限表达式经有限次展开并取基本列前若干项得到。"` `epsilon_ss` 的用户指定极限表达式为 `(1, n)`，故规范起点为 `(1, a_2)`。

---

## 5. 该记号自有 API（核心）

### 5.1 `EpspSS`（ε_pSS）

#### 构造

```cpp
EpspSS();                                 // baseVal_ = 1
explicit EpspSS(BigInt p);                // 设置参数 p（追加量上限），baseVal_ = 1
explicit EpspSS(const std::string& s);    // 解析，baseVal_ = 1
EpspSS(BigInt p, const std::string& s);   // 设置 p 并解析，baseVal_ = 1
```

#### 参数

```cpp
void   set_p(BigInt p);                   // 设置参数 p
BigInt p() const;                         // 读取参数 p
```

#### 覆盖的接口

```cpp
std::string name() const override;        // "epsilon_p_ss"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "sequence"
std::string style() const override;       // "difference"
std::string creator() const override;     // "zahin"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void        string_to_it(const std::string& s) override;
std::string to_string() const override;

EpspSS& expand(BigInt n) override;        // 就地展开 n 步，返回 *this
EpspSS& expand_to(BigInt len) override;   // -> expandLen(A, len)，返回 *this

EpspSS operator[](BigInt n) const;        // A[n] = 基本列第 n 项；不改 *this（返回副本）

static EpspSS limit(BigInt n);            // 极限表达式 LIMIT=(1,ω) 第 n 项
static EpspSS master_limit();             // 主极限表达式 LIMIT（标记 is_master_limit_）

int compare(const Notation& other) const override;   // 序列字典序；跨类型抛 NotComparable

OrdinalNotation* clone() const override;  // return new EpspSS(*this);
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;  // { master_limit() }

friend std::istream& operator>>(std::istream&, EpspSS&);
```

### 5.2 `EpsOmegaSS`（ε_ωSS）

#### 构造

```cpp
EpsOmegaSS();                             // baseVal_ = 1
explicit EpsOmegaSS(const std::string& s);// 解析，baseVal_ = 1
```

#### 覆盖的接口

```cpp
std::string name() const override;        // "epsilon_omega_ss"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "sequence"
std::string style() const override;       // "difference"
std::string creator() const override;     // "zahin"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void        string_to_it(const std::string& s) override;
std::string to_string() const override;

EpsOmegaSS& expand(BigInt n) override;    // 就地展开 n 步，返回 *this
EpsOmegaSS& expand_to(BigInt len) override; // -> expandLen(A, len)，返回 *this

EpsOmegaSS operator[](BigInt n) const;    // A[n]；不改 *this（返回副本）

static EpsOmegaSS limit(BigInt n);        // LIMIT=(1,ω) 第 n 项
static EpsOmegaSS master_limit();         // 主极限表达式 LIMIT

int compare(const Notation& other) const override;
OrdinalNotation* clone() const override;  // return new EpsOmegaSS(*this);
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
friend std::istream& operator>>(std::istream&, EpsOmegaSS&);
```

**行为要点（两类型相同）**
- `expand` / `expand_to` / `normalize`：就地（in-place）修改 `*this`；`expand`/`expand_to` 返回自身引用。
- `operator[]`：返回副本，**不**改 `*this`。重复下标安全。
- `expand_to(len)` 等价于 `expandLen(A, len)`。
- 抛出的异常：`compare` 跨类型抛 `NotComparable`；越界下标按文章原样保留、防御性守卫抛异常（不静默回绕）；`normalize` 卡住/非标准输入时保持 `*this` 不变。

---

## 6. 范例（C++ 用法片段）

```cpp
#include "googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// 例 1：ε_pSS 构造（带参数 p）并展开
EpspSS a(5, "(1,3)");                     // p=5，解析序列
std::cout << a.to_string() << "\n";
a.expand(2);
std::cout << a.to_string() << "\n";

// 例 2：ε_ωSS 基本列下标（不改原值）
EpsOmegaSS b("(1,2)");
EpsOmegaSS b1 = b[1];                      // 基本列第 1 项，b 不变
std::cout << b1.to_string() << "\n";

// 例 3：规范化 + 标准型检测 + 同类型比较
EpspSS c("(1,3,2)");
c.normalize();
std::cout << std::boolalpha << c.is_standard() << "\n";
EpspSS d("(1,3,1)");
int r = c.compare(d);                     // 字典序；跨类型（如与 Prss）抛 NotComparable
```

---

## 7. 设计要点

- **记号即表达式，一律不求值**：`to_string()` 仅打印 LaTeX；无 `Evaluate` / `ToOrdinal`。
- **同一文章 = 同一类型 = 同一模块**：ε_pSS 与 ε_ωSS 同出一篇文，故同置 `epsilon_ss` 模块、共继承 `OrdinalNotation`、共享 `baseVal_=1` 与同一套算法；唯一区别是每步追加量（ε_pSS 封顶于 `p`，ε_ωSS 全量）。
- **标准型**：统领定义同 PrSS；`normalize()` 为规范化动作、`is_standard()` 为谓词、`isSuccessor()` 为无关的后继判定（序列以 1 结尾即后继）；用户指定极限表达式为 `(1, n)`。
- `compare` 在标准型下为良定义的序列字典序；跨类型抛 `NotComparable`。
- **忠实策略（no added interpretation）**：文章公式逐字实现；越界下标保留原写、防御性守卫抛异常而非静默回绕。

> 脚注：本文档所有函数签名以 `include/googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp` 头文件为准。
