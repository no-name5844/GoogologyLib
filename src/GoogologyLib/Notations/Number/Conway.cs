using GoogologyLib.Core;

namespace GoogologyLib.Notations.Number;

/// <summary>Conway chained-arrow notation (康威链式箭头).</summary>
public class Conway : Notation
{
    private List<CNode> _chain = [];

    public Conway() { }
    public Conway(string s) => StringToIt(s);

    public override string Name => "conway";
    public override Family Family => Core.Family.Number;
    public override string Creator => "John Conway";
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
        var t = s.Replace("→", "->").Replace(" ", "");
        _chain = t.Split("->")
            .Select(part => CNode.Value(long.Parse(part)))
            .ToList();
    }

    public override string ToLatex() => Ser(_chain);

    public override Notation Expand(int n = 1)
    {
        for (int i = 0; i < n; i++)
            _chain = StepOnce(_chain);
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

    // --- internal ---

    private class CNode
    {
        public bool IsInt;
        public long Val;
        public List<CNode>? Sub;

        public static CNode Value(long v) => new() { IsInt = true, Val = v };
        public static CNode SubChain(List<CNode> s) => new() { IsInt = false, Sub = s };
    }

    private static string Ser(List<CNode> ch)
    {
        if (ch.Count == 2 && ch[0].IsInt && ch[1].IsInt)
            return $"{ch[0].Val}^{{{ch[1].Val}}}";
        var parts = ch.Select(node =>
            node.IsInt ? node.Val.ToString() : $"({Ser(node.Sub!)})");
        return string.Join(" \\rightarrow ", parts);
    }

    private static List<CNode> StepOnce(List<CNode> ch)
    {
        var n = ch.Count;
        if (n <= 2)
        {
            if (n <= 1) return ch;
            if (ch[0].IsInt && ch[1].IsInt) return ch;  // terminal: a -> b
            return RecurseSub(ch);
        }

        // rule 2a: trailing 1  (X -> a -> 1 = X -> a)
        if (ch[n - 1].IsInt && ch[n - 1].Val == 1)
            return ch.Take(n - 1).ToList();

        // rule 2b: middle 1  (X -> 1 -> Y = X)
        if (ch[n - 2].IsInt && ch[n - 2].Val == 1)
            return ch.Take(n - 2).ToList();

        // rule 3: X -> a -> b = X -> (X -> a-1 -> b) -> b-1
        if (ch[n - 1].IsInt && ch[n - 2].IsInt)
        {
            var a = ch[n - 2].Val;
            var b = ch[n - 1].Val;
            var x = ch.Take(n - 2).ToList();
            var inner = x.Concat([CNode.Value(a - 1), CNode.Value(b)]).ToList();
            return x.Concat([CNode.SubChain(inner), CNode.Value(b - 1)]).ToList();
        }

        return RecurseSub(ch);
    }

    private static List<CNode> RecurseSub(List<CNode> ch)
    {
        var changed = false;
        var r = ch.Select(node =>
        {
            if (!node.IsInt)
            {
                var before = Ser(node.Sub!);
                var s = StepOnce(node.Sub!);
                var after = Ser(s);
                if (after != before)
                {
                    changed = true;
                    if (s.Count == 1 && s[0].IsInt)
                        return CNode.Value(s[0].Val);
                    return CNode.SubChain(s);
                }
            }
            return node;
        }).ToList();
        return changed ? r : ch;
    }
}