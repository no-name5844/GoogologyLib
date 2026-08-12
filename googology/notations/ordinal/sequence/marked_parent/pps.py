"""
PPS (Parented Predecessor Sequence) — 标记父项型 (marked_parent) 序数序列记号。

由 318`4 创造。PPS 系列 = PPS1（原始）+ PPS2 + PPS4 + 变体 wPPS4 / tPPS4 /
fPPS4；PPS3 / ePPS4 / sPPS4 资料缺失（N/A），不实现。已知结论（外部）：
良序极限 = ζ₀；PPS2 为失败的弱化修复；PPS4 旨在删除无穷降链。本库一律
不求值，expand 只产出一步基本列。

公共骨架（全部变体一致）：
  序列 A = (a_1, ..., a_n)，首项是第 1 项；极限表达式 0,1,2,3,4,5,...
  末项值 x = a_n，末项列标 y = n，坏根 = 第 x 项，坏根值 b = a_x，L = y - x。
  1. 末项为 0 → 后继序数（去掉末项）。
  2. 按变体替换末项（见 _replace_last）。
  3. 复制：对任意 i > x，第 i+L 项由第 i 项确定：a_i >= x → a_i + L，否则 a_i。
  基本列[n] = 展开到第 y + nL - 1 项（n 从 1 起）。库 0 索引：expand(k) =
  基本列[k+1] = expandLen((k+1)L - 1)；expand_to(M) = expandLen(A, M)。

变体差异（仅末项替换 / 强展开复制）：
  PPS1  存在值 = b 的项 → 换为 b；否则减 1。
  PPS2  存在值 = b 的项（取最右，列标 k）时，比较 a_{k+j} 与 a_{c+j}
        （c = x），首个不同处前者更大才换 b，否则减 1。
  PPS4  存在值 = b → 弱展开（换 b）；否则强展开：第 b 列与第 x 列之间
        （不含）最右侧值 = b 的项，末项换为该列标；找不到 → 等同弱展开。
  wPPS4 注记称把 PPS4 强展开判定由 ≤ b 收紧为 = b（PPS4 正文即 = b，
        按字面实现后二者一致；差异来源见 C++ spec）。
  tPPS4 wPPS4 + 强展开时由末项复制出的项加阶差 L。
  fPPS4 tPPS4 + 强展开时末项第一次复制为坏根列标 x。
"""

from enum import Enum

from googology.core import Op, Capabilities, Family, Notation, NotComparable


class PpsVariant(Enum):
    """PPS 变体枚举。"""
    PPS1 = "pps1"
    PPS2 = "pps2"
    PPS4 = "pps4"
    WPPS4 = "wpps4"
    TPPS4 = "tpps4"
    FPPS4 = "fpps4"

    @property
    def version(self) -> str:
        return {"pps1": "1", "pps2": "2", "pps4": "4",
                "wpps4": "4w", "tpps4": "4t", "fpps4": "4f"}[self.value]


class Pps(Notation):
    """PPS 基类：variant + 公共展开骨架。各变体子类仅设定 _variant。"""

    _variant = PpsVariant.PPS1

    def __init__(self, s: str = ""):
        self._seq: list[int] = []
        self._is_master_limit = False
        if s:
            self.string_to_it(s)

    # --- taxonomy ---
    @property
    def name(self) -> str:
        return self._variant.value

    @property
    def family(self) -> Family:
        return Family.Ordinal

    @property
    def subfamily(self) -> str:
        return "sequence"

    @property
    def style(self) -> str:
        return "marked_parent"

    @property
    def creator(self) -> str:
        return "318`4"

    @property
    def version(self) -> str:
        return self._variant.version

    @property
    def capabilities(self) -> Capabilities:
        c = Capabilities()
        for op in (Op.FromString, Op.ToString, Op.Normalize,
                   Op.Compare, Op.Expand, Op.ExpandTo, Op.Successor):
            c.set(op)
        return c

    # --- 辅助 ---
    def _rightmost_eq(self, lo: int, hi: int, b: int) -> int:
        """开区间 (lo, hi) 列标内最右侧值 = b 的列标；无则 0。"""
        for j in range(hi - 1, lo, -1):
            if 1 <= j <= len(self._seq) and self._seq[j - 1] == b:
                return j
        return 0

    def _tail_cmp(self, k: int, c: int) -> int:
        """字典序比较 a_{k+j} 与 a_{c+j}（j = 0,1,2,...）。"""
        y = len(self._seq)
        j = 0
        while True:
            ik, ic = k + j, c + j
            ok_k = 1 <= ik <= y
            ok_c = 1 <= ic <= y
            if not ok_k and not ok_c:
                return 0
            if not ok_k:
                return -1
            if not ok_c:
                return 1
            if self._seq[ik - 1] != self._seq[ic - 1]:
                return 1 if self._seq[ik - 1] > self._seq[ic - 1] else -1
            j += 1

    def _replace_last(self, x: int, b: int, y: int) -> tuple:
        """按变体替换末项，返回 (新末项值, strong)。"""
        v = self._variant
        if v is PpsVariant.PPS1:
            k = self._rightmost_eq(x, y, b)
            return (b if k != 0 else x - 1), False
        if v is PpsVariant.PPS2:
            k = self._rightmost_eq(x, y, b)
            if k != 0 and self._tail_cmp(k, x) > 0:
                return b, False
            return x - 1, False
        # PPS4 家族：弱/强展开二分
        k = self._rightmost_eq(x, y, b)
        if k != 0:
            return b, False
        j = self._rightmost_eq(b, x, b)
        return (j if j != 0 else b), True

    def _copied(self, src: int, x: int, L: int, y: int, i: int, strong: bool) -> int:
        """复制规则：源位置 i（1-based）处值 src 的映射。"""
        base = src + L if src >= x else src
        if not strong:
            return base
        if self._variant in (PpsVariant.TPPS4, PpsVariant.FPPS4):
            if i % L == y % L:  # 末项链（源 i = y, y+L, y+2L, ...）
                if self._variant is PpsVariant.FPPS4 and i == y:
                    return x
                return src + L
        return base

    def _expand_len(self, M: int) -> None:
        """expandLen(A, M)：替换末项 + 追加 M 项（源 i = p + y - L）。"""
        if not self._seq:
            return
        y = len(self._seq)
        x = self._seq[-1]
        if x == 0:
            self._seq.pop()
            return
        if x >= y:
            raise ValueError("Pps: 坏根 = 第 x 项不存在（x >= y）")
        b = self._seq[x - 1]
        L = y - x
        new_last, strong = self._replace_last(x, b, y)
        self._seq[y - 1] = new_last
        for p in range(1, M + 1):
            i = p + y - L
            if i < 1 or i > len(self._seq):
                raise ValueError("Pps: 源索引越界")
            self._seq.append(self._copied(self._seq[i - 1], x, L, y, i, strong))

    # --- string ---
    def string_to_it(self, s: str) -> None:
        t = "".join(ch for ch in s if not ch.isspace())
        while t and t[0] in "([{":
            t = t[1:]
        while t and t[-1] in ")]}":
            t = t[:-1]
        self._seq = [int(p) for p in t.split(",") if p] if t else []

    def to_latex(self) -> str:
        if self._is_master_limit:
            return "(0, 1, 2, …)"
        return "(" + ", ".join(str(v) for v in self._seq) + ")"

    # --- operations ---
    def expand(self, n: int = 1) -> None:
        """库 0 索引：expand(k) = 基本列[k+1] = expandLen((k+1)L - 1)。"""
        if self._is_master_limit:
            self._seq = list(range(n))
            self._is_master_limit = False
            return
        if not self._seq:
            return
        x = self._seq[-1]
        if x == 0:
            self._seq.pop()
            return
        y = len(self._seq)
        if x >= y:
            raise ValueError("Pps: 坏根 = 第 x 项不存在（x >= y）")
        L = y - x
        self._expand_len((n + 1) * L - 1)

    def expand_to(self, length: int) -> None:
        self._expand_len(length)

    def __getitem__(self, n: int) -> "Pps":
        clone = self._clone()
        clone.expand(n)
        return clone

    def _clone(self) -> "Pps":
        c = self.__class__()
        c._seq = list(self._seq)
        c._is_master_limit = self._is_master_limit
        return c

    def compare(self, other: Notation) -> int:
        if not isinstance(other, Pps):
            raise NotComparable(self.name)
        if type(self) is not type(other):
            raise NotComparable(self.name)
        if self._is_master_limit or other._is_master_limit:
            if self._is_master_limit and other._is_master_limit:
                return 0
            return 1 if self._is_master_limit else -1
        a, b = self._seq, other._seq
        for x, y in zip(a, b):
            if x > y:
                return 1
            if x < y:
                return -1
        if len(a) > len(b):
            return 1
        if len(a) < len(b):
            return -1
        return 0

    @property
    def is_successor(self) -> bool:
        return (not self._is_master_limit) and bool(self._seq) and self._seq[-1] == 0

    # --- §12 limit API ---
    @classmethod
    def limit(cls, n: int) -> "Pps":
        p = cls()
        p._seq = list(range(n))
        return p

    @classmethod
    def master_limit(cls) -> "Pps":
        p = cls()
        p._is_master_limit = True
        return p


class Pps1(Pps):
    _variant = PpsVariant.PPS1


class Pps2(Pps):
    _variant = PpsVariant.PPS2


class Pps4(Pps):
    _variant = PpsVariant.PPS4


class WPps4(Pps):
    _variant = PpsVariant.WPPS4


class TPps4(Pps):
    _variant = PpsVariant.TPPS4


class FPps4(Pps):
    _variant = PpsVariant.FPPS4
