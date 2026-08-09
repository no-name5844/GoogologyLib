package googology.core;

/** Exception for unsupported operations. */
public class UnsupportedOperation extends RuntimeException {
    public UnsupportedOperation(String name, Op op) {
        super("Notation '" + name + "' does not implement the requested operation");
    }
}