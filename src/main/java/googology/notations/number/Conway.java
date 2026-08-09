package googology.notations.number;

import java.util.*;
import java.util.stream.*;
import googology.core.*;

/** Conway chained-arrow notation (康威链式箭头). */
public class Conway extends Notation {
    private List<CNode> chain = new ArrayList<>();

    public Conway() {}
    public Conway(String s) { stringToIt(s); }

    @Override public String name() { return "conway"; }
    @Override public Family family() { return Family.NUMBER; }
    @Override public String creator() { return "John Conway"; }
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
        var t = s.replace("→", "->").replace(" ", "");
        chain = Arrays.stream(t.split("->"))
            .map(part -> CNode.value(Long.parseLong(part)))
            .collect(Collectors.toList());
    }

    @Override
    public String toLatex() { return ser(chain); }

    @Override
    public Notation expand(int n) {
        for (int i = 0; i < n; i++) chain = stepOnce(chain);
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

    // --- internal ---

    private static class CNode {
        boolean isInt;
        long val;
        List<CNode> sub;

        static CNode value(long v) { var n = new CNode(); n.isInt = true; n.val = v; return n; }
        static CNode subChain(List<CNode> s) { var n = new CNode(); n.isInt = false; n.sub = s; return n; }
    }

    private static String ser(List<CNode> ch) {
        if (ch.size() == 2 && ch.get(0).isInt && ch.get(1).isInt)
            return ch.get(0).val + "^{" + ch.get(1).val + "}";
        return ch.stream()
            .map(node -> node.isInt ? Long.toString(node.val) : "(" + ser(node.sub) + ")")
            .collect(Collectors.joining(" \\rightarrow "));
    }

    private static List<CNode> stepOnce(List<CNode> ch) {
        int n = ch.size();
        if (n <= 2) {
            if (n <= 1) return ch;
            if (ch.get(0).isInt && ch.get(1).isInt) return ch;  // terminal
            return recurseSub(ch);
        }

        // rule 2a: trailing 1
        if (ch.get(n - 1).isInt && ch.get(n - 1).val == 1)
            return ch.subList(0, n - 1);

        // rule 2b: middle 1
        if (ch.get(n - 2).isInt && ch.get(n - 2).val == 1)
            return ch.subList(0, n - 2);

        // rule 3: X -> a -> b = X -> (X -> a-1 -> b) -> b-1
        if (ch.get(n - 1).isInt && ch.get(n - 2).isInt) {
            long a = ch.get(n - 2).val;
            long b = ch.get(n - 1).val;
            var x = ch.subList(0, n - 2);
            var inner = new ArrayList<>(x);
            inner.add(CNode.value(a - 1));
            inner.add(CNode.value(b));
            var result = new ArrayList<>(x);
            result.add(CNode.subChain(inner));
            result.add(CNode.value(b - 1));
            return result;
        }

        return recurseSub(ch);
    }

    private static List<CNode> recurseSub(List<CNode> ch) {
        boolean changed = false;
        var r = new ArrayList<CNode>();
        for (var node : ch) {
            if (!node.isInt) {
                String before = ser(node.sub);
                var s = stepOnce(node.sub);
                String after = ser(s);
                if (!after.equals(before)) {
                    changed = true;
                    if (s.size() == 1 && s.get(0).isInt)
                        r.add(CNode.value(s.get(0).val));
                    else
                        r.add(CNode.subChain(s));
                    continue;
                }
            }
            r.add(node);
        }
        return changed ? r : ch;
    }
}