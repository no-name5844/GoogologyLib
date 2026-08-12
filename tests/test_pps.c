// PPS 系列 C 分支单测（与 C++ tests/unit/test_pps.cpp 断言一致）。
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "googology/notations/ordinal/sequence/marked_parent/pps/pps.h"

static void expect_latex(goog_notation_t* n, const char* expected) {
    char* s = goog_to_latex(n);
    assert(strcmp(s, expected) == 0);
    free(s);
}

int main(void) {
    // ---- parse / to_string ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0, 1, 0, 3)");
        expect_latex(p, "(0, 1, 0, 3)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "0,1,0,3");
        expect_latex(p, "(0, 1, 0, 3)");
        goog_destroy(p);
    }

    // ---- PPS1 展开：坏根 = 第 x 项 ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 1, 1)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2)");
        goog_expand(p, 1);
        expect_latex(p, "(0, 1, 1, 1)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2)");
        goog_expand(p, 2);
        expect_latex(p, "(0, 1, 1, 1, 1)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,0,3)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 1, 0, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,0,3)");
        goog_expand(p, 1);
        expect_latex(p, "(0, 1, 0, 2, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,0,1,2)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 1, 0, 1, 1, 0, 1)");
        goog_destroy(p);
    }

    // ---- 后继：末项为 0 → 去掉末项 ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,0)");
        goog_expand(p, 3);
        expect_latex(p, "(0, 1)");
        goog_destroy(p);
    }

    // ---- expand_to = expandLen ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2)");
        goog_expand_to(p, 2);
        expect_latex(p, "(0, 1, 1, 1, 1)");
        goog_destroy(p);
    }

    // ---- 坏根越界：保持原样（C++ throws，此处 panic-free）----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,5)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 5)");
        goog_destroy(p);
    }

    // ---- PPS1 vs PPS2 ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,2,4,4,2,3)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 2, 4, 4, 2, 4, 7, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS2, "(0,2,4,4,2,3)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 2, 4, 4, 2, 2, 7, 2)");
        goog_destroy(p);
    }

    // ---- PPS4 弱/强展开 ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS4, "(0,1,0,2,2,3)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 1, 0, 2, 2, 1, 2, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,0,2,2,3)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 1, 0, 2, 2, 2, 2, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS4, "(0,1,2,1,3)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 1, 2, 1, 2, 1)");
        goog_destroy(p);
    }

    // ---- wPPS4（与 PPS4 一致）----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_WPPS4, "(0,1,0,2,2,3)");
        goog_expand(p, 0);
        expect_latex(p, "(0, 1, 0, 2, 2, 1, 2, 2)");
        goog_destroy(p);
    }

    // ---- tPPS4 / fPPS4 强展开复制修饰 ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_TPPS4, "(0,1,0,2,2,3)");
        goog_expand(p, 1);
        expect_latex(p, "(0, 1, 0, 2, 2, 1, 2, 2, 4, 2, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS4, "(0,1,0,2,2,3)");
        goog_expand(p, 1);
        expect_latex(p, "(0, 1, 0, 2, 2, 1, 2, 2, 1, 2, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_FPPS4, "(0,1,0,2,2,3)");
        goog_expand(p, 1);
        expect_latex(p, "(0, 1, 0, 2, 2, 1, 2, 2, 3, 2, 2)");
        goog_destroy(p);
    }

    // ---- §12 limit API ----
    {
        goog_notation_t* p = goog_pps_limit(PPS_VARIANT_PPS1, 3);
        expect_latex(p, "(0, 1, 2)");
        goog_destroy(p);
    }
    {
        goog_notation_t* p = goog_pps_master_limit(PPS_VARIANT_PPS1);
        goog_expand(p, 3);
        expect_latex(p, "(0, 1, 2)");
        goog_destroy(p);
    }

    // ---- compare：字典序，跨变体 NotComparable ----
    {
        goog_notation_t* a = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2)");
        goog_notation_t* b = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,3)");
        assert(a->vtable->compare(a, b) == -1);
        assert(b->vtable->compare(b, a) == 1);
        assert(a->vtable->compare(a, a) == 0);
        goog_notation_t* c = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2,0)");
        assert(a->vtable->compare(a, c) == -1);
        goog_destroy(a); goog_destroy(b); goog_destroy(c);
    }
    {
        goog_notation_t* a = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2)");
        goog_notation_t* b = goog_pps_parse(PPS_VARIANT_PPS2, "(0,1,2)");
        assert(a->vtable->compare(a, b) == GOOG_ERR_NOT_COMPARABLE);
        goog_destroy(a); goog_destroy(b);
    }

    // ---- taxonomy ----
    {
        goog_notation_t* p = goog_pps_parse(PPS_VARIANT_PPS1, "(0,1,2)");
        assert(strcmp(goog_name(p), "pps1") == 0);
        assert(goog_family(p) == FAMILY_ORDINAL);
        assert(strcmp(p->vtable->creator(p), "318`4") == 0);
        assert(strcmp(p->vtable->version(p), "1") == 0);
        assert(goog_can(p, OP_COMPARE));
        assert(goog_can(p, OP_EXPAND));
        goog_destroy(p);
        goog_notation_t* t = goog_pps_parse(PPS_VARIANT_TPPS4, "(0,1,2)");
        assert(strcmp(goog_name(t), "tpps4") == 0);
        assert(strcmp(t->vtable->version(t), "4t") == 0);
        goog_destroy(t);
    }

    printf("test_pps: PASS\n");
    return 0;
}
