package googology.notations.number;

import googology.core.*;

/** Knuth up-arrow notation (高德纳箭头). */
public class Knuth extends Notation {
    private KNode root = KNode.value(0);

    public Knuth() {}
    public Knuth(String s) { stringToIt(s); }

    @Override public String name() { return "knuth"; }
    @Override public Family family() { return Family.NUMBER; }
    @Override public String creator() { return "Donald Knuth"; }
    @Override public String version() { return "1"; }

    @Override
    public Capabilities capabilities() {
        var c = new Capabilities();
        c.set(Op.FROM_STRING);
        c.set(Op.TO_STRING);
        c.set(Op.EXPAND);
        c.set(Op.EXPAND_TO);
        return c;
    }

    @Override
    public void stringToIt(String s) {
        var t = s.replace("↑", "^").replace(" ", "");
        if (t.isEmpty()) throw new IllegalArgumentException("Knuth: empty input");
        root = parseExpr(t);
    }

    @Override
    public String toLatex() { return root.tex(); }

    @Override
    public Notation expand(int n) {
        for (int i = 0; i < n; i++) root = root.step();
        return this;
    }

    @Override
    public Notation expandTo(int length) {
        String prev = toLatex();
        while (true) {
            expand(1);
            String cur = toLatex();
            if (cur.length() >= length || cur.equals(prev)) return this;
            prev = cur;
        }
    }

    // --- internal AST ---

    private static class KNode {
        boolean isVal;
        long val;
        long baseVal;
        long height;
        KNode exp;

        static KNode value(long v) { var n = new KNode(); n.isVal = true; n.val = v; return n; }
        static KNode arrow(long base, long h, KNode e) {
            var n = new KNode(); n.isVal = false; n.baseVal = base; n.height = h; n.exp = e; return n;
        }

        String tex() {
            if (isVal) return Long.toString(val);
            var bt = Long.toString(baseVal);
            var et = exp != null ? exp.tex() : "";
            var arr = height == 1 ? "\\uparrow" : "\\uparrow^{" + height + "}";
            var expHasArrow = exp != null && !exp.isVal;
            var expPart = expHasArrow ? "(" + et + ")" : et;
            return bt + " " + arr + " " + expPart;
        }

        KNode step() {
            if (isVal) return this;
            if (exp != null && exp.isVal) {
                long b = exp.val;
                if (b == 1) return value(baseVal);  // a ^c 1 = a
                if (height == 1) return this;  // a^b: terminal
                var inner = arrow(baseVal, height, value(b - 1));
                return arrow(baseVal, height - 1, inner);
            }
            // recurse into the rightmost sub-term
            return arrow(baseVal, height, exp.step());
        }
    }

    private static KNode parseExpr(String s) {
        int pos = s.indexOf('^');
        if (pos < 0) {
            if (s.isEmpty()) throw new IllegalArgumentException("Knuth: empty operand");
            return KNode.value(Long.parseLong(s));
        }
        long a = Long.parseLong(s.substring(0, pos));
        int j = pos;
        int c = 0;
        while (j < s.length() && s.charAt(j) == '^') { c++; j++; }
        String rest = s.substring(j);
        if (rest.isEmpty()) throw new IllegalArgumentException("Knuth: dangling arrows (no exponent)");
        var exp = parseExpr(rest);
        return KNode.arrow(a, c, exp);
    }
}