/-
Symbolic ordinal expression tree (used for EXPAND pattern-matching on article
cases) + Mathlib.Ordinal-backed semantic ordinal value / compare / CNF.

Rationale (architecture decision, matches this library's design §5 / C2/C3):
  * "Never evaluate to a NUMBER" applies only to LARGE-NUMBER notations
    (Knuth, Conway). ORDINAL notations must be grounded in the TRUE set-
    theoretic ordinals — the exact supremum (upper bound) of the FS chain
    expand(A,0), expand(A,1), … is exactly the ordinal that A denotes, so
    for comparison / isZero / isSuccessor / predecessor / subtraction we
    use Mathlib's `Ordinal` (Lean 4's bona-fide ordinals, proved well-founded
    & class of all well-orders).
  * HOWEVER `expand()` dispatches on the article's ORIGINAL CASE SPLITS
    (§11 pluggable FS source). Those cases are defined over the SYNTAX of the
    original expression — `expand(α + β, n) = α + expand(β, n)` is a SYNTACTIC
    rewrite, NOT an ordinal-arithmetic simplification. So we keep a
    SYMBOLIC AST: `OrdinalExpr`. Only when we need the SEMANTIC value (for
    compare / predecessor / isSuccessor) do we "collapse" a CLOSED (WV-free)
    expression to a Mathlib.Ordinal.
  * The predicates used by `expand` itself (`isZero`, `isSuccessor`,
    `predecessor`) must stay COMPUTABLE: `expand` is a `partial` function,
    and Lean forbids partial definitions from depending on `noncomputable`
    ones. So they are implemented via ordinal-arithmetic identities over the
    syntax tree instead of via `toMOrdinal` evaluation (which is noncomputable
    through `Ordinal.instPow`). The identities hold for closed expressions;
    WV nodes are treated conservatively as nonzero / not-successor.

Types
-----
  * `OrdinalExpr` — symbolic expression tree (Zero/Omega/Succ/Add/Mul/Pow/WV).
    Pattern matching targets for expand(). WV stands for WeakVeblen (a list
    of (a_i @ b_i) coordinates; its ordinal value is undefined by the
    article, so `toMOrdinal` returns `none` for expressions containing
    a WV node).
  * `_root_.Ordinal` (from `Mathlib.SetTheory.Ordinal.Basic`) — true ordinals.

Conventions (faithful to C++):
  C2 : expand(ω, n) = n                   (standard FS for ω)
  C3 : component arithmetic (a = c+1, b_i > b_{i+1}, etc.) is evaluated on
       the Mathlib.Ordinal value (components must be CLOSED — otherwise a
       defensive `none`/fallback is returned).
-/

import Googology.Basic
import Mathlib.SetTheory.Ordinal.Basic
import Mathlib.SetTheory.Ordinal.Arithmetic
import Mathlib.SetTheory.Ordinal.CantorNormalForm

namespace Googology

abbrev MOrdinal := Ordinal.{0}   -- the real, Mathlib ordinals (fixed universe)

/-! Symbolic expression tree — SORTED BY case split priority for expand. -/
inductive OrdinalExpr where
  | zero
  | omega
  | succ : OrdinalExpr → OrdinalExpr
  | add  : OrdinalExpr → OrdinalExpr → OrdinalExpr
  | mul  : OrdinalExpr → OrdinalExpr → OrdinalExpr
  | pow  : OrdinalExpr → OrdinalExpr → OrdinalExpr
  | wv   : List (OrdinalExpr × OrdinalExpr) → OrdinalExpr
  deriving Repr

namespace OrdinalExpr

instance : Inhabited OrdinalExpr := ⟨.zero⟩

/-! Factories ---------------------------------------------------------- -/
def one   : OrdinalExpr := .succ .zero
def fromInt (n : Int) : OrdinalExpr :=
  match n with
  | Int.ofNat m => fromNat m
  | Int.negSucc _ => .zero   -- n ≤ 0 → 0
where
  fromNat : Nat → OrdinalExpr
    | 0 => .zero
    | m + 1 => .succ (fromNat m)

/-! Collapse a CLOSED (WV-free) expression to a true Mathlib.Ordinal.
    Returns `none` if the tree contains a `WV` node or any malformed part.
    This is the semantic floor: the denoted ordinal exists and equals this
    Mathlib.Ordinal whenever the function succeeds; WV nodes are (by the
    article) explicitly NOT endowed with an ordinal value. -/
noncomputable def toMOrdinal : OrdinalExpr → Option MOrdinal
  | .zero => some 0
  | .omega => some Ordinal.omega0
  | .succ a => toMOrdinal a |>.map fun o => o + 1
  | .add a b => do let oa ← toMOrdinal a; let ob ← toMOrdinal b; some (oa + ob)
  | .mul a b => do let oa ← toMOrdinal a; let ob ← toMOrdinal b; some (oa * ob)
  | .pow a b => do let oa ← toMOrdinal a; let ob ← toMOrdinal b; some (oa ^ ob)
  | .wv _ => none   -- article omits ordinal value of a WV expression

/-! Derived predicates & ops. ------------------------------------------
    `isZero` / `isSuccessor` / `predecessor` are COMPUTABLE ordinal-identity
    versions (see header): a+b=0 ⇔ a=0∧b=0; a·b=0 ⇔ a=0∨b=0; a^b=0 ⇔ a=0∧b≠0;
    a+b is a successor iff b is, or b=0 and a is; a·b iff both are; a^b iff
    a=1, or b=0, or both are. `predecessor` unwraps a syntactic `succ` (all
    closed finite ordinals from `fromInt`/`parse` are succ-chains). -/

/-- Zero test via ordinal identities (no evaluation, computable). WV nodes
    are never zero (conservative; the article gives them no value). -/
def isZero : OrdinalExpr → Bool
  | .zero => true
  | .succ _ => false
  | .omega => false
  | .add a b => isZero a && isZero b
  | .mul a b => isZero a || isZero b
  | .pow a b => isZero a && !isZero b
  | .wv _ => false

/-- Syntactic test for the expression 1 = succ 0. -/
def isOne : OrdinalExpr → Bool
  | .succ .zero => true
  | _ => false

/-- Successor test via ordinal identities (computable). -/
def isSuccessor : OrdinalExpr → Bool
  | .zero => false
  | .omega => false
  | .succ _ => true
  | .add a b => if isZero b then isSuccessor a else isSuccessor b
  | .mul a b => isSuccessor a && isSuccessor b
  | .pow a b => isOne a || isZero b || (isSuccessor a && isSuccessor b)
  | .wv _ => false

/-- Predecessor of a syntactic successor; non-succ expressions are left
    untouched (callers fall back to a conservative `.wv comps`). -/
def predecessor (e : OrdinalExpr) : Option OrdinalExpr :=
  match e with
  | .succ a => some a
  | _ => none

/-- Semantic 3-way compare via Mathlib.Ordinal values. Falls back to 0 when
    either side is open (WV); the caller detects mixed/open via type tags. -/
noncomputable def compare (A B : OrdinalExpr) : Int :=
  match toMOrdinal A, toMOrdinal B with
  | some oa, some ob =>
    if oa < ob then -1 else if oa = ob then 0 else 1
  | _, _ => 0   -- undefined case: treat as equal (caller handles type tags)

noncomputable def equals (A B : OrdinalExpr) : Bool := compare A B = 0
noncomputable instance : BEq OrdinalExpr where beq := equals

/-- The unique natural `n` with `↑n = o`, for a finite ordinal `o < ω`. -/
noncomputable def omegaNat (o : MOrdinal) : Nat :=
  if h : o < Ordinal.omega0 then
    Classical.choose (Ordinal.lt_omega0.mp h)
  else 0

/-- Left subtraction A - B (requires A >= B, closed). -/
noncomputable def subtract (A B : OrdinalExpr) : Option OrdinalExpr :=
  match toMOrdinal A, toMOrdinal B with
  | some oa, some ob =>
    if ob ≤ oa then
      let d : MOrdinal := oa - ob
      if d < Ordinal.omega0 then some (fromInt (omegaNat d))
      else none  -- infinite diff not expressible as fromInt (we don't need it)
    else none
  | _, _ => none

/-! expand — fundamental sequence. Returns an error when undefined (matches
    C++ policy: 0 / closed / successor CNF / successor have no FS). The WV
    case splits (C++ Ordinal::expandWV) live in the `where` clause so that
    the mutual recursion `expand ↔ expandWV` forms a single partial group. -/
partial def expand : OrdinalExpr → Int → Except String OrdinalExpr
  | .zero, _ => .error "expand: 0 has no FS"
  | .omega, n => .ok (fromInt n)   -- C2
  | .succ _, _ => .error "expand: successor has no FS"
  | .add α β, n =>
    if isZero β then .ok α
    else
      match expand β n with
      | .ok β' => .ok (.add α β')
      | .error e => .error e
  | .mul α β, n =>
    if isZero β then .ok .zero
    else
      match expand β n with
      | .ok β' => .ok (.mul α β')
      | .error e => .error e
  | .pow α β, n =>
    if isZero β then .ok one
    else
      match expand β n with
      | .ok e =>
        -- α^1 → α collapse
        if isOne e then .ok α
        else .ok (.pow α e)
      | .error e => .error e
  | .wv comps, n => .ok (expandWV comps n)
where
  /-- The 6 weak-Veblen cases (C++ Ordinal::expandWV). -/
  expandWV (comps : List (OrdinalExpr × OrdinalExpr)) (n : Int) : OrdinalExpr :=
    match comps with
    | (a, _) :: [] => if isZero a then fromInt n else expandWVRest comps n   -- case 1: A=(0) -> n
    | (a0, _) :: rest => if isZero a0 then .wv rest else expandWVRest comps n  -- case 2: drop (0@b1)
    | _ => expandWVRest comps n
  expandWVRest (comps : List (OrdinalExpr × OrdinalExpr)) (n : Int) : OrdinalExpr :=
    let last := comps.getLast!
    let aLast := last.1
    let bLast := last.2
    let withoutLast := comps.dropLast
    match aLast with
    | .succ a =>
      if isZero bLast then
        -- Case 3: (#, (a+1)@0)
        let base : List (OrdinalExpr × OrdinalExpr) := withoutLast ++ [(a, .zero)]
        let baseWV := .wv base
        if n = 0 then .add baseWV one
        else
          let recCase := expandWV comps (n - 1)
          .pow baseWV recCase
      else
        match bLast with
        | .succ b =>
          -- Case 4: (#, (a+1)@(b+1))
          let base : List _ := withoutLast ++ [(a, .succ b)]
          let baseWV := .wv base
          if n = 0 then .add baseWV one
          else
            let recCase := expandWV comps (n - 1)
            let r : List _ := base ++ [(recCase, b)]
            .wv r
        | _ =>
          -- Case 5: b is a limit; apply the standard §11 FS on b
          match expand bLast n with
          | .ok eb =>
            let r : List _ := withoutLast ++ [(a, bLast), (one, eb)]
            .wv r
          | .error _ => .wv comps
    | _ =>
      -- Case 6: a is a limit (not a syntactic successor) — expand a[n]@b
      match expand aLast n with
      | .ok ea =>
        let r : List _ := withoutLast ++ [(ea, bLast)]
        .wv r
      | .error _ => .wv comps

/-! Parser (ordinal sub-expressions used by WeakVeblen coordinates & Ns).
    Grammar: right-assoc ^, then *, then +; ω/w/omega/UTF-8 ω; parentheses;
    integers. Empty → 0.
    Positions are `s.Pos` (Lean 4.30 byte offsets with validity proof);
    character access is via `Pos.get?`, stepping via `Pos.next!`. --------- -/
private structure OpState where
  s : String
  i : s.Pos := s.startPos

private partial def skipSpacesLoop {s : String} (p : s.Pos) : s.Pos :=
  match p.get? with
  | some c => if c = ' ' ∨ c = '\t' then skipSpacesLoop p.next! else p
  | none => p

private def OpState.skipSpaces (st : OpState) : OpState :=
  { st with i := skipSpacesLoop st.i }

private def OpState.peek (st : OpState) : Option Char :=
  (st.skipSpaces).i.get?

private def OpState.get (st : OpState) : Option Char × OpState :=
  let st' := st.skipSpaces
  match st'.i.get? with
  | some c => (some c, { st' with i := st'.i.next! })
  | none => (none, st')

private def tryOmega (st : OpState) : Bool × OpState :=
  let st' := st.skipSpaces
  match st'.i.get? with
  | some c =>
    if c = 'w' ∨ c = 'W' then (true, { st' with i := st'.i.next! })
    else if c = 'o' then
      let p1 := st'.i.next!
      let p2 := p1.next!
      let p3 := p2.next!
      let p4 := p3.next!
      match p1.get?, p2.get?, p3.get?, p4.get? with
      | some 'm', some 'e', some 'g', some 'a' => (true, { st' with i := p4.next! })
      | _, _, _, _ => (false, st)
    else if c = 'ω' then (true, { st' with i := st'.i.next! })
    else (false, st)
  | none => (false, st)

private partial def digitsLoop (cur : Int) (s : OpState) : Int × OpState :=
  match s.get with
  | (some c, s') => if c.isDigit then digitsLoop (cur * 10 + (c.toNat - 48 : Int)) s' else (cur, s)
  | (none, s') => (cur, s')

private partial def parseExpr (st : OpState) : Except String (OrdinalExpr × OpState) :=
  parseAdd st
where
  parseAdd st := do
    let (l, st2) ← parseMul st
    addCont l st2
  addCont l st :=
    let st' := st.skipSpaces
    match st'.peek with
    | some '+' =>
      let (_, st2) := st'.get
      do let (r, st3) ← parseMul st2
         addCont (.add l r) st3
    | _ => .ok (l, st')
  parseMul st := do
    let (l, st2) ← parsePow st
    mulCont l st2
  mulCont l st :=
    let st' := st.skipSpaces
    match st'.peek with
    | some '*' =>
      let (_, st2) := st'.get
      do let (r, st3) ← parsePow st2
         mulCont (.mul l r) st3
    | _ => .ok (l, st')
  parsePow st := do
    let (l, st2) ← parseAtom st
    let st' := st2.skipSpaces
    match st'.peek with
    | some '^' =>
      let (_, st3) := st'.get
      do let (r, st4) ← parsePow st3
         .ok (.pow l r, st4)
    | _ => .ok (l, st')
  parseAtom st :=
    match tryOmega st with
    | (true, st2) => .ok (.omega, st2)
    | (false, _) =>
      match st.skipSpaces.get with
      | (none, _) => .error "parseAtom: empty"
      | (some '(', st2) => do
          let (v, st3) ← parseExpr st2
          let (c, st4) := st3.skipSpaces.get
          if c = some ')' then .ok (v, st4)
          else .error "parseAtom: expected ')'"
      | (some d, st2) =>
        if d.isDigit then
          let (val, st3) := digitsLoop (d.toNat - 48 : Int) st2
          .ok (fromInt val, st3)
        else if d = '-' then .ok (.zero, st2)
        else .error s!"parseAtom: unexpected '{d}'"

/-! Public top-level parse. -/
def parse (s : String) : Except String OrdinalExpr :=
  match parseExpr ⟨s, s.startPos⟩ with
  | .ok (v, st') =>
    if st'.skipSpaces.i ≠ st'.s.endPos then
      .error s!"parse: trailing at pos {st'.i.offset}"
    else .ok v
  | .error e => .error e

end OrdinalExpr
end Googology
