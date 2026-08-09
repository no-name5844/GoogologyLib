package googology;

import googology.core.*;
import googology.notations.number.Knuth;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class KnuthTest {
    @Test
    void testParseAndString() {
        var k = new Knuth("2 ^^ 3");
        assertTrue(k.toLatex().contains("uparrow"));
    }

    @Test
    void testExpandOnce() {
        var k = new Knuth("2 ^^ 3");
        k.expand(1);
        assertTrue(k.toLatex().contains("uparrow"));
    }

    @Test
    void testNotComparable() {
        var k = new Knuth("2 ^^ 3");
        assertThrows(NotComparable.class, () -> k.compare(new Knuth("3 ^^ 3")));
    }

    @Test
    void testCapabilities() {
        var k = new Knuth("2 ^^ 3");
        assertTrue(k.can(Op.FROM_STRING));
        assertTrue(k.can(Op.TO_STRING));
        assertTrue(k.can(Op.EXPAND));
        assertTrue(k.can(Op.EXPAND_TO));
        assertFalse(k.can(Op.COMPARE));
    }
}