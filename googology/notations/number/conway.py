"""
Conway chained-arrow notation (康威链式箭头).

Rules:
  1. [a -> b]                = a ^ b
  2. X -> 1 -> Y             = X
  3. X -> a -> b             = X -> (X -> a-1 -> b) -> b-1
"""

from googology.core import Op, Capabilities, Family, Notation


class CNode:
    """A chain element: either an integer leaf, or a nested sub-chain."""

    def __init__(self, is_int: bool, val: int = 0, sub: list = None):
        self.is_int = is_int
        self.val = val
        self.sub = sub if sub else []

    @staticmethod
    def value(v: int) -> 'CNode':
        return CNode(True, val=v)

    @staticmethod
    def subchain(s: list) -> 'CNode':
        return CNode(False, sub=list(s))


class Conway(Notation):
    """Conway chained-arrow notation."""

    def __init__(self, chain: list = None):
        self._chain = chain if chain else []

    @property
    def name(self) -> str:
        return "conway"

    @property
    def family(self) -> Family:
        return Family.Number

    @property
    def creator(self) -> str:
        return "John Conway"

    @property
    def version(self) -> str:
        return "1"

    @property
    def capabilities(self) -> Capabilities:
        c = Capabilities()
        c.set(Op.FromString)
        c.set(Op.ToString)
        c.set(Op.Expand)
        c.set(Op.ExpandTo)
        return c

    @staticmethod
    def _ser(ch: list) -> str:
        if len(ch) == 2 and ch[0].is_int and ch[1].is_int:
            return f"{ch[0].val}^{{{ch[1].val}}}"
        parts = []
        for node in ch:
            if node.is_int:
                parts.append(str(node.val))
            else:
                parts.append(f"({Conway._ser(node.sub)})")
        return " \\rightarrow ".join(parts)

    @staticmethod
    def _step_once(ch: list) -> list:
        n = len(ch)
        if n <= 2:
            if n <= 1:
                return list(ch)
            if ch[0].is_int and ch[1].is_int:
                return list(ch)  # terminal: a -> b
            return Conway._recurse_sub(ch)

        # rule 2a: trailing 1  (X -> a -> 1  =  X -> a)
        if ch[-1].is_int and ch[-1].val == 1:
            return list(ch[:-1])

        # rule 2b: middle 1  (X -> 1 -> Y  =  X)
        if ch[-2].is_int and ch[-2].val == 1:
            return list(ch[:-2])

        # rule 3: X -> a -> b  =  X -> (X -> a-1 -> b) -> b-1
        if ch[-1].is_int and ch[-2].is_int:
            a = ch[-2].val
            b = ch[-1].val
            x = list(ch[:-2])
            inner = x + [CNode.value(a - 1), CNode.value(b)]
            return x + [CNode.subchain(inner), CNode.value(b - 1)]

        # no top-level rule: step any nested sub-chain
        return Conway._recurse_sub(ch)

    @staticmethod
    def _recurse_sub(ch: list) -> list:
        r = list(ch)
        changed = False
        for i, node in enumerate(r):
            if not node.is_int:
                before = Conway._ser(node.sub)
                s = Conway._step_once(node.sub)
                after = Conway._ser(s)
                if after != before:
                    if len(s) == 1 and s[0].is_int:
                        r[i] = CNode.value(s[0].val)
                    else:
                        r[i] = CNode.subchain(s)
                    changed = True
        if changed:
            return r
        return list(ch)

    @classmethod
    def from_string(cls, s: str) -> 'Conway':
        c = cls()
        c.string_to_it(s)
        return c

    def string_to_it(self, s: str) -> None:
        t = s.replace("→", "->").replace(" ", "")
        parts = t.split("->")
        self._chain = [CNode.value(int(p)) for p in parts]

    def to_latex(self) -> str:
        if not self._chain:
            return ""
        return self._ser(self._chain)

    def expand(self, n: int = 1) -> None:
        for _ in range(n):
            self._chain = self._step_once(self._chain)

    def expand_to(self, length: int) -> None:
        prev = self.to_latex()
        while True:
            self.expand(1)
            cur = self.to_latex()
            if len(cur) >= length or cur == prev:
                return
            prev = cur