package googology;

import googology.core.*;
import googology.notations.number.*;
import googology.notations.ordinal.sequence.marked_parent.Pps;

/** Simple test runner (no JUnit dependency needed). */
public class RunTests {
    static int passed = 0, failed = 0;

    static void check(String name, Runnable test) {
        try {
            test.run();
            passed++;
            System.out.println("  PASS: " + name);
        } catch (Throwable e) {
            failed++;
            System.out.println("  FAIL: " + name + " (" + e.getMessage() + ")");
        }
    }

    public static void main(String[] args) {
        System.out.println("=== Knuth tests ===");
        check("parse and string", () -> {
            var k = new Knuth("2 ^^ 3");
            assert k.toLatex().contains("uparrow") : "no uparrow";
        });
        check("expand once", () -> {
            var k = new Knuth("2 ^^ 3");
            k.expand(1);
            assert k.toLatex().contains("uparrow") : "no uparrow after expand";
        });
        check("not comparable", () -> {
            var k = new Knuth("2 ^^ 3");
            try { k.compare(new Knuth("3 ^^ 3")); assert false : "should throw"; }
            catch (NotComparable e) { /* expected */ }
        });
        check("capabilities", () -> {
            var k = new Knuth("2 ^^ 3");
            assert k.can(Op.FROM_STRING);
            assert k.can(Op.TO_STRING);
            assert k.can(Op.EXPAND);
            assert k.can(Op.EXPAND_TO);
            assert !k.can(Op.COMPARE);
        });

        System.out.println("=== Conway tests ===");
        check("parse and string", () -> {
            var c = new Conway("3 -> 3 -> 2");
            assert c.toLatex().contains("rightarrow") : "no rightarrow";
        });
        check("expand once", () -> {
            var c = new Conway("3 -> 3 -> 2");
            c.expand(1);
            assert c.toLatex().contains("rightarrow") : "no rightarrow after expand";
        });
        check("trailing one", () -> {
            var c = new Conway("3 -> 4 -> 1");
            c.expand(1);
            assert c.toLatex().contains("4") : "no 4";
        });
        check("capabilities", () -> {
            var c = new Conway("3 -> 3 -> 2");
            assert c.can(Op.FROM_STRING);
            assert c.can(Op.TO_STRING);
            assert c.can(Op.EXPAND);
            assert c.can(Op.EXPAND_TO);
            assert !c.can(Op.COMPARE);
        });

        System.out.println("=== PPS tests ===");
        check("pps1 parse/string", () -> {
            var p = new Pps.Pps1("(0, 1, 0, 3)");
            assert p.toLatex().equals("(0, 1, 0, 3)") : p.toLatex();
        });
        check("pps1 (0,1,2).expand(0)", () -> {
            var p = new Pps.Pps1("(0,1,2)");
            p.expand(0);
            assert p.toLatex().equals("(0, 1, 1)") : p.toLatex();
        });
        check("pps1 (0,1,2).expand(2)", () -> {
            var p = new Pps.Pps1("(0,1,2)");
            p.expand(2);
            assert p.toLatex().equals("(0, 1, 1, 1, 1)") : p.toLatex();
        });
        check("pps1 (0,1,0,3).expand(0)", () -> {
            var p = new Pps.Pps1("(0,1,0,3)");
            p.expand(0);
            assert p.toLatex().equals("(0, 1, 0, 2)") : p.toLatex();
        });
        check("pps1 (0,1,0,1,2).expand(0)", () -> {
            var p = new Pps.Pps1("(0,1,0,1,2)");
            p.expand(0);
            assert p.toLatex().equals("(0, 1, 0, 1, 1, 0, 1)") : p.toLatex();
        });
        check("pps1 successor strips 0", () -> {
            var p = new Pps.Pps1("(0,1,0)");
            p.expand(3);
            assert p.toLatex().equals("(0, 1)") : p.toLatex();
            assert new Pps.Pps1("(0,1,0)").isSuccessor();
        });
        check("pps1 expand_to", () -> {
            var p = new Pps.Pps1("(0,1,2)");
            p.expandTo(2);
            assert p.toLatex().equals("(0, 1, 1, 1, 1)") : p.toLatex();
        });
        check("pps1 vs pps2 (0,2,4,4,2,3)", () -> {
            var p1 = new Pps.Pps1("(0,2,4,4,2,3)");
            p1.expand(0);
            assert p1.toLatex().equals("(0, 2, 4, 4, 2, 4, 7, 2)") : p1.toLatex();
            var p2 = new Pps.Pps2("(0,2,4,4,2,3)");
            p2.expand(0);
            assert p2.toLatex().equals("(0, 2, 4, 4, 2, 2, 7, 2)") : p2.toLatex();
        });
        check("pps4 strong col expand(0)", () -> {
            var p = new Pps.Pps4("(0,1,0,2,2,3)");
            p.expand(0);
            assert p.toLatex().equals("(0, 1, 0, 2, 2, 1, 2, 2)") : p.toLatex();
        });
        check("pps4 strong not-found", () -> {
            var p = new Pps.Pps4("(0,1,2,1,3)");
            p.expand(0);
            assert p.toLatex().equals("(0, 1, 2, 1, 2, 1)") : p.toLatex();
        });
        check("wpps4 same as pps4", () -> {
            var p = new Pps.WPps4("(0,1,0,2,2,3)");
            p.expand(0);
            assert p.toLatex().equals("(0, 1, 0, 2, 2, 1, 2, 2)") : p.toLatex();
        });
        check("tpps4 expand(1)", () -> {
            var p = new Pps.TPps4("(0,1,0,2,2,3)");
            p.expand(1);
            assert p.toLatex().equals("(0, 1, 0, 2, 2, 1, 2, 2, 4, 2, 2)") : p.toLatex();
        });
        check("fpps4 expand(1)", () -> {
            var p = new Pps.FPps4("(0,1,0,2,2,3)");
            p.expand(1);
            assert p.toLatex().equals("(0, 1, 0, 2, 2, 1, 2, 2, 3, 2, 2)") : p.toLatex();
        });
        check("pps4 expand(1) (no strong copy mod)", () -> {
            var p = new Pps.Pps4("(0,1,0,2,2,3)");
            p.expand(1);
            assert p.toLatex().equals("(0, 1, 0, 2, 2, 1, 2, 2, 1, 2, 2)") : p.toLatex();
        });
        check("limit/master_limit", () -> {
            var l = new Pps.Pps1().limit(3);
            assert l.toLatex().equals("(0, 1, 2)") : l.toLatex();
            var m = new Pps.Pps1().masterLimit();
            m.expand(3);
            assert m.toLatex().equals("(0, 1, 2)") : m.toLatex();
        });
        check("compare lexicographic", () -> {
            assert new Pps.Pps1("(0,1,2)").compare(new Pps.Pps1("(0,1,3)")) == -1;
            assert new Pps.Pps1("(0,1,3)").compare(new Pps.Pps1("(0,1,2)")) == 1;
            assert new Pps.Pps1("(0,1,2)").compare(new Pps.Pps1("(0,1,2,0)")) == -1;
        });
        check("cross-variant compare throws", () -> {
            try {
                new Pps.Pps1("(0,1,2)").compare(new Pps.Pps2("(0,1,2)"));
                assert false : "should throw";
            } catch (NotComparable e) { /* expected */ }
        });
        check("taxonomy/capabilities", () -> {
            var p = new Pps.Pps1("(0,1,2)");
            assert p.name().equals("pps1");
            assert p.style().equals("marked_parent");
            assert p.creator().equals("318`4");
            assert p.version().equals("1");
            assert new Pps.TPps4().name().equals("tpps4");
            assert new Pps.TPps4().version().equals("4t");
            assert p.can(Op.COMPARE) && p.can(Op.EXPAND);
        });

        System.out.println("\n=== Results: " + passed + " passed, " + failed + " failed ===");
        System.exit(failed > 0 ? 1 : 0);
    }}
