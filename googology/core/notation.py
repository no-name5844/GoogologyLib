from enum import IntEnum
from .capability import Op, Capabilities


class Family(IntEnum):
    """Top-level taxonomy of notations."""
    Ordinal = 0
    Number = 1
    Hierarchy = 2
    RealSequence = 3


class UnsupportedOperation(Exception):
    def __init__(self, name: str, op: Op):
        super().__init__(f"Notation '{name}' does not implement the requested operation")


class NotComparable(Exception):
    def __init__(self, name: str):
        super().__init__(f"Notation '{name}': comparison is undefined for large-number notations")


class Notation:
    """Generic notation base. Every notation derives from this class."""

    # --- taxonomy ---
    @property
    def name(self) -> str:
        raise NotImplementedError

    @property
    def family(self) -> Family:
        raise NotImplementedError

    @property
    def subfamily(self) -> str:
        return ""

    @property
    def style(self) -> str:
        return ""

    # --- attribution ---
    @property
    def creator(self) -> str:
        return ""

    @property
    def version(self) -> str:
        return ""

    # --- capability ---
    @property
    def capabilities(self) -> Capabilities:
        raise NotImplementedError

    def can(self, op: Op) -> bool:
        return self.capabilities.has(op)

    # --- string conversion ---
    def string_to_it(self, s: str) -> None:
        raise UnsupportedOperation(self.name, Op.FromString)

    def to_latex(self) -> str:
        raise NotImplementedError

    # --- operations ---
    def compare(self, other: 'Notation') -> int:
        raise NotComparable(self.name)

    def expand(self, n: int = 1) -> None:
        raise UnsupportedOperation(self.name, Op.Expand)

    def expand_to(self, length: int) -> None:
        raise UnsupportedOperation(self.name, Op.ExpandTo)

    @property
    def comparable(self) -> bool:
        return self.can(Op.Compare)

    def reduce(self) -> None:
        prev = self.to_latex()
        for _ in range(1000):
            self.expand(1)
            cur = self.to_latex()
            if cur == prev:
                return
            prev = cur

    def __str__(self) -> str:
        return self.to_latex()