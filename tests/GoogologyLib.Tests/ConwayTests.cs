using GoogologyLib.Core;
using GoogologyLib.Notations.Number;
using Xunit;

namespace GoogologyLib.Tests;

public class ConwayTests
{
    [Fact]
    public void TestParseAndString()
    {
        var c = new Conway("3 -> 3 -> 2");
        var s = c.ToLatex();
        Assert.Contains("rightarrow", s);
    }

    [Fact]
    public void TestExpandOnce()
    {
        var c = new Conway("3 -> 3 -> 2");
        c.Expand(1);
        var s = c.ToLatex();
        Assert.Contains("rightarrow", s);
    }

    [Fact]
    public void TestTrailingOne()
    {
        var c = new Conway("3 -> 4 -> 1");
        c.Expand(1);
        var s = c.ToLatex();
        Assert.Contains("4", s);
    }

    [Fact]
    public void TestCapabilities()
    {
        var c = new Conway("3 -> 3 -> 2");
        Assert.True(c.Can(Op.FromString));
        Assert.True(c.Can(Op.ToString));
        Assert.True(c.Can(Op.Expand));
        Assert.True(c.Can(Op.ExpandTo));
        Assert.False(c.Can(Op.Compare));
    }
}