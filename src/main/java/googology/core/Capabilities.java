package googology.core;

/** Bitset of supported operations. */
public class Capabilities {
    private byte bits = 0;

    public void set(Op op) {
        bits |= (byte) (1 << op.value());
    }

    public boolean has(Op op) {
        return (bits & (byte) (1 << op.value())) != 0;
    }
}