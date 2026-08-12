using GoogologyLib.Core;
using GoogologyLib.Notations.Ordinal.Sequence.MarkedParent;
using Xunit;

namespace GoogologyLib.Tests;

public class PpsTests
{
    [Fact]
    public void TestParseAndString()
    {
        var p = new Pps.Pps1("(0, 1, 0, 3)");
        Assert.Equal("(0, 1, 0, 3)", p.ToLatex());
    }

    [Fact]
    public void TestPps1Expand()
    {
        var p = new Pps.Pps1("(0,1,2)");
        p.Expand(0);
        Assert.Equal("(0, 1, 1)", p.ToLatex());
        p = new Pps.Pps1("(0,1,2)");
        p.Expand(2);
        Assert.Equal("(0, 1, 1, 1, 1)", p.ToLatex());
    }

    [Fact]
    public void TestPps1NoTermBetween()
    {
        var p = new Pps.Pps1("(0,1,0,3)");
        p.Expand(0);
        Assert.Equal("(0, 1, 0, 2)", p.ToLatex());
        p = new Pps.Pps1("(0,1,0,3)");
        p.Expand(1);
        Assert.Equal("(0, 1, 0, 2, 2)", p.ToLatex());
    }

    [Fact]
    public void TestPps1TermEqualToB()
    {
        var p = new Pps.Pps1("(0,1,0,1,2)");
        p.Expand(0);
        Assert.Equal("(0, 1, 0, 1, 1, 0, 1)", p.ToLatex());
    }

    [Fact]
    public void TestSuccessor()
    {
        var p = new Pps.Pps1("(0,1,0)");
        p.Expand(3);
        Assert.Equal("(0, 1)", p.ToLatex());
        Assert.True(new Pps.Pps1("(0,1,0)").IsSuccessor);
        Assert.False(new Pps.Pps1("(0,1,2)").IsSuccessor);
    }

    [Fact]
    public void TestExpandTo()
    {
        var p = new Pps.Pps1("(0,1,2)");
        p.ExpandTo(2);
        Assert.Equal("(0, 1, 1, 1, 1)", p.ToLatex());
    }

    [Fact]
    public void TestPps1VsPps2()
    {
        var p1 = new Pps.Pps1("(0,2,4,4,2,3)");
        p1.Expand(0);
        Assert.Equal("(0, 2, 4, 4, 2, 4, 7, 2)", p1.ToLatex());
        var p2 = new Pps.Pps2("(0,2,4,4,2,3)");
        p2.Expand(0);
        Assert.Equal("(0, 2, 4, 4, 2, 2, 7, 2)", p2.ToLatex());
    }

    [Fact]
    public void TestPps4WeakStrong()
    {
        Pps p = new Pps.Pps4("(0,1,0,2,2,3)");
        p.Expand(0);
        Assert.Equal("(0, 1, 0, 2, 2, 1, 2, 2)", p.ToLatex());
        p = new Pps.Pps1("(0,1,0,2,2,3)");
        p.Expand(0);
        Assert.Equal("(0, 1, 0, 2, 2, 2, 2, 2)", p.ToLatex());
        p = new Pps.Pps4("(0,1,2,1,3)");
        p.Expand(0);
        Assert.Equal("(0, 1, 2, 1, 2, 1)", p.ToLatex());
    }

    [Fact]
    public void TestWpps4SameAsPps4()
    {
        var p = new Pps.WPps4("(0,1,0,2,2,3)");
        p.Expand(0);
        Assert.Equal("(0, 1, 0, 2, 2, 1, 2, 2)", p.ToLatex());
    }

    [Fact]
    public void TestTpps4Fpps4()
    {
        Pps p = new Pps.TPps4("(0,1,0,2,2,3)");
        p.Expand(1);
        Assert.Equal("(0, 1, 0, 2, 2, 1, 2, 2, 4, 2, 2)", p.ToLatex());
        p = new Pps.FPps4("(0,1,0,2,2,3)");
        p.Expand(1);
        Assert.Equal("(0, 1, 0, 2, 2, 1, 2, 2, 3, 2, 2)", p.ToLatex());
        p = new Pps.Pps4("(0,1,0,2,2,3)");
        p.Expand(1);
        Assert.Equal("(0, 1, 0, 2, 2, 1, 2, 2, 1, 2, 2)", p.ToLatex());
    }

    [Fact]
    public void TestLimitAndMasterLimit()
    {
        var l = new Pps.Pps1().Limit(3);
        Assert.Equal("(0, 1, 2)", l.ToLatex());
        var m = new Pps.Pps1().MasterLimit();
        m.Expand(3);
        Assert.Equal("(0, 1, 2)", m.ToLatex());
    }

    [Fact]
    public void TestCompare()
    {
        Assert.Equal(-1, new Pps.Pps1("(0,1,2)").Compare(new Pps.Pps1("(0,1,3)")));
        Assert.Equal(1, new Pps.Pps1("(0,1,3)").Compare(new Pps.Pps1("(0,1,2)")));
        Assert.Equal(0, new Pps.Pps1("(0,1,2)").Compare(new Pps.Pps1("(0,1,2)")));
        Assert.Equal(-1, new Pps.Pps1("(0,1,2)").Compare(new Pps.Pps1("(0,1,2,0)")));
    }

    [Fact]
    public void TestCrossVariantCompareThrows()
    {
        Assert.Throws<NotComparable>(() =>
            new Pps.Pps1("(0,1,2)").Compare(new Pps.Pps2("(0,1,2)")));
    }

    [Fact]
    public void TestTaxonomyAndCapabilities()
    {
        var p = new Pps.Pps1("(0,1,2)");
        Assert.Equal("pps1", p.Name);
        Assert.Equal("marked_parent", p.Style);
        Assert.Equal("318`4", p.Creator);
        Assert.Equal("1", p.Version);
        Assert.Equal("tpps4", new Pps.TPps4().Name);
        Assert.Equal("4t", new Pps.TPps4().Version);
        Assert.True(p.Can(Op.Compare));
        Assert.True(p.Can(Op.Expand));
    }
}
