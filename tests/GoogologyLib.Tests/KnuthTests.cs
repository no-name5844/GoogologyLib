using GoogologyLib.Core;
using GoogologyLib.Notations.Number;
using Xunit;

namespace GoogologyLib.Tests;

public class KnuthTests
{
    [Fact]
    public void TestParseAndString()
    {
        var k = new Knuth("2 ^^ 3");
        var s = k.ToLatex();
        Assert.Contains("uparrow", s);
    }

    [Fact]
    public void TestExpandOnce()
    {
        var k = new Knuth("2 ^^ 3");
        k.Expand(1);
        var s = k.ToLatex();
        Assert.Contains("uparrow", s);
    }

    [Fact]
    public void TestNotComparable()
    {
        var k = new Knuth("2 ^^ 3");
        Assert.Throws<NotComparable>(() => k.Compare(new Knuth("3 ^^ 3")));
    }

    [Fact]
    public void TestCapabilities()
    {
        var k = new Knuth("2 ^^ 3");
        Assert.True(k.Can(Op.FromString));
        Assert.True(k.Can(Op.ToString));
        Assert.True(k.Can(Op.Expand));
        Assert.True(k.Can(Op.ExpandTo));
        Assert.False(k.Can(Op.Compare));
    }
}