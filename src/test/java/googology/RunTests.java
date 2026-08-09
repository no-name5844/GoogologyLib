package googology;

import googology.core.*;
import googology.notations.number.*;

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

        System.out.println("\n=== Results: " + passed + " passed, " + failed + " failed ===");
        System.exit(failed > 0 ? 1 : 0);
    }
}