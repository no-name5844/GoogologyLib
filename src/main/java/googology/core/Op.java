package googology.core;

/** Operations a notation may support (capability model). */
public enum Op {
    FROM_STRING(0),
    TO_STRING(1),
    NORMALIZE(2),
    COMPARE(3),
    EXPAND(4),
    EXPAND_TO(5),
    SUCCESSOR(6);

    private final int value;

    Op(int value) { this.value = value; }
    public int value() { return value; }
}