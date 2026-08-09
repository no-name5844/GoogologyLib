"""
Knuth up-arrow notation (高德纳箭头).

  a ^c b = a ^ b                 (c = 1)
  a ^c b = a                     (b = 1)
  a ^c b = a ^(c-1) (a ^c (b-1)) (c > 1, b > 1)
"""

from googology.core import Op, Capabilities, Family, Notation, UnsupportedOperation


class KNode:
    """One node of a Knuth up-arrow expression AST."""

    def __init__(self, is_val: bool, val: int = 0, base: int = 0, height: int = 0, exp: 'KNode' = None):
        self.is_val = is_val
        self.val = val
        self.base = base
        self.height = height
        self.exp = exp

    @staticmethod
    def value(v: int) -> 'KNode':
        return KNode(True, val=v)

    @staticmethod
    def arrow(base: int, h: int, e: 'KNode') -> 'KNode':
        return KNode(False, base=base, height=h, exp=e)

    def tex(self) -> str:
        if self.is_val:
            return str(self.val)
        bt = str(self.base)
        et = self.exp.tex() if self.exp else ""
        arr = "\\uparrow" if self.height == 1 else f"\\uparrow^{{{self.height}}}"
        exp_has_arrow = self.exp and not self.exp.is_val
        if exp_has_arrow:
            return f"{bt} {arr} ({et})"
        return f"{bt} {arr} {et}"

    def step(self) -> 'KNode':
        if self.is_val:
            return KNode.value(self.val)
        if self.exp and self.exp.is_val:
            b = self.exp.val
            if b == 1:
                return KNode.value(self.base)  # a ^c 1 = a
            if self.height == 1:
                return KNode.arrow(self.base, self.height, KNode.value(b))
            inner = KNode.arrow(self.base, self.height, KNode.value(b - 1))
            return KNode.arrow(self.base, self.height - 1, inner)
        # recurse into the rightmost sub-term
        return KNode.arrow(self.base, self.height, self.exp.step())

    @staticmethod
    def parse(s: str) -> 'KNode':
        s = s.strip()
        pos = s.find('^')
        if pos == -1:
            if not s:
                raise ValueError("Knuth: empty operand")
            return KNode.value(int(s))
        a = int(s[:pos])
        j = pos
        c = 0
        while j < len(s) and s[j] == '^':
            c += 1
            j += 1
        rest = s[j:].strip()
        if not rest:
            raise ValueError("Knuth: dangling arrows (no exponent)")
        exp = KNode.parse(rest)
        return KNode.arrow(a, c, exp)


class Knuth(Notation):
    """Knuth up-arrow notation."""

    def __init__(self, root: KNode = None):
        self._root = root if root else KNode.value(0)

    # --- taxonomy ---
    @property
    def name(self) -> str:
        return "knuth"

    @property
    def family(self) -> Family:
        return Family.Number

    @property
    def creator(self) -> str:
        return "Donald Knuth"

    @property
    def version(self) -> str:
        return "1"

    # --- capability ---
    @property
    def capabilities(self) -> Capabilities:
        c = Capabilities()
        c.set(Op.FromString)
        c.set(Op.ToString)
        c.set(Op.Expand)
        c.set(Op.ExpandTo)
        return c

    @classmethod
    def from_string(cls, s: str) -> 'Knuth':
        k = cls()
        k.string_to_it(s)
        return k

    # --- string conversion ---
    def string_to_it(self, s: str) -> None:
        t = s.replace("↑", "^").replace(" ", "")
        if not t:
            raise ValueError("Knuth: empty input")
        self._root = KNode.parse(t)

    def to_latex(self) -> str:
        return self._root.tex() if self._root else ""

    # --- operations ---
    def expand(self, n: int = 1) -> None:
        for _ in range(n):
            self._root = self._root.step()

    def expand_to(self, length: int) -> None:
        prev = self.to_latex()
        while True:
            self.expand(1)
            cur = self.to_latex()
            if len(cur) >= length or cur == prev:
                return
            prev = cur