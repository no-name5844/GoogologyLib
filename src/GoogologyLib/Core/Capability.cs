namespace GoogologyLib.Core;

/// <summary>Operations a notation may support (capability model).</summary>
public enum Op
{
    FromString = 0,
    ToString = 1,
    Normalize = 2,
    Compare = 3,
    Expand = 4,
    ExpandTo = 5,
    Successor = 6,
}

/// <summary>Bitset of supported operations.</summary>
public class Capabilities
{
    private byte _bits;

    public void Set(Op op) => _bits |= (byte)(1 << (int)op);
    public bool Has(Op op) => (_bits & (byte)(1 << (int)op)) != 0;
}