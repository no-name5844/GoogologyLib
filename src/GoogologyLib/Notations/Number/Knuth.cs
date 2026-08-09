using System.Text.RegularExpressions;
using GoogologyLib.Core;

namespace GoogologyLib.Notations.Number;

/// <summary>Knuth up-arrow notation (高德纳箭头).</summary>
public class Knuth : Notation
{
    private KNode _root = KNode.Value(0);

    public Knuth() { }
    public Knuth(string s) => StringToIt(s);

    public override string Name => "knuth";
    public override Family Family => Core.Family.Number;
    public override string Creator => "Donald Knuth";
    public override string Version => "1";

    public override Capabilities Capabilities
    {
        get
        {
            var c = new Capabilities();
            c.Set(Op.FromString);
            c.Set(Op.ToString);
            c.Set(Op.Expand);
            c.Set(Op.ExpandTo);
            return c;
        }
    }

    public override void StringToIt(string s)
    {
        var t = s.Replace("↑", "^").Replace(" ", "");
        if (string.IsNullOrEmpty(t))
            throw new ArgumentException("Knuth: empty input");
        _root = ParseExpr(t);
    }

    public override string ToLatex() => _root.Tex();

    public override Notation Expand(int n = 1)
    {
        for (int i = 0; i < n; i++)
            _root = _root.Step();
        return this;
    }

    public override Notation ExpandTo(int length)
    {
        var prev = ToLatex();
        while (true)
        {
            Expand(1);
            var cur = ToLatex();
            if (cur.Length >= length || cur == prev) return this;
            prev = cur;
        }
    }

    // --- internal AST ---

    private class KNode
    {
        public bool IsVal;
        public long Val;
        public long BaseVal;
        public long Height;
        public KNode? Exp;

        public static KNode Value(long v) => new() { IsVal = true, Val = v };
        public static KNode Arrow(long baseVal, long height, KNode exp) =>
            new() { IsVal = false, BaseVal = baseVal, Height = height, Exp = exp };

        public string Tex()
        {
            if (IsVal) return Val.ToString();
            var bt = BaseVal.ToString();
            var et = Exp?.Tex() ?? "";
            var arr = Height == 1 ? "\\uparrow" : $"\\uparrow^{{{Height}}}";
            var expHasArrow = Exp is { IsVal: false };
            var expPart = expHasArrow ? $"({et})" : et;
            return $"{bt} {arr} {expPart}";
        }

        public KNode Step()
        {
            if (IsVal) return this;
            if (Exp is { IsVal: true })
            {
                var b = Exp.Val;
                if (b == 1) return Value(BaseVal);  // a ^c 1 = a
                if (Height == 1) return this;  // a^b: terminal
                var inner = Arrow(BaseVal, Height, Value(b - 1));
                return Arrow(BaseVal, Height - 1, inner);
            }
            // recurse into the rightmost sub-term
            return Arrow(BaseVal, Height, Exp!.Step());
        }
    }

    private static KNode ParseExpr(string s)
    {
        var pos = s.IndexOf('^');
        if (pos < 0)
        {
            if (string.IsNullOrEmpty(s)) throw new ArgumentException("Knuth: empty operand");
            return KNode.Value(long.Parse(s));
        }
        var a = long.Parse(s[..pos]);
        var j = pos;
        var c = 0;
        while (j < s.Length && s[j] == '^') { c++; j++; }
        var rest = s[j..];
        if (string.IsNullOrEmpty(rest))
            throw new ArgumentException("Knuth: dangling arrows (no exponent)");
        var exp = ParseExpr(rest);
        return KNode.Arrow(a, c, exp);
    }
}