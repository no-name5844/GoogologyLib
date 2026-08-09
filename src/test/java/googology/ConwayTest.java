package googology;

import googology.core.*;
import googology.notations.number.Conway;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class ConwayTest {
    @Test
    void testParseAndString() {
        var c = new Conway("3 -> 3 -> 2");
        assertTrue(c.toLatex().contains("rightarrow"));
    }

    @Test
    void testExpandOnce() {
        var c = new Conway("3 -> 3 -> 2");
        c.expand(1);
        assertTrue(c.toLatex().contains("rightarrow"));
    }

    @Test
    void testTrailingOne() {
        var c = new Conway("3 -> 4 -> 1");
        c.expand(1);
        assertTrue(c.toLatex().contains("4"));
    }

    @Test
    void testCapabilities() {
        var c = new Conway("3 -> 3 -> 2");
        assertTrue(c.can(Op.FROM_STRING));
        assertTrue(c.can(Op.TO_STRING));
        assertTrue(c.can(Op.EXPAND));
        assertTrue(c.can(Op.EXPAND_TO));
        assertFalse(c.can(Op.COMPARE));
    }
}