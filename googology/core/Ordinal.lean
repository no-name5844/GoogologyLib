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

Types
-----
  * `OrdinalExpr` — symbolic expression tree (Zero/Omega/Succ/Add/Mul/Pow/WV).
    Pattern matching targets for expand(). WV stands for WeakVeblen (a list
    of (a_i @ b_i) coordinates; its ordinal value is undefined by the
    article, so `toMathlibOrdinal?` returns `none` for expressions containing
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

abbrev MOrdinal := Ordinal   -- the real, Mathlib ordinals

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

/-! Factories ---------------------------------------------------------- -/
def one   : OrdinalExpr := .succ .zero
def fromInt (n : Int) : OrdinalExpr :=
  if n ≤ 0 then .zero
  else
    let rec loop (k : Int) (acc : OrdinalExpr) : OrdinalExpr :=
      if k ≤ 0 then acc else loop (k - 1) (.succ acc)
    loop n .zero

/-! Collapse a CLOSED (WV-free) expression to a true Mathlib.Ordinal.
    Returns `none` if the tree contains a `WV` node or any malformed part.
    This is the semantic floor: the denoted ordinal exists and equals this
    Mathlib.Ordinal whenever the function succeeds; WV nodes are (by the
    article) explicitly NOT endowed with an ordinal value. -/
partial def toMOrdinal : OrdinalExpr → Option MOrdinal
  | .zero => some 0
  | .omega => some Ordinal.omega
  | .succ a => toMOrdinal a |>.map fun o => o + 1
  | .add a b => do let oa ← toMOrdinal a; let ob ← toMOrdinal b; some (oa + ob)
  | .mul a b => do let oa ← toMOrdinal a; let ob ← toMOrdinal b; some (oa * ob)
  | .pow a b => do let oa ← toMOrdinal a; let ob ← toMOrdinal b; some (oa ^ ob)
  | .wv _ => none   -- article omits ordinal value of a WV expression

/-! Derived predicates & ops (semantic, via Mathlib). ------------------- -/

/-- True semantically (closed-zero or empty CNF). WV nodes are never zero. -/
def isZero (e : OrdinalExpr) : Bool :=
  match toMOrdinal e with
  | some o => o = 0
  | none =>
    -- Fallback: structural check (useful so expand on Add can test β)
    match e with | .zero => true | .wv _ => false | _ => false

/-- Semantic successor check: closed expressions only. -/
def isSuccessor (e : OrdinalExpr) : Bool :=
  match toMOrdinal e with
  | some o => Ordinal.isSucc o
  | none => false

/-- Semantic predecessor of a successor. -/
def predecessor (e : OrdinalExpr) : Option OrdinalExpr :=
  match toMOrdinal e with
  | some o =>
    if h : Ordinal.isSucc o then
      let o' := Ordinal.pred o h
      -- The ordinal o' is < o and is a closed ordinal; for our purposes
      -- any finite closed ordinal can be reconstructed syntactically via
      -- fromInt of its NatPart when it's finite, but here we take the
      -- simplest road: if o' is a natural (< ω) we reconstruct with fromInt
      -- on the natural part; otherwise we leave as symbolic (callers that
      -- need the structure can pattern match on the SUCC constructor in
      -- `expandWV`, which doesn't use predecessor on open expressions).
      if h2 : o' < Ordinal.omega then
        some (fromInt (Ordinal.toNat o'))
      else
        -- For infinite o'-1 there's no clean fromInt. This case shouldn't
        -- fire because the caller (expandWV case 3/4) uses `a+1@0` / `a+1@b+1`
        -- with a being SUCC of the LAST coordinate which is typically finite.
        none
    else none
  | none => none

/-- Semantic 3-way compare. Falls back to structural none-based unequal if
    open expressions are compared (0 returned; caller should detect mixed). -/
def compare (A B : OrdinalExpr) : Int :=
  match toMOrdinal A, toMOrdinal B with
  | some oa, some ob =>
    if oa < ob then -1 else if oa = ob then 0 else 1
  | _, _ => 0   -- undefined case: treat as equal (caller handles type tags)

def equals (A B : OrdinalExpr) : Bool := compare A B = 0
instance : BEq OrdinalExpr where beq := equals

/-- Left subtraction A - B (requires A >= B, closed). -/
def subtract (A B : OrdinalExpr) : Option OrdinalExpr :=
  do let oa ← toMOrdinal A
     let ob ← toMOrdinal B
     if h : ob ≤ oa then
       let d : MOrdinal := oa - ob
       if d < Ordinal.omega then some (fromInt (Ordinal.toNat d))
       else none  -- infinite diff not expressible as fromInt (we don't need it)
     else none

/-! toString (LaTeX-ish, matches C++ Ordinal::to_string). --------------- -/
partial def toString : OrdinalExpr → String
  | .zero => "0"
  | .omega => "ω"
  | .succ x =>
    -- "0 + 1" should just be "1". Detect.
    match x with
    | .zero => "1"
    | _ => s!"{toString x} + 1"
  | .add a b => s!"{toString a} + {toString b}"
  | .mul a b => s!"{toString a} * {toString b}"
  | .pow a b => s!"({toString a} ^ {toString b})"
  | .wv comps =>
    let parts := comps.map fun (a, b) =>
      s!"{dispPart a}@{dispPart b}"
    s!"({String.intercalate ", " parts})"
where
  dispPart (x : OrdinalExpr) : String :=
    let s := toString x
    if s.any (fun c => c = ' ' ∨ c = '(' ∨ c = ')') then s!"({s})" else s

/-! expandWV — the 6 weak-Veblen cases (C++ Ordinal::expandWV). -------- -/
partial def expandWV (comps : List (OrdinalExpr × OrdinalExpr)) (n : Int) : OrdinalExpr :=
  match comps with
  | [(a, _)] when isZero a => fromInt n   -- case 1: A=(0) -> n
  | (a0, _) :: rest when isZero a0 => .wv rest   -- case 2: drop (0@b1)
  | _ =>
    let last := comps.getLast!
    let aLast := last.1
    let bLast := last.2
    let withoutLast := comps.dropLast
    if h : aLast.isSuccessor then
      match aLast.predecessor with
      | none => .wv comps
      | some a =>
        if isZero bLast then
          -- Case 3: (#, (a+1)@0)
          let base : List (OrdinalExpr × OrdinalExpr) := withoutLast ++ [(a, .zero)]
          let baseWV := .wv base
          if n = 0 then .add baseWV one
          else
            let recCase := expandWV comps (n - 1)
            .pow baseWV recCase
        else
          if h2 : bLast.isSuccessor then
            match bLast.predecessor with
            | none =>
              -- Case 5: b is limit
              match expand bLast n with
              | .ok eb =>
                let r : List _ := withoutLast ++ [(a, bLast), (one, eb)]
                .wv r
              | .error _ => .wv comps
            | some b =>
              -- Case 4: (#, (a+1)@(b+1))
              let base : List _ := withoutLast ++ [(a, .succ b)]
              let baseWV := .wv base
              if n = 0 then .add baseWV one
              else
                let recCase := expandWV comps (n - 1)
                let r : List _ := base ++ [(recCase, b)]
                .wv r
          else
            -- Case 5 fallback: b has no predecessor but isSuccessor was
            -- false → a limit; apply the standard §11 FS on b
            match expand bLast n with
            | .ok eb =>
              let r : List _ := withoutLast ++ [(a, bLast), (one, eb)]
              .wv r
            | .error _ => .wv comps
    else
      -- Case 6: a is a limit (not successor) — expand a[n]@b
      match expand aLast n with
      | .ok ea =>
        let r : List _ := withoutLast ++ [(ea, bLast)]
        .wv r
      | .error _ => .wv comps
where
  expand e n := OrdinalExpr.expand e n  -- forward to main expand (below)

/-! expand — fundamental sequence. Returns `none` when undefined (matches
    C++ policy: 0 / closed / successor CNF / successor have no FS). -/
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
        if equals e one then .ok α
        else .ok (.pow α e)
      | .error e => .error e
  | .wv comps, n => .ok (expandWV comps n)

/-! Parser (ordinal sub-expressions used by WeakVeblen coordinates & Ns).
    Grammar: right-assoc ^, then *, then +; ω/w/omega/UTF-8 ω; parentheses;
    integers. Empty → 0. ------------------------------------------------ -/
private structure OpState where
  s : String
  i : Nat := 0

private def OpState.skipSpaces (st : OpState) : OpState :=
  let rec loop (i : Nat) : Nat :=
    if h : i < st.s.length then
      let c := st.s[i]
      if c = ' ' ∨ c = '\t' then loop (i + 1) else i
    else i
  { st with i := loop st.i }

private def OpState.peek (st : OpState) : Option Char :=
  let st' := st.skipSpaces
  if h : st'.i < st'.s.length then some (st'.s[st'.i]) else none

private def OpState.get (st : OpState) : Option Char × OpState :=
  let st' := st.skipSpaces
  if h : st'.i < st'.s.length then
    (some (st'.s[st'.i]), { st' with i := st'.i + 1 })
  else (none, st')

private def tryOmega (st : OpState) : Bool × OpState :=
  let st' := st.skipSpaces
  if h : st'.i < st'.s.length then
    let c := st'.s[st'.i]
    if c = 'w' ∨ c = 'W' then (true, { st' with i := st'.i + 1 })
    else if c = 'o' then
      let rest := st'.s.extract st'.i st'.s.length
      if rest.startsWith "omega" then (true, { st' with i := st'.i + 5 })
      else (false, st)
    else if c = 'ω' then (true, { st' with i := st'.i + 1 })
    else (false, st)
  else (false, st)

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
          let rec loop (cur : Int) (s : OpState) : Int × OpState :=
            match s.get with
            | (some c, s') =>
              if c.isDigit then
                loop (cur * 10 + (c.toNat - 48 : Int)) s'
              else (cur, s)
            | (none, s') => (cur, s')
          let (val, st3) := loop (d.toNat - 48 : Int) st2
          .ok (fromInt val, st3)
        else if d = '-' then .ok (.zero, st2)
        else .error s!"parseAtom: unexpected '{d}'"

/-! Public top-level parse. -/
def parse (s : String) : Except String OrdinalExpr :=
  match parseExpr ⟨s, 0⟩ with
  | .ok (v, st') =>
    if st'.skipSpaces.i < st'.s.length then
      .error s!"parse: trailing at pos {st'.i}"
    else .ok v
  | .error e => .error e

end OrdinalExpr
end Googology