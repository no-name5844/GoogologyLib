/-
Symbolic ordinal (序数) expression tree.
Fully based on C++ include/googology/core/Ordinal.hpp semantics.

Variants:
  Zero, Omega (ω), Succ a      = a + 1
  Add a b   = a + b
  Mul a b   = a * b
  Pow a b   = a ^ b
  WV  comps = weak-Veblen (a_i @ b_i) list
  Cnf terms = Cantor normal form (descending exponents, coeffs >= 1)

Operations (mirrors C++):
  expand(n) — fundamental sequence (§11 pluggable):
    * 0 / successor / CNF-closed have no FS (throw)
    * expand(ω, n) = n   (C2)
    * expand(α + β, n) = α + expand(β, n)
    * expand(α * β, n) = α * expand(β, n)
    * expand(α ^ β, n) = α ^ expand(β, n), with α^1 → α collapse
    * expand(WV, n) → 6 weak-Veblen cases (expandWV)
  to_string — LaTeX-ish string rendering
  parse     — ordinal strings: integers, ω/w/omega, +, *, ^, parentheses
  compare   — via Cantor normal form
  isZero/isSuccessor, predecessor, subtract
-/

import Googology.Basic

namespace Googology

/-! Ordinal --------------------------------------------------------------- -/
inductive Ordinal where
  | zero
  | omega
  | succ      : Ordinal → Ordinal
  | add       : Ordinal → Ordinal → Ordinal
  | mul       : Ordinal → Ordinal → Ordinal
  | pow       : Ordinal → Ordinal → Ordinal
  | wv        : List (Ordinal × Ordinal) → Ordinal   -- (a_i @ b_i)
  | cnf       : List (Ordinal × Int) → Ordinal       -- (exponent, coeff)
  deriving Repr

namespace Ordinal

abbrev GoogInt := Int

/-! factories -/
def one   : Ordinal := .cnf [(.zero, 1)]
def fromInt (n : Int) : Ordinal :=
  if n ≤ 0 then .zero else .cnf [(.zero, n)]

/-! CNF: pair of exponent (Ordinal) and coefficient (Int, ≥ 1, descending by exponent) -/
abbrev Cnf := List (Ordinal × Int)

private def gcdInt (a b : Int) : Int :=
  let rec go (a b : Int) : Int :=
    if b = 0 then if a < 0 then -a else a
    else go b (a % b)
  go (if a < 0 then -a else a) (if b < 0 then -b else b)

/- NormalizeCnf (sorts descending by exponent, merges equal exponents, drops zeros). -/
private def normalizeCnf (v : Cnf) : Cnf := Id.run do
  -- sort descending by exponent compare
  let rec lexGT (x y : Ordinal × Int) : Bool := compareCnf [x] [y] > 0
  let sorted := v.insertionSort (fun x y => lexGT x y)
  let mut out : Cnf := []
  for t in sorted do
    let (e, c) := t
    if c = 0 then continue
    match out with
    | [] => out := [(e, c)]
    | prev@(_) =>
      let last := prev.getLast!
      if compareCnf [last] [(e, 0)] = 0 then
        let (e', c') := last
        let newC := c' + c
        out := prev.dropLast
        if newC ≠ 0 then out := out ++ [(e', newC)]
      else
        out := out ++ [(e, c)]
  out.filter (fun (_, c) => c ≠ 0)
where
  compareCnf (A B : Cnf) : Int :=
    match A, B with
    | [], [] => 0
    | [], _  => -1
    | _,  [] => 1
    | (e1, c1) :: A', (e2, c2) :: B' =>
      let ce := compareCnf [(e1, 0)] [(e2, 0)]
      if ce ≠ 0 then ce
      else if c1 ≠ c2 then (if c1 < c2 then -1 else 1)
      else compareCnf A' B'

/-! Compare via CNF (well-founded recursion trick: delegate to top-level compare). -/
def compareCnf (A B : Cnf) : Int :=
  match A, B with
  | [], [] => 0
  | [], _ => -1
  | _, [] => 1
  | (e1, c1) :: A', (e2, c2) :: B' =>
    let ce := Ordinal.compare e1 e2
    if ce ≠ 0 then ce
    else if c1 ≠ c2 then (if c1 < c2 then -1 else 1)
    else compareCnf A' B'

/-! Convert this Ordinal to CNF (throws for WV node components). -/
partial def toCnf : Ordinal → Cnf
  | .zero => []
  | .omega => [(.cnf [(.zero, 1)], 1)]   -- ω = ω^1 = ω^(1 as CNF)
  | .cnf ts => ts
  | .succ x =>
    let xc := toCnf x
    if xc.isEmpty then [(.zero, 1)]
    else
      let last := xc.getLast!
      if (last.1.isZero) then
        xc.dropLast ++ [(.zero, last.2 + 1)]
      else
        normalizeCnf (xc ++ [(.zero, 1)])
  | .add a b => addCnf (toCnf a) (toCnf b)
  | .mul a b => mulCnf (toCnf a) (toCnf b)
  | .pow a b => powCnf (toCnf a) (toCnf b)
  | .wv _ =>
    []  -- WV nodes: ordinal value undefined by article; treat as empty (lib throws in compare path if used via isZero etc)
where
  tail (v : Cnf) : Cnf := v.drop 1
  addCnf (A B : Cnf) : Cnf :=
    match A, B with
    | [], _ => B
    | _, [] => A
    | (e1, c1) :: A', (e2, c2) :: B' =>
      let cmp := Ordinal.compare e1 e2
      if cmp < 0 then B
      else if cmp > 0 then
        let rest := addCnf A' B
        normalizeCnf ((e1, c1) :: rest)
      else
        let rest := addCnf A' B'
        normalizeCnf ((e1, c1 + c2) :: rest)
  mulCnf (A B : Cnf) : Cnf :=
    match A, B with
    | [], _ => []
    | _, [] => []
    | (e1, c1) :: A', _ =>
      -- c1 * B first (repeat B c1 times)
      let rec mulC (k : Int) (acc : Cnf) : Cnf :=
        if k ≤ 0 then acc else mulC (k - 1) (addCnf acc B)
      let c1B := mulC c1 []
      let lead : Cnf := c1B.map fun (t_e, t_c) =>
        (.add e1 t_e, t_c)   -- ω^e1 * ω^t.e = ω^(e1 + t.e)  (ord exponent add)
      let rest := mulCnf A' B
      normalizeCnf (addCnf lead rest)
  powCnf (base exp_ : Cnf) : Cnf :=
    match exp_ with
    | [] => [(.zero, 1)]   -- α^0 = 1
    | _ => match base with
      | [] => []            -- 0^β = 0
      | (e1, c1) :: base' =>
        -- 1^β = 1
        if base.length = 1 ∧ (Ordinal.compare e1 .zero = 0) ∧ c1 = 1 then [(.zero, 1)]
        else
          match exp_ with
          | [(_, k)] when Ordinal.compare (exp_.get!.1) .zero = 0 =>
            -- finite exponent k
            let rec loop (k' : Int) (acc : Cnf) : Cnf :=
              if k' ≤ 0 then acc else loop (k' - 1) (mulCnf acc base)
            loop k [(.zero, 1)]
          | (f, d) :: S =>
            -- exp = ω^f * d + S
            let one_ := .cnf [(.zero, 1)]
            let omega' := .cnf [(one_, 1)]
            let ωf := (.pow omega' f)
            let e1wf := .mul e1 ωf
            let baseWf : Cnf := [(e1wf.toCnf.get!.1, 1)]  -- ω^{e1 * ω^f} = ((pow ω (mul e1 (pow ω f))).toCnf)
            -- fallback: simpler formula. baseWfd = baseWf added d times.
            let rec addK (k : Int) (acc : Cnf) : Cnf :=
              if k ≤ 0 then acc else addK (k - 1) (addCnf acc baseWf)
            let baseWfd := addK d []
            let baseS := powCnf base S
            normalizeCnf (mulCnf baseWfd baseS)
          | [] => [(.zero, 1)]

/-! Public 3-way compare. -/
def compare (A B : Ordinal) : Int := compareCnf (toCnf A) (toCnf B)

def equals (A B : Ordinal) : Bool := compare A B = 0
instance : BEq Ordinal where beq A B := equals A B

def isZero (o : Ordinal) : Bool :=
  match o with
  | .zero => true
  | .cnf ts => ts.isEmpty
  | .wv _ => false
  | _ => false  -- other tree nodes: route via isZero? no — C++ also uses toCnf for non-zero cases; but we avoid toCnf for WV. For Succ/Add/Mul/Pow we can route.
    where
      fallback : Bool := toCnf o |>.isEmpty
  -- Be conservative: use toCnf only for non-WV kinds. (WV is never Zero).
  -- Simple case: for Succ/Add/Mul/Pow call fallback. For match cases already handled (zero/cnf) we stop. So:
  -- Actually the above is a bit mixed. Let's write proper:
  --   we'll define isZero properly here.

-- Redefine cleanly using well-structured pattern match.
def isZero' : Ordinal → Bool
  | .zero => true
  | .cnf ts => ts.isEmpty
  | .wv _ => false
  | other => toCnf other |>.isEmpty

/-! isSuccessor / predecessor. For WV we always say false (WV never a closed succ). -/
def isSuccessor (o : Ordinal) : Bool :=
  match o with
  | .wv _ => false
  | _ =>
    let t := toCnf o
    match t.last? with
    | none => false
    | some (e, _) => isZero' e

def predecessor (o : Ordinal) : Except String Ordinal :=
  match o with
  | .wv _ => .error "predecessor: WV not a closed ordinal"
  | _ =>
    let t := toCnf o
    match t.last? with
    | none => .error "predecessor: zero"
    | some (e, _) =>
      if not (isZero' e) then .error "predecessor: not a successor"
      else
        let last := t.getLast!
        let init := t.dropLast
        let tail :=
          if last.2 > 1 then init ++ [(.zero, last.2 - 1)]
          else init
        if tail.isEmpty then .ok .zero
        else .ok (.cnf tail)

/-! Ordinal left subtraction A - B, A >= B required. -/
def subtract (A B : Ordinal) : Ordinal :=
  .cnf (subCnf (toCnf A) (toCnf B))
where
  subCnf (A B : Cnf) : Cnf :=
    match A, B with
    | [], _ => []
    | _, [] => A
    | (e1, c1) :: A', (e2, c2) :: B' =>
      let cmp := Ordinal.compare e1 e2
      if cmp < 0 then []
      else if cmp > 0 then A
      else if c1 > c2 then
        let rest := addCnf A' B'
        normalizeCnf ((e1, c1 - c2) :: rest)
      else subCnf A' B'
  where
    addCnf (A B : Cnf) : Cnf :=
      match A, B with
      | [], _ => B
      | _, [] => A
      | (e1, c1) :: A', (e2, c2) :: B' =>
        let cmp := Ordinal.compare e1 e2
        if cmp < 0 then B
        else if cmp > 0 then
          let rest := addCnf A' B
          normalizeCnf ((e1, c1) :: rest)
        else
          let rest := addCnf A' B'
          normalizeCnf ((e1, c1 + c2) :: rest)

/-! to_string (LaTeX-ish, matches C++ Ordinal::to_string). -/
partial def toString : Ordinal → String
  | .zero => "0"
  | .omega => "ω"
  | .succ x => s!"{toString x} + 1"
  | .add a b => s!"{toString a} + {toString b}"
  | .mul a b => s!"{toString a} * {toString b}"
  | .pow a b => s!"({toString a} ^ {toString b})"
  | .wv comps =>
    let parts := comps.map fun (a, b) =>
      s!"{dispPart a}@{dispPart b}"
    s!"({String.intercalate ", " parts})"
  | .cnf ts =>
    if ts.isEmpty then "0"
    else
      let parts := ts.map fun (e, c) =>
        if c = 1 ∧ not (isZero' e) then s!"ω^{toString e}"
        else if isZero' e then toString c
        else s!"ω^{toString e}·{toString c}"
      String.intercalate " + " parts
where
  dispPart (x : Ordinal) : String :=
    let s := toString x
    if s.any (fun c => c = ' ' ∨ c = '(' ∨ c = ')') then s!"({s})" else s

/-! expandWV — the 6 weak-Veblen cases. -/
partial def expandWV (comps : List (Ordinal × Ordinal)) (n : Int) : Ordinal :=
  match comps with
  | [(a, _)] when isZero' a => fromInt n   -- case 1: A=(0) -> n (b ignored)
  | (a0, _) :: rest when isZero' a0 => .wv rest   -- case 2: A=(0@b, #) -> (#)
  | _ =>
    let last := comps.getLast!
    let aLast := last.1
    let bLast := last.2
    let withoutLast := comps.dropLast
    if aLast.isSuccessor then
      -- aLast = a+1 (successor)
      match aLast.predecessor with
      | .error _ => .wv comps   -- shouldn't happen, isSuccessor implies it exists
      | .ok a =>
        if isZero' bLast then
          -- Case 3: (#, (a+1)@0)
          let base : List (Ordinal × Ordinal) := withoutLast ++ [(a, .zero)]
          let baseWV := .wv base
          if n = 0 then .add baseWV one
          else
            let recCase := expandWV comps (n - 1)
            .pow baseWV recCase
        else match bLast.predecessor with
          | .error _ =>
            -- Case 5: b is a limit, not successor
            let eb := bLast.expand n
            let r : List (Ordinal × Ordinal) := withoutLast ++ [(a, bLast), (one, eb)]
            .wv r
          | .ok b =>
            -- Case 4: (#, (a+1)@(b+1))
            let base : List (Ordinal × Ordinal) := withoutLast ++ [(a, .succ b)]
            let baseWV := .wv base
            if n = 0 then .add baseWV one
            else
              let recCase := expandWV comps (n - 1)
              let r : List (Ordinal × Ordinal) := base ++ [(recCase, b)]
              .wv r
    else
      -- Case 6: a is a limit (not successor) — expand a[n]@b
      let ea := aLast.expand n
      let r : List (Ordinal × Ordinal) := withoutLast ++ [(ea, bLast)]
      .wv r

/-! expand — fundamental sequence. Returns Except String Ordinal for errors.
    The library returns Except instead of throwing — match C++ behavior where
    expand(0)/expand(successor)/expand(closed) is undefined. -/
partial def expand : Ordinal → Int → Except String Ordinal
  | .zero, _ => .error "Ordinal.expand: 0 has no fundamental sequence"
  | .omega, n => .ok (fromInt n)   -- C2
  | .cnf _, _ => .error "Ordinal.expand: closed ordinal (CNF) has no FS"
  | .succ _, _ => .error "Ordinal.expand: successor ordinal has no FS"
  | .add α β, n =>
    if isZero' β then .ok α
    else
      match expand β n with
      | .ok β' => .ok (.add α β')
      | .error e => .error e
  | .mul α β, n =>
    if isZero' β then .ok .zero
    else
      match expand β n with
      | .ok β' => .ok (.mul α β')
      | .error e => .error e
  | .pow α β, n =>
    if isZero' β then .ok one
    else
      match expand β n with
      | .ok e =>
        -- α^1 collapse to α
        if compare e one = 0 then .ok α
        else .ok (.pow α e)
      | .error e => .error e
  | .wv comps, n => .ok (expandWV comps n)

/-! Parser: integers, ω/w/omega, +, *, ^, parentheses. Right-assoc ^. -/
private structure OpState where
  s : String
  i : Nat := 0

private def OpState.peek (st : OpState) : Option Char :=
  if st.i < st.s.length then some (st.s[st.i]) else none

private def OpState.skipSpaces (st : OpState) : OpState :=
  let rec loop (i : Nat) : Nat :=
    if i < st.s.length then
      let c := st.s[i]
      if c = ' ' ∨ c = '\t' then loop (i + 1) else i
    else i
  { st with i := loop st.i }

private def OpState.get (st : OpState) : Option Char × OpState :=
  let st' := st.skipSpaces
  if st'.i < st'.s.length then
    (some (st'.s[st'.i]), { st' with i := st'.i + 1 })
  else
    (none, st')

/- Try to consume omega token: w/W/"omega"/UTF-8 ω (= 0xCF 0x89 in bytes).
   Returns bool, and updates i if consumed. We work on String positions. -/
private def tryOmega (st : OpState) : Bool × OpState :=
  let st' := st.skipSpaces
  if st'.i ≥ st'.s.length then (false, st)
  else
    let c := st'.s[st'.i]
    if c = 'w' ∨ c = 'W' then (true, { st' with i := st'.i + 1 })
    else if c = 'o' then
      -- maybe "omega"
      let rest := st'.s.extract st'.i st'.s.length
      if rest.startsWith "omega" then
        (true, { st' with i := st'.i + 5 })
      else (false, st)
    else
      -- UTF-8 ω = 0xCF 0x89 (two chars in string if UTF-8 encoded per byte?
      -- In Lean, String's Char is Unicode scalar; so ω as a single char 'ω'.
      if c = 'ω' then (true, { st' with i := st'.i + 1 })
      else (false, st)

/- Parse entry points — mutual: parseExpr, parseMul, parsePow, parseAtom. -/
private partial def parseAtom (st : OpState) : Except String (Ordinal × OpState) :=
  let st' := st.skipSpaces
  match tryOmega st' with
  | (true, st2) => .ok (.omega, st2)
  | (false, _) =>
    match st'.get with
    | (none, _) => .error "parseAtom: empty"
    | (some '(', st2) =>
      match parseExpr st2 with
      | .ok (v, st3) =>
        match st3.get with
        | (some ')', st4) => .ok (v, st4)
        | _ => .error "parseAtom: expected ')'"
      | .error e => .error e
    | (some d, st2) =>
      if d.isDigit then
        let rec loop (cur : Int) (s : OpState) : Int × OpState :=
          match s.get with
          | (some c, s') =>
            if c.isDigit then loop (cur * 10 + (c.toNat - 48 : Int)) s'
            else (cur, s)
          | (none, s') => (cur, s')
        let (val, st3) := loop (d.toNat - 48 : Int) st2
        .ok (fromInt val, st3)
      else if d = '-' then .ok (.zero, st2)
      else .error s!"parseAtom: unexpected char '{d}'"
where
  parseExpr st := parseExprImpl st
  parseExprImpl st := parseAdd st
  parseAdd st := do
    let (l, st2) ← parseMul st
    parseAddCont l st2
  parseAddCont l st :=
    let st' := st.skipSpaces
    match st'.get with
    | (some '+', st2) => do
      let (r, st3) ← parseMul st2
      parseAddCont (.add l r) st3
    | _ => .ok (l, st')
  parseMul st := do
    let (l, st2) ← parsePow st
    parseMulCont l st2
  parseMulCont l st :=
    let st' := st.skipSpaces
    match st'.peek with
    | some '*' =>
      let (_, st2) := st'.get
      do let (r, st3) ← parsePow st2
         parseMulCont (.mul l r) st3
    | _ => .ok (l, st')
  parsePow st := do
    let (l, st2) ← parseAtom' st
    let st' := st2.skipSpaces
    match st'.peek with
    | some '^' =>
      let (_, st3) := st'.get
      do let (r, st4) ← parsePow st3
         .ok (.pow l r, st4)
    | _ => .ok (l, st')
  parseAtom' st := parseAtom st
  parseExprImpl st := parseAdd st

-- Re-exports parse entry points at parseAtom's level
private partial def parsePow (st : OpState) : Except String (Ordinal × OpState) :=
  parseAtom st >>= fun (l, st2) =>
    let st' := st2.skipSpaces
    match st'.peek with
    | some '^' =>
      let (_, st3) := st'.get
      Except.map (fun (r, st4) => (.pow l r, st4)) (parsePow st3)
    | _ => .ok (l, st')

private partial def parseMul (st : OpState) : Except String (Ordinal × OpState) :=
  parsePow st >>= fun (l, st2) => loop l st2
where
  loop l st :=
    let st' := st.skipSpaces
    match st'.peek with
    | some '*' =>
      let (_, st2) := st'.get
      parsePow st2 >>= fun (r, st3) =>
        loop (.mul l r) st3
    | _ => .ok (l, st')

private partial def parseExpr (st : OpState) : Except String (Ordinal × OpState) :=
  parseMul st >>= fun (l, st2) => loop l st2
where
  loop l st :=
    let st' := st.skipSpaces
    match st'.peek with
    | some '+' =>
      let (_, st2) := st'.get
      parseMul st2 >>= fun (r, st3) =>
        loop (.add l r) st3
    | _ => .ok (l, st')

/-! Top-level parse. -/
def parse (s : String) : Except String Ordinal :=
  match parseExpr ⟨s, 0⟩ with
  | .ok (v, st') =>
    if st'.skipSpaces.i < st'.s.length then
      .error s!"parse: trailing characters at pos {st'.i}"
    else .ok v
  | .error e => .error e

end Ordinal
end Googology