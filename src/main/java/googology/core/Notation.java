package googology.core;

/** Generic notation base. Every notation extends this class. */
public abstract class Notation {
    // --- taxonomy ---
    public abstract String name();
    public abstract Family family();
    public String subfamily() { return ""; }
    public String style() { return ""; }

    // --- attribution ---
    public String creator() { return ""; }
    public String version() { return ""; }

    // --- capability ---
    public abstract Capabilities capabilities();
    public boolean can(Op op) { return capabilities().has(op); }

    // --- string conversion ---
    public void stringToIt(String s) {
        throw new UnsupportedOperation(name(), Op.FROM_STRING);
    }

    public abstract String toLatex();

    // --- operations ---
    public int compare(Notation other) {
        throw new NotComparable(name());
    }

    public Notation expand(int n) {
        throw new UnsupportedOperation(name(), Op.EXPAND);
    }

    public Notation expandTo(int length) {
        throw new UnsupportedOperation(name(), Op.EXPAND_TO);
    }

    public boolean comparable() { return can(Op.COMPARE); }

    public Notation reduce() {
        String prev = toLatex();
        for (int i = 0; i < 1000; i++) {
            expand(1);
            String cur = toLatex();
            if (cur.equals(prev)) return this;
            prev = cur;
        }
        return this;
    }

    @Override
    public String toString() { return toLatex(); }
}