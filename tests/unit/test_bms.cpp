// BMS 模块单测（BM4 精确断言 + BM1/BM3.3 冒烟）。
// 注意：本库 expand(n) = 第 n 项基本列 FS_n(S)（一步展开，与 Prss/EpspSS 一致）。
// 笔记 §0.2.2 的 "expand(S,n)=expand(FS_n,2n)" 是递归迭代展示，故不与其长串逐字符比对。
#include <cassert>
#include <iostream>
#include <string>
#include "googology/notations/ordinal/matrix/bms/BMS.hpp"
#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"

using namespace googology;
using namespace googology::ordinal;

static int g_fail = 0;
#define CHECK(cond) do { if (!(cond)) { std::cout << "FAIL: " #cond " @ " __FILE__ ":" << __LINE__ << "\n"; ++g_fail; } } while (0)

static void test_parse_and_standard() {
    BM4 a("(0,0,0)(1,1,1)(2,2,0)");
    // to_string 输出简写（末尾尾随零忽略），(0,0,0)->(0)
    CHECK(a.to_string() == "(0)(1,1,1)(2,2)");
    CHECK(a.family() == Family::Ordinal);
    CHECK(a.subfamily() == "bms");
    CHECK(a.style() == "matrix");
    CHECK(a.name() == "bm4");
    CHECK(a.capabilities().has(Op::Expand));
    CHECK(a.capabilities().has(Op::Normalize));
    CHECK(a.capabilities().has(Op::Compare));
    CHECK(a.isSuccessor() == false);

    // 标准型检测走通用 §12 引擎（is_standard 是 OrdinalNotation 的
    // NON-VIRTUAL 通用实现，与 Prss / ε_pSS / … 完全一致，无任何区别）：
    // 从 master limit (0)(1,1,1,…) 出发，经有限次 expand + 取基本列前缀
    // 可达者为标准型。
    //   () / (0) / (0)(0) / (0)(1)=ω / limit(n) 等均可达 -> 标准型。
    CHECK(BM4("").is_standard() == true);           // () = 0
    CHECK(BM4("(0)").is_standard() == true);        // 1（后继）
    CHECK(BM4("(0)(0)").is_standard() == true);     // 2
    CHECK(BM4("(0)(1)").is_standard() == true);     // ω = limit(2)
    CHECK(BMS::master_limit()->is_standard() == true);

    // 合法且标准型：(0,0,0)(1,1,1)(2,2,0) 满足 §0.1 合法性，且其值
    // (0)(1,1,1)(2,2) 夹在 limit(4) 与 limit(5) 之间，从该记号系统的
    // master limit 经有限次 expand 可达 -> 标准型。
    // compare 按值（尾随零不改大小）判定可达性。
    CHECK(a.is_standard() == true);

    // 非法（首列非全 0）：更谈不上标准型。
    BM4 bad("(1,0,0)(0,0,0)");
    CHECK(bad.is_standard() == false);
    // normalize 对非法矩阵保持原值不动（库惯例）
    BM4 bad2("(1,0,0)");
    bad2.normalize();
    // 输出简写（尾随零忽略）：(1,0,0) -> (1)
    CHECK(bad2.to_string() == "(1)");
}

static void test_single_column() {
    BM4 a("(5)");
    a.expand(1);
    CHECK(a.to_string() == "(4)");
    BM4 b("(5)");
    b.expand(3);
    CHECK(b.to_string() == "(2)");
    BM4 c("(1)");
    c.expand(1);
    CHECK(c.to_string() == "" || c.to_string() == "(0)");
}

static void test_bm4_fs() {
    // back-gear 矩阵 (0,0,0)(1,1,1)(2,2,0)
    // 索引整体偏移 1（spec n=1,2 ↔ lib n=0,1）：
    //   lib expand(k) = spec FS_{k+1} = G + B^(0)+...+B^(k)（k+1 个坏块）。
    //   末列 LNZ z=1，坏根 r=p_1(2)=1；G=列0=(0,0,0)，B=列1=(1,1,1)；
    //   Δ_0 = S_{2,0}-S_{1,0} = 1，Δ_1=0 (k>=z)；故 B^(i)=(1+i,1,1)。
    //   k=0：G+B^(0)        = (0,0,0)(1,1,1)        -> (0)(1,1,1)
    //   k=1：G+B^(0)+B^(1)  = (0,0,0)(1,1,1)(2,1,1) -> (0)(1,1,1)(2,1,1)
    //   k=2：G+...+B^(2)     = ...(3,1,1)             -> (0)(1,1,1)(2,1,1)(3,1,1)
    //   to_string 输出简写（尾随零忽略）：(0,0,0)->(0)，故下面用简写。
    BM4 a("(0,0,0)(1,1,1)(2,2,0)");
    a.expand(0);
    CHECK(a.to_string() == "(0)(1,1,1)");
    BM4 b("(0,0,0)(1,1,1)(2,2,0)");
    b.expand(1);
    CHECK(b.to_string() == "(0)(1,1,1)(2,1,1)");
    BM4 c("(0,0,0)(1,1,1)(2,2,0)");
    c.expand(2);
    CHECK(c.to_string() == "(0)(1,1,1)(2,1,1)(3,1,1)");
}

static void test_bm33_note_example() {
    // 笔记 §9.2.2 给出的 BM3.3 具体示例：
    //   (0,0,0)(1,1,1)(2,1,0)(1,1,1) 展开首项 = ...(1,1,0)
    //   即 FS_1 应为 (0,0,0)(1,1,1)(2,1,0)(1,1,0)。
    BM3_3 a("(0,0,0)(1,1,1)(2,1,0)(1,1,1)");
    a.expand(1);
    // 满形 FS_1 = (0,0,0)(1,1,1)(2,1,0)(1,1,0)；to_string 简写
    // （尾随零忽略）-> (0,0,0)->(0)，(2,1,0)->(2,1)，(1,1,0)->(1,1)。
    CHECK(a.to_string() == "(0)(1,1,1)(2,1)(1,1)");
}

static void test_variants_smoke() {
    // BM1 / BM3.3 冒烟：能解析、expand(1) 不抛、结果非空且确定性。
    BM1 m1("(0,0,0)(1,1,1)(2,2,0)");
    m1.expand(1);
    std::string s1 = m1.to_string();
    CHECK(!s1.empty());
    BM1 m1b("(0,0,0)(1,1,1)(2,2,0)");
    m1b.expand(1);
    CHECK(m1b.to_string() == s1);              // 确定性

    BM3_3 m3("(0,0,0)(1,1,1)(2,1,0)(1,1,1)");
    m3.expand(1);
    std::string s3 = m3.to_string();
    CHECK(!s3.empty());
    CHECK(m3.name() == "bm3.3");
    BM3_3 m3b("(0,0,0)(1,1,1)(2,1,0)(1,1,1)");
    m3b.expand(1);
    CHECK(m3b.to_string() == s3);

    // 跨类型比较应抛 NotComparable
    bool threw = false;
    try { BM4 x("(1)"); Prss p("(0,1)"); x.compare(p); }
    catch (const NotComparable&) { threw = true; }
    CHECK(threw);
}

static void test_compare() {
    // 两层字典序：列主序（col 0 最优先）；两列之间按各元素字典序、
    // 行下标从小到大（y=0 顶行最先比）。纯句法序，非真序数序。
    CHECK(BM4("(0,0,0)(1,1,1)(2,2,0)").compare(
          BM4("(0,0,0)(1,1,1)(2,2,0)")) == 0);            // 相等
    CHECK(BM4("(1,0)").compare(BM4("(0,1)")) == 1);        // col 0 决定 A>B
    CHECK(BM4("(0,0)(1,0)").compare(BM4("(0,0)(0,1)")) == 1); // col1 y0 不同
    CHECK(BM4("(0,0)(0,1)").compare(BM4("(0,0)(0,0)")) == 1); // col1 y1 不同
    CHECK(BM4("(0,2)").compare(BM4("(0,1)")) == 1);        // 多行 y1 不同
    // 行下标从小到大：A=(1,0) B=(0,9) -> y0:1>0 立即 A>B，不看 y1
    CHECK(BM4("(1,0)").compare(BM4("(0,9)")) == 1);
    // 前缀：短者为小（标准字典序，等长前缀后长者更大）
    CHECK(BM4("(0)").compare(BM4("(0)(0)")) == -1);
}

static void test_limit() {
    // §0.3 极限表达式（所有 BMS 版本共用）：
    //   (),(0),(0)(1),(0)(1,1),(0)(1,1,1),...
    CHECK(BMS::limit(0)->to_string() == "");      // 空矩阵 -> 序列化为空串（与 BM4("") 一致）
    CHECK(BMS::limit(1)->to_string() == "(0)");
    CHECK(BMS::limit(2)->to_string() == "(0)(1)");
    CHECK(BMS::limit(3)->to_string() == "(0)(1,1)");
    CHECK(BMS::limit(4)->to_string() == "(0)(1,1,1)");

    // master limit：to_string 渲染为 (0)(1,1,1,…)
    auto L = BMS::master_limit();
    CHECK(L->to_string() == "(0)(1,1,1,…)");
    // master_limit().expand(m) == limit(m)
    L->expand(3);
    CHECK(L->to_string() == "(0)(1,1)");
    // 子类继承：BM4::limit 同样可用
    CHECK(BM4::limit(3)->to_string() == "(0)(1,1)");
}

int main() {
    test_parse_and_standard();
    test_single_column();
    test_bm4_fs();
    test_bm33_note_example();
    test_variants_smoke();
    test_compare();
    test_limit();
    if (g_fail == 0)
        std::cout << "test_bms: all assertions passed\n";
    else
        std::cout << "test_bms: " << g_fail << " FAILED\n";
    return g_fail ? 1 : 0;
}
