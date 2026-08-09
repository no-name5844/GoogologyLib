#include "googology/notations/number/knuth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- AST node ---
typedef struct knuth_node {
    int is_val;
    goog_int_t val;
    goog_int_t base_val;
    goog_int_t height;
    struct knuth_node* exp;
} knuth_node_t;

static knuth_node_t* knuth_node_value(goog_int_t v) {
    knuth_node_t* n = calloc(1, sizeof(knuth_node_t));
    n->is_val = 1;
    n->val = v;
    return n;
}

static knuth_node_t* knuth_node_arrow(goog_int_t base, goog_int_t h, knuth_node_t* e) {
    knuth_node_t* n = calloc(1, sizeof(knuth_node_t));
    n->is_val = 0;
    n->base_val = base;
    n->height = h;
    n->exp = e;
    return n;
}

static void knuth_node_free(knuth_node_t* n) {
    if (!n) return;
    if (n->exp) knuth_node_free(n->exp);
    free(n);
}

// --- Knuth struct ---
typedef struct {
    goog_notation_t base;
    knuth_node_t* root;
} knuth_t;

// Forward declarations
static const char* knuth_name(const goog_notation_t* n);
static goog_family_t knuth_family(const goog_notation_t* n);
static const char* knuth_creator(const goog_notation_t* n);
static const char* knuth_version(const goog_notation_t* n);
static goog_capabilities_t knuth_capabilities(const goog_notation_t* n);
static void knuth_string_to_it(goog_notation_t* n, const char* s);
static char* knuth_to_latex(const goog_notation_t* n);
static void knuth_expand(goog_notation_t* n, goog_int_t steps);
static void knuth_expand_to(goog_notation_t* n, goog_int_t len);
static int knuth_compare(const goog_notation_t* a, const goog_notation_t* b);
static void knuth_destroy(goog_notation_t* n);

static const goog_notation_vtable_t knuth_vtable = {
    .name = knuth_name,
    .family = knuth_family,
    .creator = knuth_creator,
    .version = knuth_version,
    .capabilities = knuth_capabilities,
    .string_to_it = knuth_string_to_it,
    .to_latex = knuth_to_latex,
    .expand = knuth_expand,
    .expand_to = knuth_expand_to,
    .compare = knuth_compare,
    .destroy = knuth_destroy,
};

// --- Vtable implementations ---
static const char* knuth_name(const goog_notation_t* n) { (void)n; return "knuth"; }
static goog_family_t knuth_family(const goog_notation_t* n) { (void)n; return FAMILY_NUMBER; }
static const char* knuth_creator(const goog_notation_t* n) { (void)n; return "Donald Knuth"; }
static const char* knuth_version(const goog_notation_t* n) { (void)n; return "1"; }

static goog_capabilities_t knuth_capabilities(const goog_notation_t* n) {
    (void)n;
    goog_capabilities_t c = {0};
    goog_capabilities_set(&c, OP_FROM_STRING);
    goog_capabilities_set(&c, OP_TO_STRING);
    goog_capabilities_set(&c, OP_EXPAND);
    goog_capabilities_set(&c, OP_EXPAND_TO);
    return c;
}

// --- Internal helpers ---

static char* tex_node(const knuth_node_t* node) {
    if (node->is_val) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%lld", (long long)node->val);
        return strdup(buf);
    }
    char* bt = tex_node(&(knuth_node_t){.is_val = 1, .val = node->base_val});
    char* et = node->exp ? tex_node(node->exp) : strdup("");
    char arr[64];
    if (node->height == 1)
        snprintf(arr, sizeof(arr), "\\uparrow");
    else
        snprintf(arr, sizeof(arr), "\\uparrow^{%lld}", (long long)node->height);

    int exp_has_arrow = node->exp && !node->exp->is_val;
    char* result;
    if (exp_has_arrow)
        result = malloc(strlen(bt) + strlen(arr) + strlen(et) + 10);
    else
        result = malloc(strlen(bt) + strlen(arr) + strlen(et) + 10);

    if (exp_has_arrow)
        sprintf(result, "%s %s (%s)", bt, arr, et);
    else
        sprintf(result, "%s %s %s", bt, arr, et);

    free(bt); free(et);
    return result;
}

static knuth_node_t* step_node(const knuth_node_t* node) {
    if (node->is_val) {
        return knuth_node_value(node->val);
    }
    if (node->exp && node->exp->is_val) {
        goog_int_t b = node->exp->val;
        if (b == 1) {
            return knuth_node_value(node->base_val);  // a ^c 1 = a
        }
        if (node->height == 1) {
            return knuth_node_arrow(node->base_val, node->height,
                                    knuth_node_value(node->exp->val));
        }
        knuth_node_t* inner = knuth_node_arrow(node->base_val, node->height,
                                                knuth_node_value(b - 1));
        return knuth_node_arrow(node->base_val, node->height - 1, inner);
    }
    // recurse into the rightmost sub-term
    return knuth_node_arrow(node->base_val, node->height, step_node(node->exp));
}

static knuth_node_t* parse_expr(const char** sp) {
    const char* s = *sp;
    while (*s == ' ') s++;

    // Find first '^'
    const char* caret = strchr(s, '^');
    if (!caret) {
        // bare integer
        char* end;
        goog_int_t v = strtoll(s, &end, 10);
        *sp = end;
        return knuth_node_value(v);
    }

    // Parse base
    char base_str[64];
    size_t base_len = (size_t)(caret - s);
    strncpy(base_str, s, base_len);
    base_str[base_len] = '\0';
    goog_int_t base = strtoll(base_str, NULL, 10);

    // Count carets
    int c = 0;
    const char* j = caret;
    while (*j == '^') { c++; j++; }

    // Parse exponent
    *sp = j;
    knuth_node_t* exp = parse_expr(sp);
    return knuth_node_arrow(base, c, exp);
}

static void knuth_string_to_it(goog_notation_t* n, const char* s) {
    knuth_t* k = (knuth_t*)n;
    if (k->root) { knuth_node_free(k->root); k->root = NULL; }

    // Normalize: replace UTF-8 up-arrow with ^, remove spaces
    char* t = strdup(s);
    char* p = t;
    char* q = t;
    while (*p) {
        if ((unsigned char)*p == 0xE2 && (unsigned char)*(p+1) == 0x86 && (unsigned char)*(p+2) == 0x91) {
            *q++ = '^';
            p += 3;
        } else if (!isspace((unsigned char)*p)) {
            *q++ = *p++;
        } else {
            p++;
        }
    }
    *q = '\0';

    if (strlen(t) == 0) { free(t); return; }

    const char* sp = t;
    k->root = parse_expr(&sp);
    free(t);
}

static char* knuth_to_latex(const goog_notation_t* n) {
    const knuth_t* k = (const knuth_t*)n;
    if (!k->root) return strdup("");
    return tex_node(k->root);
}

static void knuth_expand(goog_notation_t* n, goog_int_t steps) {
    knuth_t* k = (knuth_t*)n;
    for (goog_int_t i = 0; i < steps; i++) {
        knuth_node_t* new_root = step_node(k->root);
        knuth_node_free(k->root);
        k->root = new_root;
    }
}

static void knuth_expand_to(goog_notation_t* n, goog_int_t len) {
    char* prev = knuth_to_latex(n);
    while (1) {
        knuth_expand(n, 1);
        char* cur = knuth_to_latex(n);
        if (strlen(cur) >= (size_t)len || strcmp(cur, prev) == 0) {
            free(prev); free(cur);
            return;
        }
        free(prev);
        prev = cur;
    }
}

static int knuth_compare(const goog_notation_t* a, const goog_notation_t* b) {
    (void)a; (void)b;
    return GOOG_ERR_NOT_COMPARABLE;
}

static void knuth_destroy(goog_notation_t* n) {
    knuth_t* k = (knuth_t*)n;
    if (k->root) knuth_node_free(k->root);
    free(k);
}

// --- Public API ---
goog_notation_t* goog_knuth_new(void) {
    knuth_t* k = calloc(1, sizeof(knuth_t));
    k->base.vtable = &knuth_vtable;
    k->root = knuth_node_value(0);
    return (goog_notation_t*)k;
}

goog_notation_t* goog_knuth_parse(const char* s) {
    goog_notation_t* n = goog_knuth_new();
    knuth_string_to_it(n, s);
    return n;
}