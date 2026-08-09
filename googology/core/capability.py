from enum import IntEnum


class Op(IntEnum):
    """Operations a notation may support (capability model)."""
    FromString = 0
    ToString = 1
    Normalize = 2
    Compare = 3
    Expand = 4
    ExpandTo = 5
    Successor = 6


class Capabilities:
    """Bitset of supported operations."""

    def __init__(self):
        self._bits = 0

    def set(self, op: Op) -> None:
        self._bits |= 1 << int(op)

    def has(self, op: Op) -> bool:
        return (self._bits & (1 << int(op))) != 0

    def __repr__(self) -> str:
        return f"Capabilities({self._bits:07b})"