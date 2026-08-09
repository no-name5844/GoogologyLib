package googology.core;

/** Exception for non-comparable notations (large-number notations). */
public class NotComparable extends RuntimeException {
    public NotComparable(String name) {
        super("Notation '" + name + "': comparison is undefined for large-number notations");
    }
}