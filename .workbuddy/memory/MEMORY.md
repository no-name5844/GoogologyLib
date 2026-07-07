# GoogologyLib — project notes

Multi-language notation library. Each language on its own git branch
(C, C++, java, python, Lean4); `master` holds language-neutral `spec/` +
`tests/vectors/` (golden vectors). User audits the **C++** branch.

## Core architectural rules (decided with user)
- **序数记号 ≠ 大数记号**: ordinal notations have well-defined `compare`
  (ordinal order); large-number notations do NOT — `compare()` throws
  `NotComparable`, `comparable()` false. Never compare across families.
- Generic `Notation` base with hierarchical taxonomy accessors
  `family()/subfamily()/style()` and a `Capabilities` bitset (Op enum).
  Unsupported ops throw `UnsupportedOperation` (so interface is always
  complete even when only some ops implemented — user's "其他运算也要包含进去").
- `expand(n)` = fundamental sequence nth term (ordinal) / one rewrite step
  (number). `expand_to(len)` also provided ("两者都要").
- **No numeric evaluation (一律不求值):** library NEVER collapses a notation to a
  value. No `evaluate()` returning a number; `expand`/`reduce` return the notation's
  own symbolic form (even ordinary arithmetic like `3^3` stays symbolic). `Evaluate`
  and `ToOrdinal` are REMOVED from the Op enum.
- `BigInt` is just an int64_t alias for parse params / expansion indices; no boost
  dependency (numeric evaluation removed).
- Docs are bilingual (Chinese + English): README, design.md, spec/notations/*.md.

## Taxonomy
- ordinal/sequence/{difference:pps, marked_parent:y_sequence}
  (Y=标记父向型, PPS=阶差型; both 自然数序列记号)
- ordinal/{cnf, veblen, buchholz, taranovsky, bocf, mountain}
- number/{knuth, conway, beaf, hyper_e, extensible_e, bird}
- hierarchy/{fgh, hardy, shg}

## C++ branch status (2026-07-07)
- Core scaffolding + number/knuth + number/conway committed (a224020); builds & tests PASS.
- Docs committed bilingual + no-evaluation principle (b82f71e).
- CURRENT MISMATCH: code still has `evaluate()`; next step removes it and switches
  knuth/conway to symbolic expand/reduce (Parse/Serialize/Expand/ExpandTo, NO Evaluate).
- Other notations are future slots.
