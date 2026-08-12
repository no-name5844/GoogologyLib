package googology.notations.ordinal.sequence.marked_parent;

import googology.core.*;
import java.util.ArrayList;
import java.util.List;

/**
 * PPS (Parented Predecessor Sequence) — marked-parent ordinal sequence
 * notation. Created by 318`4. Variants: PPS1 (original) / PPS2 / PPS4 /
 * wPPS4 / tPPS4 / fPPS4. PPS3 / ePPS4 / sPPS4 are N/A (no data) — not
 * implemented.
 *
 * Bad root = the x-th term (x = last value); b = bad-root value; L = y - x
 * (y = last column). expandLen replaces the last term (per variant) and
 * appends elements from the running sequence: a_i >= x -> a_i + L, else a_i.
 * 基本列[n] = expand to the (y + nL - 1)-th term (n >= 1). Library 0-index:
 * expand(k) = 基本列[k+1] = expandLen((k+1)L - 1). compare is lexicographic;
 * cross-variant / cross-family throws NotComparable.
 */
public abstract class Pps extends Notation {

    /** PPS variants. */
    public enum Variant {
        PPS1("pps1", "1"), PPS2("pps2", "2"), PPS4("pps4", "4"),
        WPPS4("wpps4", "4w"), TPPS4("tpps4", "4t"), FPPS4("fpps4", "4f");

        public final String regName;
        public final String ver;

        Variant(String n, String v) { regName = n; ver = v; }
    }

    private final Variant variant;
    protected final List<Long> seq = new ArrayList<>();
    protected boolean isMasterLimit = false;

    protected Pps(Variant v) { variant = v; }
    protected Pps(Variant v, String s) { variant = v; stringToIt(s); }

    @Override public String name() { return variant.regName; }
    @Override public Family family() { return Family.ORDINAL; }
    @Override public String subfamily() { return "sequence"; }
    @Override public String style() { return "marked_parent"; }
    @Override public String creator() { return "318`4"; }
    @Override public String version() { return variant.ver; }

    @Override
    public Capabilities capabilities() {
        var c = new Capabilities();
        c.set(Op.FROM_STRING);
        c.set(Op.TO_STRING);
        c.set(Op.NORMALIZE);
        c.set(Op.COMPARE);
        c.set(Op.EXPAND);
        c.set(Op.EXPAND_TO);
        c.set(Op.SUCCESSOR);
        return c;
    }

    // --- internal helpers ---

    /** Rightmost 1-based column j in the open interval (lo, hi) with value = b. */
    private long rightmostEq(long lo, long hi, long b) {
        for (long j = hi - 1; j > lo; j--) {
            if (j < 1 || j > seq.size()) continue;
            if (seq.get((int) (j - 1)) == b) return j;
        }
        return 0;
    }

    /** Lexicographic compare of a_{k+j} vs a_{c+j} (j = 0,1,2,...). */
    private int tailCmp(long k, long c) {
        long y = seq.size();
        for (long j = 0; ; j++) {
            long ik = k + j, ic = c + j;
            boolean okK = ik >= 1 && ik <= y;
            boolean okC = ic >= 1 && ic <= y;
            if (!okK && !okC) return 0;
            if (!okK) return -1;
            if (!okC) return 1;
            long vk = seq.get((int) (ik - 1)), vc = seq.get((int) (ic - 1));
            if (vk != vc) return vk > vc ? 1 : -1;
        }
    }

    /** Per-variant last-term replacement: {newLast, strong}. */
    private long[] replaceLast(long x, long b, long y) {
        switch (variant) {
            case PPS1: {
                long k = rightmostEq(x, y, b);
                return new long[]{ k != 0 ? b : x - 1, 0 };
            }
            case PPS2: {
                long k = rightmostEq(x, y, b);
                return new long[]{ k != 0 && tailCmp(k, x) > 0 ? b : x - 1, 0 };
            }
            default: {
                long k = rightmostEq(x, y, b);
                if (k != 0) return new long[]{ b, 0 };          // weak expansion
                long j = rightmostEq(b, x, b);                  // strong expansion
                return new long[]{ j != 0 ? j : b, 1 };         // not found -> as weak
            }
        }
    }

    /** Copy rule; tPPS4/fPPS4 strong-branch modifiers on the last-term chain. */
    private long copied(long src, long x, long L, long y, long i, boolean strong) {
        long base = src >= x ? src + L : src;
        if (!strong) return base;
        if (variant == Variant.TPPS4 || variant == Variant.FPPS4) {
            if (i % L == y % L) {
                if (variant == Variant.FPPS4 && i == y) return x;
                return src + L;
            }
        }
        return base;
    }

    /** expandLen(A, M): replace last + append M elements (source i = q + y - L). */
    private void expandLen(long M) {
        if (seq.isEmpty()) return;
        long y = seq.size();
        long x = seq.get(seq.size() - 1);
        if (x == 0) { seq.remove(seq.size() - 1); return; }   // successor
        if (x >= y) return;                                    // bad root OOR (C++ throws)
        long b = seq.get((int) (x - 1));
        long L = y - x;
        long[] r = replaceLast(x, b, y);
        seq.set((int) (y - 1), r[0]);
        boolean strong = r[1] != 0;
        for (long q = 1; q <= M; q++) {
            long i = q + y - L;
            if (i < 1 || i > seq.size()) return;               // defensive
            long src = seq.get((int) (i - 1));
            seq.add(copied(src, x, L, y, i, strong));
        }
    }

    @Override
    public void stringToIt(String s) {
        seq.clear();
        isMasterLimit = false;
        String t = s.replaceAll("\\s", "");
        while (!t.isEmpty() && "([{".indexOf(t.charAt(0)) >= 0) t = t.substring(1);
        while (!t.isEmpty() && ")]}".indexOf(t.charAt(t.length() - 1)) >= 0)
            t = t.substring(0, t.length() - 1);
        if (t.isEmpty()) return;
        for (String p : t.split(",")) if (!p.isEmpty()) seq.add(Long.parseLong(p));
    }

    @Override
    public String toLatex() {
        if (isMasterLimit) return "(0, 1, 2, …)";
        StringBuilder sb = new StringBuilder("(");
        for (int i = 0; i < seq.size(); i++) {
            if (i > 0) sb.append(", ");
            sb.append(seq.get(i));
        }
        sb.append(")");
        return sb.toString();
    }

    /** Library 0-index: expand(k) = 基本列[k+1] = expandLen((k+1)L - 1). */
    @Override
    public Notation expand(int n) {
        if (isMasterLimit) {
            seq.clear();
            for (long i = 0; i < n; i++) seq.add(i);
            isMasterLimit = false;
            return this;
        }
        if (seq.isEmpty()) return this;
        long x = seq.get(seq.size() - 1);
        if (x == 0) { seq.remove(seq.size() - 1); return this; }
        long y = seq.size();
        if (x >= y) return this;   // bad root OOR (C++ throws)
        long L = y - x;
        expandLen((n + 1) * L - 1);
        return this;
    }

    @Override
    public Notation expandTo(int length) {
        expandLen(length);
        return this;
    }

    @Override
    public int compare(Notation other) {
        if (!(other instanceof Pps)) throw new NotComparable(name());
        Pps o = (Pps) other;
        if (variant != o.variant) throw new NotComparable(name());
        if (isMasterLimit || o.isMasterLimit) {
            if (isMasterLimit && o.isMasterLimit) return 0;
            return isMasterLimit ? 1 : -1;
        }
        int m = Math.min(seq.size(), o.seq.size());
        for (int i = 0; i < m; i++) {
            long a = seq.get(i), b = o.seq.get(i);
            if (a > b) return 1;
            if (a < b) return -1;
        }
        return Integer.compare(seq.size(), o.seq.size());
    }

    /** Sequence ends with 0 => successor. */
    public boolean isSuccessor() {
        return !isMasterLimit && !seq.isEmpty() && seq.get(seq.size() - 1) == 0;
    }

    /** limit(n) = (0,1,...,n-1) of this variant. */
    public Pps limit(int n) {
        Pps p = newInstance();
        for (long i = 0; i < n; i++) p.seq.add(i);
        return p;
    }

    /** Master limit expression of this variant (supremum). */
    public Pps masterLimit() {
        Pps p = newInstance();
        p.isMasterLimit = true;
        return p;
    }

    private Pps newInstance() {
        try {
            return getClass().getDeclaredConstructor().newInstance();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    // --- concrete variants (same module, per project convention) ---

    public static class Pps1 extends Pps {
        public Pps1() { super(Variant.PPS1); }
        public Pps1(String s) { super(Variant.PPS1, s); }
    }

    public static class Pps2 extends Pps {
        public Pps2() { super(Variant.PPS2); }
        public Pps2(String s) { super(Variant.PPS2, s); }
    }

    public static class Pps4 extends Pps {
        public Pps4() { super(Variant.PPS4); }
        public Pps4(String s) { super(Variant.PPS4, s); }
    }

    public static class WPps4 extends Pps {
        public WPps4() { super(Variant.WPPS4); }
        public WPps4(String s) { super(Variant.WPPS4, s); }
    }

    public static class TPps4 extends Pps {
        public TPps4() { super(Variant.TPPS4); }
        public TPps4(String s) { super(Variant.TPPS4, s); }
    }

    public static class FPps4 extends Pps {
        public FPps4() { super(Variant.FPPS4); }
        public FPps4(String s) { super(Variant.FPPS4, s); }
    }
}
