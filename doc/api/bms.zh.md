# BMS / BM4 / BM1 / BM3.3（Bashicu 矩阵系统，序数记号）

> 一句话定义：把按列书写的非负整数**矩阵**作为表达式，通过对**末列**做一步展开（基本列前驱 $FS_n$）递归生成序数。所有版本共用 §0 骨架，差异只在**父项条件**与**上升度 $a_{k,m}$**。本库**一律不求值、只展开一步**，故终止性 / 良基性被刻意忽略（用户指令：把良基丢进垃圾桶）。

头文件：`include/googology/notations/ordinal/matrix/bms/BMS.hpp`
实现文件：`src/notations/ordinal/matrix/bms/BMS.cpp`
命名空间：`googology::ordinal`

---

## 1. 分类（Taxonomy）

| 接口 | 返回值（BM4 为例；BM1 / BM3.3 同名 `name()` 不同）|
|------|--------|
| `name()` | `"bm4"`（`BM1`→`"bm1"`，`BM3_3`→`"bm3.3"`）|
| `family()` | `Family::Ordinal` |
| `subfamily()` | `"bms"` |
| `style()` | `"matrix"` |

**类别**：**序数记号（ordinal）**——矩阵经 `Trans()` 映射到序数（见 `spec/notations/bms.zh.md` §0.2.2）。`compare()` 用**语法全序（列主序字典序）**，是容器 / 排序用，**非**真正序数序（真序数序需 `Trans()`，可能不终止——已丢进垃圾桶）。

---

## 2. 创建者 / 版本

| 接口 | 返回值 |
|------|--------|
| `creator()` | `""`（待用户在计划模程确认后回填；源文章署 Bashicu）|
| `version()` | `""`（待补；BM4=2018，BM1=2014，BM3.3=2019）|

> 子类各自覆写 `name()`；`creator()` / `version()` 当前沿用基类默认 `""`（库惯例：未知待补）。

---

## 3. 能力（Capabilities）

`capabilities()` 声明支持：

- `Op::FromString` ✔（`string_to_it` 解析 `(a,b,...)(c,d,...)`）
- `Op::ToString` ✔（`to_string` 逆序列化回同格式）
- `Op::Expand` ✔（`expand(n)` = 第 $n$ 项基本列 $FS_n$，就地改写并返回 `*this`）
- `Op::Compare` ✔（语法全序；跨类型抛 `NotComparable`）
- `Op::Normalize` ✔（强制 §0.1 三条件；不满足保持 `*this` 不变，库惯例）

**不支持**：
- `Op::Successor` ✘ —— BMS 无简单后继概念，`isSuccessor()` 恒返回 `false`（非 `OrdinalNotation` 虚方法，故不写 `override`）。
- `Op::ExpandTo` ✘ —— 未实现（基本列单步 `expand` 已够用）。

> 判定：`bms.can(Op::X)`（继承 `Notation::can`，包 `capabilities().has(Op::X)`）返回 `bool`；或 `bms.capabilities().has(Op::X)`。

---

## 4. 继承的基类接口（`OrdinalNotation` / `Notation`）

BMS 继承 `googology::ordinal::OrdinalNotation`（→ `Notation`），因此：

- 拥有：`name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare`（已覆盖）/ `expand`（已覆盖）/ `comparable` / `print` / `operator<<` / `reduce`。
- 序数记号成员（均为 `OrdinalNotation` 虚方法，BMS 已覆盖其中相关者）：`normalize` / `is_standard` / `isSuccessor`（非虚，BMS 提供自身定义）/ `clone` / `roots`。

> `is_standard()` 为 **`virtual`**（符合"标准型判定逐记号"）：BMS 用 §0.1 三条件直接句法判定；`OrdinalNotation` 默认走 §12 引擎，此处被 BMS 覆盖。

---

## 5. 该记号自有 API（核心）

### 5.1 公共成员（基类 `BMS`）

```cpp
std::vector<std::vector<BigInt>> cols_;   // 列主序矩阵：cols_[x] = 第 x 列（自上而下）
```

所有版本共享同一份矩阵存储与 §0 骨架；差异通过两个 **`virtual` 钩子**暴露：

```cpp
// 父项 p_k(m)：最大 p<m 满足 S_{p,k}<S_{m,k}，且 (k==0 ? true : ascensionDegree(k-1,m,p)==1)
virtual int  parentOf(int k, int m) const;
// 上升度 a_{k,m}(t)：t 是 m 在 p_k 下的祖先（含 m 自身）时为真
virtual bool ascensionDegree(int k, int m, int t) const;
```

### 5.2 覆盖的接口

```cpp
std::string name() const override;            // 各子类："bm4" / "bm1" / "bm3.3"
Family      family() const override;           // Family::Ordinal
std::string subfamily() const override;    // "bms"
std::string style() const override;          // "matrix"

Capabilities capabilities() const override;

void  string_to_it(const std::string& s) override;  // 解析 (a,b,...)(c,d,...) 格式
std::string to_string() const override;            // 逆序列化回同格式

BMS& expand(BigInt n) override;          // FS_n：就地改写 *this，返回自身（协变返回）
int  compare(const Notation& other) const override;  // 语法全序；非 BMS 实参抛 NotComparable

void normalize() override;                 // 强制 §0.1；不满足则保持原值
bool is_standard() const override;         // §0.1 三条件（句法）直接判定
bool isSuccessor() const;                  // 非虚：恒 false
OrdinalNotation* clone() const override = 0;        // 各子类实现
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;  // 返回同类型空矩阵种子
```

### 5.3 子类（版本差异）

| 类 | 差异（相对 BM4）|
|----|------|
| `BM4` | 唯一完整规则；`parentOf` 含上行祖先检查，`ascensionDegree` 逐行独立判定祖先链（`(p_k)^c(m)=t`）。|
| `BM1` | `parentOf` **去掉祖先检查**（仅按首行判定）；`ascensionDegree` 由首行统一 $a_{k,m}(t)=a_{0,m}(t)$。终止性：否。|
| `BM3_3` | `parentOf` 同 BM4；`ascensionDegree` 去掉 "父项 = 坏根" 子句（$a_{k,m}=1\iff a_{k,\mathrm{parent}(m)}=1\land\mathrm{parent}>r$）。终止性：未证明。|

> 其余版本（BM2 / BM2.1 / BM2.2 / BM3 / BM3.1 / BM3.2 / PsiCubed2 / Idealized）因 $a_{k,m}$ 精确公式"仅见于 basmat 源码、公开文档未给"而**未实现**；头注释标明待补，后续以子类接入（同 `parentOf` / `ascensionDegree` 钩子）。

### 5.4 比较语义

```cpp
int compare(const Notation& other) const override;  // 列主序字典序（语法全序）
```
- 同类型（同为某 BMS 子类）：按列主序（先比第 0 列逐行，再第 1 列……）字典比较；长短不一以存在列为准，越界行按 $0$。返回 $-1/0/+1$。
- **跨类型**（如实参不是 `BMS` 子类）：抛 `NotComparable`（`compare` 非真序数序，库不跨族比较）。

### 5.5 标准型 / 规范化

- `is_standard()`：**句法**判定 §0.1 三条件（首列全 0 / 每列非增 / 同行不超前超 1），不依赖展开或终止性。
- `normalize()`：若已满足 §0.1 则无操作；若不满足（非法矩阵），按库惯例**保持 `*this` 不变**（不抛、不坍缩）。
- `isSuccessor()`：恒 `false`（BMS 无简单后继概念）。

---

## 6. 范例（C++ 用法片段）

```cpp
#include "googology/notations/ordinal/matrix/bms/BMS.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// 例 1：BM4 一步展开（基本列 FS_n）
BM4 a("(0,0,0)(1,1,1)(2,2,0)");
a.expand(1);
std::cout << a.to_string() << "\n";   // (0,0,0)(1,1,1)  == FS_1
BM4 b("(0,0,0)(1,1,1)(2,2,0)");
b.expand(2);
std::cout << b.to_string() << "\n";   // (0,0,0)(1,1,1)(2,1,1) == FS_2

// 例 2：单列递减（末列无父项，r<0 分支）
BM4 c("(5)");
c.expand(3);
std::cout << c.to_string() << "\n";   // (2)   （5 - 3）

// 例 3：BM3.3 笔记示例首项
BM3_3 d("(0,0,0)(1,1,1)(2,1,0)(1,1,1)");
d.expand(1);
std::cout << d.to_string() << "\n";   // (0,0,0)(1,1,1)(2,1,0)(1,1,0)

// 例 4：能力 / 标准型 / 跨类型比较
BM4 e("(0,0,0)(1,1,1)(2,2,0)");
std::cout << e.is_standard() << "\n";          // 1（满足 §0.1）
std::cout << e.capabilities().has(Op::Expand) << "\n";  // 1
bool threw = false;
try { BM4 x("(1)"); /* 与非 BMS 比较 */ }
catch (const NotComparable&) { threw = true; }
```

---

## 7. 设计要点

- **记号即表达式，一律不求值**：`to_string()` 只打印矩阵符号（`(a,b,...)(c,d,...)`）；本库**没有** `Evaluate` / `ToOrdinal` 数值求值接口。`expand(n)` 只产出第 $n$ 项基本列 $FS_n(S)$（`spec/notations/bms.zh.md` §0.2.2），不递归迭代、不坍缩成数。
- **基类 + 子类框架（用户指令）**：基类 `BMS` 实现 §0 共用骨架（列主序存储 + §0.1 标准型三条件 + §0.2.1 共享辅助 + §0.2.2 expand 骨架），通过 `parentOf` / `ascensionDegree` 两个 `virtual` 钩子暴露版本差异；`BM4` / `BM1` / `BM3_3` 作为子类 override 钩子。新增版本只需加子类。
- **终止性 / 良基性：已丢进垃圾桶**：用户明确指令忽略。库只做"一步展开"（与 Prss / EpspSS 一致），不证明也不依赖终止性。`compare()` 用**语法全序**（列主序字典序）而非真序数序，跨类型抛 `NotComparable`。
- **存储**：列主序；`S_{x,y}` = `get_(x,y)`，访问越界行按 $0$（列可不等高，短列下方视为 $0$）。`BigInt` 即 `int64_t` 别名（仅展开索引 / 解析参数用）。
- **未实现版本**：BM2 / BM2.1 / BM2.2 / BM3 / BM3.1 / BM3.2 / PsiCubed2 / Idealized 因 $a_{k,m}$ 精确公式仅见于 basmat 源码、公开文档未给，头注释标明待补，暂不接入。

> 脚注：本文档所有函数签名以 `include/googology/notations/ordinal/matrix/bms/BMS.hpp` 头文件为准；展开算法细节见 `spec/notations/bms.zh.md`。
