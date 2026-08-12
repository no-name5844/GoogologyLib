#include "googology/notations/ordinal/sequence/marked_parent/pps/pps.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// PPS state: variant + dynamic sequence + master-limit flag.
typedef struct {
    goog_notation_t base;
    goog_pps_variant_t variant;
    goog_int_t* seq;
    size_t len;
    size_t cap;
    bool is_master_limit;
} pps_t;

// Forward declarations
static const char* pps_name(const goog_notation_t* n);
static goog_family_t pps_family(const goog_notation_t* n);
static const char* pps_creator(const goog_notation_t* n);
static const char* pps_version(const goog_notation_t* n);
static goog_capabilities_t pps_capabilities(const goog_notation_t* n);
static void pps_string_to_it(goog_notation_t* n, const char* s);
static char* pps_to_latex(const goog_notation_t* n);
static void pps_expand(goog_notation_t* n, goog_int_t steps);
static void pps_expand_to(goog_notation_t* n, goog_int_t len);
static int pps_compare(const goog_notation_t* a, const goog_notation_t* b);
static void pps_destroy(goog_notation_t* n);

static const goog_notation_vtable_t pps_vtable = {
    .name = pps_name,
    .family = pps_family,
    .creator = pps_creator,
    .version = pps_version,
    .capabilities = pps_capabilities,
    .string_to_it = pps_string_to_it,
    .to_latex = pps_to_latex,
    .expand = pps_expand,
    .expand_to = pps_expand_to,
    .compare = pps_compare,
    .destroy = pps_destroy,
};

// --- Vtable implementations ---

static const char* pps_name(const goog_notation_t* n) {
    const pps_t* p = (const pps_t*)n;
    switch (p->variant) {
        case PPS_VARIANT_PPS1:  return "pps1";
        case PPS_VARIANT_PPS2:  return "pps2";
        case PPS_VARIANT_PPS4:  return "pps4";
        case PPS_VARIANT_WPPS4: return "wpps4";
        case PPS_VARIANT_TPPS4: return "tpps4";
        case PPS_VARIANT_FPPS4: return "fpps4";
    }
    return "pps";
}

static goog_family_t pps_family(const goog_notation_t* n) {
    (void)n;
    return FAMILY_ORDINAL;
}

static const char* pps_creator(const goog_notation_t* n) {
    (void)n;
    return "318`4";
}

static const char* pps_version(const goog_notation_t* n) {
    const pps_t* p = (const pps_t*)n;
    switch (p->variant) {
        case PPS_VARIANT_PPS1:  return "1";
        case PPS_VARIANT_PPS2:  return "2";
        case PPS_VARIANT_PPS4:  return "4";
        case PPS_VARIANT_WPPS4: return "4w";
        case PPS_VARIANT_TPPS4: return "4t";
        case PPS_VARIANT_FPPS4: return "4f";
    }
    return "?";
}

static goog_capabilities_t pps_capabilities(const goog_notation_t* n) {
    (void)n;
    goog_capabilities_t c = {0};
    goog_capabilities_set(&c, OP_FROM_STRING);
    goog_capabilities_set(&c, OP_TO_STRING);
    goog_capabilities_set(&c, OP_NORMALIZE);
    goog_capabilities_set(&c, OP_COMPARE);
    goog_capabilities_set(&c, OP_EXPAND);
    goog_capabilities_set(&c, OP_EXPAND_TO);
    goog_capabilities_set(&c, OP_SUCCESSOR);
    return c;
}

// --- Internal helpers ---

static void pps_push(pps_t* p, goog_int_t v) {
    if (p->len >= p->cap) {
        p->cap = p->cap ? p->cap * 2 : 8;
        p->seq = (goog_int_t*)realloc(p->seq, p->cap * sizeof(goog_int_t));
    }
    p->seq[p->len++] = v;
}

// Rightmost 1-based column j in the open interval (lo, hi) with value = b;
// returns 0 if none.
static goog_int_t pps_rightmost_eq(const pps_t* p, goog_int_t lo, goog_int_t hi,
                                   goog_int_t b) {
    for (goog_int_t j = hi - 1; j > lo; --j) {
        if (j < 1 || (size_t)j > p->len) continue;
        if (p->seq[j - 1] == b) return j;
    }
    return 0;
}

// Lexicographic compare of a_{k+j} vs a_{c+j} (j = 0,1,2,...).
static int pps_tail_cmp(const pps_t* p, goog_int_t k, goog_int_t c) {
    goog_int_t y = (goog_int_t)p->len;
    for (goog_int_t j = 0;; ++j) {
        goog_int_t ik = k + j, ic = c + j;
        bool ok_k = ik >= 1 && ik <= y;
        bool ok_c = ic >= 1 && ic <= y;
        if (!ok_k && !ok_c) return 0;
        if (!ok_k) return -1;
        if (!ok_c) return 1;
        if (p->seq[ik - 1] != p->seq[ic - 1])
            return p->seq[ik - 1] > p->seq[ic - 1] ? 1 : -1;
    }
}

// Per-variant last-term replacement. Returns the new last value; *strong
// marks the strong-expansion branch (PPS4 family).
static goog_int_t pps_replace_last(pps_t* p, goog_int_t x, goog_int_t b,
                                   goog_int_t y, bool* strong) {
    *strong = false;
    switch (p->variant) {
        case PPS_VARIANT_PPS1: {
            goog_int_t k = pps_rightmost_eq(p, x, y, b);
            return k != 0 ? b : x - 1;
        }
        case PPS_VARIANT_PPS2: {
            goog_int_t k = pps_rightmost_eq(p, x, y, b);
            if (k != 0 && pps_tail_cmp(p, k, x) > 0) return b;
            return x - 1;
        }
        case PPS_VARIANT_PPS4:
        case PPS_VARIANT_WPPS4:
        case PPS_VARIANT_TPPS4:
        case PPS_VARIANT_FPPS4: {
            goog_int_t k = pps_rightmost_eq(p, x, y, b);
            if (k != 0) return b;   // weak expansion
            *strong = true;          // strong expansion branch
            goog_int_t j = pps_rightmost_eq(p, b, x, b);
            return j != 0 ? j : b;   // not found -> as weak
        }
    }
    return x - 1;
}

// Copy rule: src >= x -> src + L, else src. tPPS4/fPPS4 on the strong branch
// add L to the last-term chain (i ≡ y mod L); fPPS4 sets the first last-term
// copy (i = y) to the bad-root column x.
static goog_int_t pps_copied(const pps_t* p, goog_int_t src, goog_int_t x,
                             goog_int_t L, goog_int_t y, goog_int_t i,
                             bool strong) {
    goog_int_t base = src >= x ? src + L : src;
    if (!strong) return base;
    if (p->variant == PPS_VARIANT_TPPS4 || p->variant == PPS_VARIANT_FPPS4) {
        if (i % L == y % L) {
            if (p->variant == PPS_VARIANT_FPPS4 && i == y) return x;
            return src + L;
        }
    }
    return base;
}

// expandLen(A, M): replace last + append M elements (source i = q + y - L).
// Bad root out of range (x >= y): leave unchanged (C++ throws out_of_range).
static void pps_expand_len(pps_t* p, goog_int_t M) {
    if (p->len == 0) return;
    goog_int_t y = (goog_int_t)p->len;
    goog_int_t x = p->seq[p->len - 1];
    if (x == 0) { p->len--; return; }   // successor: strip last
    if (x >= y) return;                 // bad root out of range
    goog_int_t b = p->seq[x - 1];
    goog_int_t L = y - x;
    bool strong = false;
    p->seq[y - 1] = pps_replace_last(p, x, b, y, &strong);
    for (goog_int_t q = 1; q <= M; ++q) {
        goog_int_t i = q + y - L;
        if (i < 1 || (size_t)i > p->len) return;   // defensive
        goog_int_t src = p->seq[i - 1];
        pps_push(p, pps_copied(p, src, x, L, y, i, strong));
    }
}

static void pps_string_to_it(goog_notation_t* n, const char* s) {
    pps_t* p = (pps_t*)n;
    p->len = 0;
    p->is_master_limit = false;

    size_t sl = strlen(s);
    char* t = (char*)malloc(sl + 1);
    size_t k = 0;
    for (size_t i = 0; i < sl; ++i)
        if (!isspace((unsigned char)s[i])) t[k++] = s[i];
    t[k] = '\0';
    while (k > 0 && (t[0] == '(' || t[0] == '[' || t[0] == '{')) {
        memmove(t, t + 1, k);
        k--;
    }
    while (k > 0 && (t[k - 1] == ')' || t[k - 1] == ']' || t[k - 1] == '}')) {
        t[--k] = '\0';
    }
    char* save = NULL;
    char* tok = strtok_r(t, ",", &save);
    while (tok) {
        pps_push(p, (goog_int_t)strtoll(tok, NULL, 10));
        tok = strtok_r(NULL, ",", &save);
    }
    free(t);
}

static char* pps_to_latex(const goog_notation_t* n) {
    const pps_t* p = (const pps_t*)n;
    if (p->is_master_limit) return strdup("(0, 1, 2, …)");
    size_t need = 2 + p->len * 40;
    char* out = (char*)malloc(need);
    char* w = out;
    *w++ = '(';
    for (size_t i = 0; i < p->len; ++i) {
        if (i) { *w++ = ','; *w++ = ' '; }
        int c = snprintf(w, need - (size_t)(w - out), "%lld", (long long)p->seq[i]);
        w += c;
    }
    *w++ = ')';
    *w = '\0';
    return out;
}

// Library 0-index: expand(k) = 基本列[k+1] = expandLen((k+1)L - 1).
static void pps_expand(goog_notation_t* n, goog_int_t k) {
    pps_t* p = (pps_t*)n;
    if (p->is_master_limit) {
        p->len = 0;
        for (goog_int_t i = 0; i < k; ++i) pps_push(p, i);
        p->is_master_limit = false;
        return;
    }
    if (p->len == 0) return;
    goog_int_t x = p->seq[p->len - 1];
    if (x == 0) { p->len--; return; }
    goog_int_t y = (goog_int_t)p->len;
    if (x >= y) return;   // bad root out of range (C++ throws)
    goog_int_t L = y - x;
    pps_expand_len(p, (k + 1) * L - 1);
}

static void pps_expand_to(goog_notation_t* n, goog_int_t len) {
    pps_expand_len((pps_t*)n, len);
}

// Lexicographic ordinal compare. Master limit is the supremum. Cross-variant
// returns GOOG_ERR_NOT_COMPARABLE (C++ throws NotComparable).
static int pps_compare(const goog_notation_t* a, const goog_notation_t* b) {
    const pps_t* pa = (const pps_t*)a;
    const pps_t* pb = (const pps_t*)b;
    if (pa->variant != pb->variant) return GOOG_ERR_NOT_COMPARABLE;
    if (pa->is_master_limit || pb->is_master_limit) {
        if (pa->is_master_limit && pb->is_master_limit) return 0;
        return pa->is_master_limit ? 1 : -1;
    }
    size_t m = pa->len < pb->len ? pa->len : pb->len;
    for (size_t i = 0; i < m; ++i) {
        if (pa->seq[i] > pb->seq[i]) return 1;
        if (pa->seq[i] < pb->seq[i]) return -1;
    }
    if (pa->len > pb->len) return 1;
    if (pa->len < pb->len) return -1;
    return 0;
}

static void pps_destroy(goog_notation_t* n) {
    pps_t* p = (pps_t*)n;
    free(p->seq);
    free(p);
}

// --- Public API ---

goog_notation_t* goog_pps_new(goog_pps_variant_t v) {
    pps_t* p = (pps_t*)calloc(1, sizeof(pps_t));
    p->base.vtable = &pps_vtable;
    p->variant = v;
    return (goog_notation_t*)p;
}

goog_notation_t* goog_pps_parse(goog_pps_variant_t v, const char* s) {
    goog_notation_t* n = goog_pps_new(v);
    pps_string_to_it(n, s);
    return n;
}

goog_notation_t* goog_pps_limit(goog_pps_variant_t v, goog_int_t n) {
    goog_notation_t* x = goog_pps_new(v);
    pps_t* p = (pps_t*)x;
    for (goog_int_t i = 0; i < n; ++i) pps_push(p, i);
    return x;
}

goog_notation_t* goog_pps_master_limit(goog_pps_variant_t v) {
    goog_notation_t* x = goog_pps_new(v);
    ((pps_t*)x)->is_master_limit = true;
    return x;
}
