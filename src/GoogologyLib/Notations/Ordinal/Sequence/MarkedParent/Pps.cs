using GoogologyLib.Core;

namespace GoogologyLib.Notations.Ordinal.Sequence.MarkedParent;

/// <summary>
/// PPS (Parented Predecessor Sequence) — marked-parent ordinal sequence
/// notation. Created by 318`4. Variants: PPS1 (original) / PPS2 / PPS4 /
/// wPPS4 / tPPS4 / fPPS4. PPS3 / ePPS4 / sPPS4 are N/A (no data) — not
/// implemented.
///
/// Bad root = the x-th term (x = last value); b = bad-root value; L = y - x
/// (y = last column). ExpandLen replaces the last term (per variant) and
/// appends elements from the running sequence: a_i &gt;= x -> a_i + L, else a_i.
/// 基本列[n] = expand to the (y + nL - 1)-th term (n &gt;= 1). Library 0-index:
/// Expand(k) = 基本列[k+1] = ExpandLen((k+1)L - 1). Compare is lexicographic;
/// cross-variant / cross-family throws NotComparable.
/// </summary>
public abstract class Pps : Notation
{
    /// <summary>PPS variants.</summary>
    public enum Variant
    {
        Pps1,
        Pps2,
        Pps4,
        WPps4,
        TPps4,
        FPps4,
    }

    private readonly Variant _variant;
    protected List<long> Seq { get; } = new();
    protected bool IsMasterLimit { get; set; }

    protected Pps(Variant v) => _variant = v;
    protected Pps(Variant v, string s)
    {
        _variant = v;
        StringToIt(s);
    }

    private static string RegName(Variant v) => v switch
    {
        Variant.Pps1 => "pps1",
        Variant.Pps2 => "pps2",
        Variant.Pps4 => "pps4",
        Variant.WPps4 => "wpps4",
        Variant.TPps4 => "tpps4",
        Variant.FPps4 => "fpps4",
        _ => "pps",
    };

    private static string Ver(Variant v) => v switch
    {
        Variant.Pps1 => "1",
        Variant.Pps2 => "2",
        Variant.Pps4 => "4",
        Variant.WPps4 => "4w",
        Variant.TPps4 => "4t",
        Variant.FPps4 => "4f",
        _ => "?",
    };

    public override string Name => RegName(_variant);
    public override Family Family => Core.Family.Ordinal;
    public override string Subfamily => "sequence";
    public override string Style => "marked_parent";
    public override string Creator => "318`4";
    public override string Version => Ver(_variant);

    public override Capabilities Capabilities
    {
        get
        {
            var c = new Capabilities();
            c.Set(Op.FromString);
            c.Set(Op.ToString);
            c.Set(Op.Normalize);
            c.Set(Op.Compare);
            c.Set(Op.Expand);
            c.Set(Op.ExpandTo);
            c.Set(Op.Successor);
            return c;
        }
    }

    // --- internal helpers ---

    /// <summary>Rightmost 1-based column j in the open interval (lo, hi) with value = b.</summary>
    private long RightmostEq(long lo, long hi, long b)
    {
        for (long j = hi - 1; j > lo; j--)
        {
            if (j < 1 || j > Seq.Count) continue;
            if (Seq[(int)(j - 1)] == b) return j;
        }
        return 0;
    }

    /// <summary>Lexicographic compare of a_{k+j} vs a_{c+j} (j = 0,1,2,...).</summary>
    private int TailCmp(long k, long c)
    {
        long y = Seq.Count;
        for (long j = 0; ; j++)
        {
            long ik = k + j, ic = c + j;
            bool okK = ik >= 1 && ik <= y;
            bool okC = ic >= 1 && ic <= y;
            if (!okK && !okC) return 0;
            if (!okK) return -1;
            if (!okC) return 1;
            long vk = Seq[(int)(ik - 1)], vc = Seq[(int)(ic - 1)];
            if (vk != vc) return vk > vc ? 1 : -1;
        }
    }

    /// <summary>Per-variant last-term replacement: (newLast, strong).</summary>
    private (long NewLast, bool Strong) ReplaceLast(long x, long b, long y)
    {
        switch (_variant)
        {
            case Variant.Pps1:
                {
                    long k = RightmostEq(x, y, b);
                    return (k != 0 ? b : x - 1, false);
                }
            case Variant.Pps2:
                {
                    long k = RightmostEq(x, y, b);
                    return (k != 0 && TailCmp(k, x) > 0 ? b : x - 1, false);
                }
            default:
                {
                    long k = RightmostEq(x, y, b);
                    if (k != 0) return (b, false);      // weak expansion
                    long j = RightmostEq(b, x, b);      // strong expansion
                    return (j != 0 ? j : b, true);      // not found -> as weak
                }
        }
    }

    /// <summary>Copy rule; tPPS4/fPPS4 strong-branch modifiers on the last-term chain.</summary>
    private long Copied(long src, long x, long L, long y, long i, bool strong)
    {
        long baseVal = src >= x ? src + L : src;
        if (!strong) return baseVal;
        if (_variant is Variant.TPps4 or Variant.FPps4)
        {
            if (i % L == y % L)
            {
                if (_variant == Variant.FPps4 && i == y) return x;
                return src + L;
            }
        }
        return baseVal;
    }

    /// <summary>ExpandLen(A, M): replace last + append M elements (source i = q + y - L).</summary>
    private void ExpandLen(long M)
    {
        if (Seq.Count == 0) return;
        long y = Seq.Count;
        long x = Seq[^1];
        if (x == 0) { Seq.RemoveAt(Seq.Count - 1); return; }   // successor
        if (x >= y) return;                                    // bad root OOR (C++ throws)
        long b = Seq[(int)(x - 1)];
        long L = y - x;
        var (newLast, strong) = ReplaceLast(x, b, y);
        Seq[(int)(y - 1)] = newLast;
        for (long q = 1; q <= M; q++)
        {
            long i = q + y - L;
            if (i < 1 || i > Seq.Count) return;                // defensive
            long src = Seq[(int)(i - 1)];
            Seq.Add(Copied(src, x, L, y, i, strong));
        }
    }

    public override void StringToIt(string s)
    {
        Seq.Clear();
        IsMasterLimit = false;
        var t = string.Concat(s.Where(ch => !char.IsWhiteSpace(ch)));
        while (t.Length > 0 && "([{".IndexOf(t[0]) >= 0) t = t[1..];
        while (t.Length > 0 && ")]}".IndexOf(t[^1]) >= 0) t = t[..^1];
        if (t.Length == 0) return;
        foreach (var p in t.Split(','))
            if (p.Length > 0) Seq.Add(long.Parse(p));
    }

    public override string ToLatex()
    {
        if (IsMasterLimit) return "(0, 1, 2, …)";
        return "(" + string.Join(", ", Seq) + ")";
    }

    /// <summary>Library 0-index: Expand(k) = 基本列[k+1] = ExpandLen((k+1)L - 1).</summary>
    public override Notation Expand(int n = 1)
    {
        if (IsMasterLimit)
        {
            Seq.Clear();
            for (long i = 0; i < n; i++) Seq.Add(i);
            IsMasterLimit = false;
            return this;
        }
        if (Seq.Count == 0) return this;
        long x = Seq[^1];
        if (x == 0) { Seq.RemoveAt(Seq.Count - 1); return this; }
        long y = Seq.Count;
        if (x >= y) return this;   // bad root OOR (C++ throws)
        long L = y - x;
        ExpandLen((n + 1) * L - 1);
        return this;
    }

    public override Notation ExpandTo(int length)
    {
        ExpandLen(length);
        return this;
    }

    public override int Compare(Notation other)
    {
        if (other is not Pps o) throw new NotComparable(Name);
        if (_variant != o._variant) throw new NotComparable(Name);
        if (IsMasterLimit || o.IsMasterLimit)
        {
            if (IsMasterLimit && o.IsMasterLimit) return 0;
            return IsMasterLimit ? 1 : -1;
        }
        int m = Math.Min(Seq.Count, o.Seq.Count);
        for (int i = 0; i < m; i++)
        {
            long a = Seq[i], b = o.Seq[i];
            if (a > b) return 1;
            if (a < b) return -1;
        }
        return Seq.Count.CompareTo(o.Seq.Count);
    }

    /// <summary>Sequence ends with 0 => successor.</summary>
    public bool IsSuccessor => !IsMasterLimit && Seq.Count > 0 && Seq[^1] == 0;

    /// <summary>limit(n) = (0,1,...,n-1) of this variant.</summary>
    public Pps Limit(int n)
    {
        var p = NewInstance();
        for (long i = 0; i < n; i++) p.Seq.Add(i);
        return p;
    }

    /// <summary>Master limit expression of this variant (supremum).</summary>
    public Pps MasterLimit()
    {
        var p = NewInstance();
        p.IsMasterLimit = true;
        return p;
    }

    private Pps NewInstance() => (Pps)Activator.CreateInstance(GetType())!;

    // --- concrete variants (same module, per project convention) ---

    public class Pps1 : Pps
    {
        public Pps1() : base(Variant.Pps1) { }
        public Pps1(string s) : base(Variant.Pps1, s) { }
    }

    public class Pps2 : Pps
    {
        public Pps2() : base(Variant.Pps2) { }
        public Pps2(string s) : base(Variant.Pps2, s) { }
    }

    public class Pps4 : Pps
    {
        public Pps4() : base(Variant.Pps4) { }
        public Pps4(string s) : base(Variant.Pps4, s) { }
    }

    public class WPps4 : Pps
    {
        public WPps4() : base(Variant.WPps4) { }
        public WPps4(string s) : base(Variant.WPps4, s) { }
    }

    public class TPps4 : Pps
    {
        public TPps4() : base(Variant.TPps4) { }
        public TPps4(string s) : base(Variant.TPps4, s) { }
    }

    public class FPps4 : Pps
    {
        public FPps4() : base(Variant.FPps4) { }
        public FPps4(string s) : base(Variant.FPps4, s) { }
    }
}
