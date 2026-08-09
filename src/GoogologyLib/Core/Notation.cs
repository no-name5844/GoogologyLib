namespace GoogologyLib.Core;

/// <summary>Exception for unsupported operations.</summary>
public class UnsupportedOperation : InvalidOperationException
{
    public UnsupportedOperation(string name, Op op)
        : base($"Notation '{name}' does not implement the requested operation") { }
}

/// <summary>Exception for non-comparable notations (large-number notations).</summary>
public class NotComparable : InvalidOperationException
{
    public NotComparable(string name)
        : base($"Notation '{name}': comparison is undefined for large-number notations") { }
}

/// <summary>Generic notation base. Every notation derives from this class.</summary>
public abstract class Notation
{
    // --- taxonomy ---
    public abstract string Name { get; }
    public abstract Family Family { get; }
    public virtual string Subfamily => "";
    public virtual string Style => "";

    // --- attribution ---
    public virtual string Creator => "";
    public virtual string Version => "";

    // --- capability ---
    public abstract Capabilities Capabilities { get; }
    public bool Can(Op op) => Capabilities.Has(op);

    // --- string conversion ---
    public virtual void StringToIt(string s) =>
        throw new UnsupportedOperation(Name, Op.FromString);

    public abstract string ToLatex();

    // --- operations ---
    public virtual int Compare(Notation other) =>
        throw new NotComparable(Name);

    public virtual Notation Expand(int n = 1) =>
        throw new UnsupportedOperation(Name, Op.Expand);

    public virtual Notation ExpandTo(int length) =>
        throw new UnsupportedOperation(Name, Op.ExpandTo);

    public bool Comparable => Can(Op.Compare);

    public Notation Reduce()
    {
        var prev = ToLatex();
        for (int i = 0; i < 1000; i++)
        {
            Expand(1);
            var cur = ToLatex();
            if (cur == prev) return this;
            prev = cur;
        }
        return this;
    }

    public override string ToString() => ToLatex();
}