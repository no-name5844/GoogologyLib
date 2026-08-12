"""PPS 系列单测（与 C++ tests/unit/test_pps.cpp 断言一致）。"""

import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from googology.notations.ordinal.sequence.marked_parent.pps import (
    Pps1, Pps2, Pps4, WPps4, TPps4, FPps4,
)
from googology.core import NotComparable


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def expect_throw(fn, exc, msg):
    try:
        fn()
    except exc:
        return
    raise AssertionError(msg)


# ---- PPS1 展开：坏根 = 第 x 项（x = 末项值）----
check(Pps1("(0, 1, 0, 3)").to_latex() == "(0, 1, 0, 3)", "parse (0,1,0,3)")
check(Pps1("0,1,0,3").to_latex() == "(0, 1, 0, 3)", "parse bare")
check(Pps1("[0,1,0,3]").to_latex() == "(0, 1, 0, 3)", "parse []")

p = Pps1("(0,1,2)"); p.expand(0)
check(p.to_latex() == "(0, 1, 1)", "pps1 (0,1,2).expand(0)")
p = Pps1("(0,1,2)"); p.expand(1)
check(p.to_latex() == "(0, 1, 1, 1)", "pps1 (0,1,2).expand(1)")
p = Pps1("(0,1,2)"); p.expand(2)
check(p.to_latex() == "(0, 1, 1, 1, 1)", "pps1 (0,1,2).expand(2)")

p = Pps1("(0,1,0,3)"); p.expand(0)
check(p.to_latex() == "(0, 1, 0, 2)", "pps1 (0,1,0,3).expand(0)")
p = Pps1("(0,1,0,3)"); p.expand(1)
check(p.to_latex() == "(0, 1, 0, 2, 2)", "pps1 (0,1,0,3).expand(1)")

p = Pps1("(0,1,0,1,2)"); p.expand(0)
check(p.to_latex() == "(0, 1, 0, 1, 1, 0, 1)", "pps1 (0,1,0,1,2).expand(0)")

# 后继：末项为 0 → 去掉末项
p = Pps1("(0,1,0)"); p.expand(3)
check(p.to_latex() == "(0, 1)", "pps1 (0,1,0).expand(3)")
check(Pps1("(0,1,0)").is_successor, "pps1 (0,1,0).is_successor")
check(not Pps1("(0,1,2)").is_successor, "pps1 (0,1,2) not successor")

# expand_to = expandLen
p = Pps1("(0,1,2)"); p.expand_to(2)
check(p.to_latex() == "(0, 1, 1, 1, 1)", "pps1 (0,1,2).expand_to(2)")

# 坏根越界 → ValueError
expect_throw(lambda: Pps1("(0,5)").expand(0), ValueError, "pps1 (0,5) bad-root OOR")

# ---- PPS1 vs PPS2 ----
p = Pps1("(0,2,4,4,2,3)"); p.expand(0)
check(p.to_latex() == "(0, 2, 4, 4, 2, 4, 7, 2)", "pps1 (0,2,4,4,2,3).expand(0)")
p = Pps2("(0,2,4,4,2,3)"); p.expand(0)
check(p.to_latex() == "(0, 2, 4, 4, 2, 2, 7, 2)", "pps2 (0,2,4,4,2,3).expand(0)")
p = Pps2("(0,1,2)"); p.expand(0)
check(p.to_latex() == "(0, 1, 1)", "pps2 (0,1,2).expand(0)")

# ---- PPS4 弱/强展开 ----
p = Pps4("(0,1,0,1,2)"); p.expand(0)
check(p.to_latex() == "(0, 1, 0, 1, 1, 0, 1)", "pps4 weak expand(0)")
p = Pps4("(0,1,0,2,2,3)"); p.expand(0)
check(p.to_latex() == "(0, 1, 0, 2, 2, 1, 2, 2)", "pps4 strong col expand(0)")
p = Pps1("(0,1,0,2,2,3)"); p.expand(0)
check(p.to_latex() == "(0, 1, 0, 2, 2, 2, 2, 2)", "pps1 same input")
p = Pps4("(0,1,2,1,3)"); p.expand(0)
check(p.to_latex() == "(0, 1, 2, 1, 2, 1)", "pps4 strong not-found")

# ---- wPPS4（与 PPS4 一致）----
p = WPps4("(0,1,0,2,2,3)"); p.expand(0)
check(p.to_latex() == "(0, 1, 0, 2, 2, 1, 2, 2)", "wpps4 expand(0)")

# ---- tPPS4 / fPPS4 强展开复制修饰 ----
p = TPps4("(0,1,0,2,2,3)"); p.expand(1)
check(p.to_latex() == "(0, 1, 0, 2, 2, 1, 2, 2, 4, 2, 2)", "tpps4 expand(1)")
p = Pps4("(0,1,0,2,2,3)"); p.expand(1)
check(p.to_latex() == "(0, 1, 0, 2, 2, 1, 2, 2, 1, 2, 2)", "pps4 expand(1)")
p = FPps4("(0,1,0,2,2,3)"); p.expand(1)
check(p.to_latex() == "(0, 1, 0, 2, 2, 1, 2, 2, 3, 2, 2)", "fpps4 expand(1)")

# ---- §12 limit API ----
check(Pps1.limit(0).to_latex() == "()", "limit(0)")
check(Pps1.limit(3).to_latex() == "(0, 1, 2)", "limit(3)")
ml = Pps1.master_limit(); ml.expand(3)
check(ml.to_latex() == "(0, 1, 2)", "master_limit.expand(3)")

# ---- compare ----
check(Pps1("(0,1,2)").compare(Pps1("(0,1,3)")) == -1, "compare <")
check(Pps1("(0,1,3)").compare(Pps1("(0,1,2)")) == 1, "compare >")
check(Pps1("(0,1,2)").compare(Pps1("(0,1,2)")) == 0, "compare ==")
check(Pps1("(0,1,2)").compare(Pps1("(0,1,2,0)")) == -1, "compare prefix")
expect_throw(lambda: Pps1("(0,1,2)").compare(Pps2("(0,1,2)")), NotComparable,
             "cross-variant compare")
from googology.notations.number.knuth import Knuth
expect_throw(lambda: Pps1("(0,1,2)").compare(Knuth("2 ^ 3")), NotComparable,
             "cross-family compare")

# ---- __getitem__（非修改）----
check(Pps1("(0,1,2)")[1].to_latex() == "(0, 1, 1, 1)", "A[1]")

# ---- taxonomy / capability ----
p = Pps1()
check(p.name == "pps1" and Pps2().name == "pps2" and Pps4().name == "pps4"
      and WPps4().name == "wpps4" and TPps4().name == "tpps4"
      and FPps4().name == "fpps4", "variant names")
check(p.style == "marked_parent", "style")
check(p.creator == "318`4", "creator")
check(p.version == "1" and TPps4().version == "4t", "versions")
check(p.can(__import__("googology.core", fromlist=["Op"]).Op.Compare), "can Compare")

print("test_pps: PASS")
