#include "googology/notations/number/conway.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Chain node ---
typedef struct conway_node {
    int is_int;
    goog_int_t val;
    struct conway_node* sub;
    size_t sub_len;
} conway_node_t;

static conway_node_t* conway_node_value(goog_int_t v) {
    conway_node_t* n = calloc(1, sizeof(conway_node_t));
    n->is_int = 1;
    n->val = v;
    return n;
}

static conway_node_t* conway_node_subchain(conway_node_t* s, size_t len) {
    conway_node_t* n = calloc(1, sizeof(conway_node_t));
    n->is_int = 0;
    n->sub = s;
    n->sub_len = len;
    return n;
}

static void conway_node_free(conway_node_t* n) {
    if (!n) return;
    if (!n->is_int && n->sub) {
        for (size_t i = 0; i < n->sub_len; i++)
            conway_node_free(&n->sub[i]);
        free(n->sub);
    }
    free(n);
}

// --- Conway struct ---
typedef struct {
    goog_notation_t base;
    conway_node_t* chain;
    size_t chain_len;
} conway_t;

// Forward declarations
static const char* conway_name(const goog_notation_t* n);
static goog_family_t conway_family(const goog_notation_t* n);
static const char* conway_creator(const goog_notation_t* n);
static const char* conway_version(const goog_notation_t* n);
static goog_capabilities_t conway_capabilities(const goog_notation_t* n);
static void conway_string_to_it(goog_notation_t* n, const char* s);
static char* conway_to_latex(const goog_notation_t* n);
static void conway_expand(goog_notation_t* n, goog_int_t steps);
static void conway_expand_to(goog_notation_t* n, goog_int_t len);
static int conway_compare(const goog_notation_t* a, const goog_notation_t* b);
static void conway_destroy(goog_notation_t* n);

static const goog_notation_vtable_t conway_vtable = {
    .name = conway_name,
    .family = conway_family,
    .creator = conway_creator,
    .version = conway_version,
    .capabilities = conway_capabilities,
    .string_to_it = conway_string_to_it,
    .to_latex = conway_to_latex,
    .expand = conway_expand,
    .expand_to = conway_expand_to,
    .compare = conway_compare,
    .destroy = conway_destroy,
};

// --- Vtable implementations ---
static const char* conway_name(const goog_notation_t* n) { (void)n; return "conway"; }
static goog_family_t conway_family(const goog_notation_t* n) { (void)n; return FAMILY_NUMBER; }
static const char* conway_creator(const goog_notation_t* n) { (void)n; return "John Conway"; }
static const char* conway_version(const goog_notation_t* n) { (void)n; return "1"; }

static goog_capabilities_t conway_capabilities(const goog_notation_t* n) {
    (void)n;
    goog_capabilities_t c = {0};
    goog_capabilities_set(&c, OP_FROM_STRING);
    goog_capabilities_set(&c, OP_TO_STRING);
    goog_capabilities_set(&c, OP_EXPAND);
    goog_capabilities_set(&c, OP_EXPAND_TO);
    return c;
}

// --- Internal helpers ---

static char* ser_chain(const conway_node_t* ch, size_t len) {
    if (len == 2 && ch[0].is_int && ch[1].is_int) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%lld^{%lld}", (long long)ch[0].val, (long long)ch[1].val);
        return strdup(buf);
    }
    // Calculate total length
    size_t total = 0;
    for (size_t i = 0; i < len; i++) {
        if (i > 0) total += 14; // " \\rightarrow "
        if (ch[i].is_int) {
            char buf[32];
            total += snprintf(buf, sizeof(buf), "%lld", (long long)ch[i].val);
        } else {
            char* sub = ser_chain(ch[i].sub, ch[i].sub_len);
            total += strlen(sub) + 2; // parentheses
            free(sub);
        }
    }
    char* result = malloc(total + 1);
    result[0] = '\0';
    for (size_t i = 0; i < len; i++) {
        if (i > 0) strcat(result, " \\rightarrow ");
        if (ch[i].is_int) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%lld", (long long)ch[i].val);
            strcat(result, buf);
        } else {
            char* sub = ser_chain(ch[i].sub, ch[i].sub_len);
            strcat(result, "(");
            strcat(result, sub);
            strcat(result, ")");
            free(sub);
        }
    }
    return result;
}

static conway_node_t* copy_chain(const conway_node_t* ch, size_t len) {
    conway_node_t* r = calloc(len, sizeof(conway_node_t));
    for (size_t i = 0; i < len; i++) {
        r[i].is_int = ch[i].is_int;
        r[i].val = ch[i].val;
        if (!ch[i].is_int) {
            r[i].sub = copy_chain(ch[i].sub, ch[i].sub_len);
            r[i].sub_len = ch[i].sub_len;
        }
    }
    return r;
}

static void free_chain(conway_node_t* ch, size_t len) {
    if (!ch) return;
    for (size_t i = 0; i < len; i++) {
        if (!ch[i].is_int && ch[i].sub) {
            free_chain(ch[i].sub, ch[i].sub_len);
        }
    }
    free(ch);
}

static conway_node_t* step_once(const conway_node_t* ch, size_t len, size_t* out_len);
static conway_node_t* recurse_sub(const conway_node_t* ch, size_t len, size_t* out_len);

static conway_node_t* step_once(const conway_node_t* ch, size_t len, size_t* out_len) {
    if (len <= 2) {
        if (len <= 1) { *out_len = len; return copy_chain(ch, len); }
        if (ch[0].is_int && ch[1].is_int) { *out_len = len; return copy_chain(ch, len); }
        return recurse_sub(ch, len, out_len);
    }

    // rule 2a: trailing 1
    if (ch[len-1].is_int && ch[len-1].val == 1) {
        *out_len = len - 1;
        return copy_chain(ch, len - 1);
    }

    // rule 2b: middle 1
    if (ch[len-2].is_int && ch[len-2].val == 1) {
        *out_len = len - 2;
        return copy_chain(ch, len - 2);
    }

    // rule 3: X -> a -> b = X -> (X -> a-1 -> b) -> b-1
    if (ch[len-1].is_int && ch[len-2].is_int) {
        goog_int_t a = ch[len-2].val;
        goog_int_t b = ch[len-1].val;
        size_t x_len = len - 2;
        *out_len = x_len + 2;  // X + subchain + (b-1)

        conway_node_t* result = calloc(*out_len, sizeof(conway_node_t));
        // Copy X
        for (size_t i = 0; i < x_len; i++) {
            result[i] = ch[i];  // shallow copy is fine since we're creating new chain
            if (!ch[i].is_int) {
                result[i].sub = copy_chain(ch[i].sub, ch[i].sub_len);
            }
        }
        // Inner chain: X + (a-1) + b
        conway_node_t* inner = calloc(x_len + 2, sizeof(conway_node_t));
        for (size_t i = 0; i < x_len; i++) {
            inner[i].is_int = ch[i].is_int;
            inner[i].val = ch[i].val;
            if (!ch[i].is_int) {
                inner[i].sub = copy_chain(ch[i].sub, ch[i].sub_len);
                inner[i].sub_len = ch[i].sub_len;
            }
        }
        conway_node_t* tmp_a = conway_node_value(a - 1);
        inner[x_len] = *tmp_a;
        free(tmp_a);
        conway_node_t* tmp_b = conway_node_value(b);
        inner[x_len + 1] = *tmp_b;
        free(tmp_b);

        result[x_len].is_int = 0;
        result[x_len].sub = inner;
        result[x_len].sub_len = x_len + 2;
        result[x_len + 1].is_int = 1;
        result[x_len + 1].val = b - 1;

        return result;
    }

    return recurse_sub(ch, len, out_len);
}

static conway_node_t* recurse_sub(const conway_node_t* ch, size_t len, size_t* out_len) {
    conway_node_t* r = copy_chain(ch, len);
    int changed = 0;
    for (size_t i = 0; i < len; i++) {
        if (!r[i].is_int) {
            char* before = ser_chain(r[i].sub, r[i].sub_len);
            size_t new_len;
            conway_node_t* s = step_once(r[i].sub, r[i].sub_len, &new_len);
            char* after = ser_chain(s, new_len);
            if (strcmp(after, before) != 0) {
                free_chain(r[i].sub, r[i].sub_len);
                if (new_len == 1 && s[0].is_int) {
                    r[i].is_int = 1;
                    r[i].val = s[0].val;
                    free(s);
                } else {
                    r[i].is_int = 0;
                    r[i].sub = s;
                    r[i].sub_len = new_len;
                }
                changed = 1;
            } else {
                free_chain(s, new_len);
            }
            free(before); free(after);
        }
    }
    if (changed) { *out_len = len; return r; }
    free_chain(r, len);
    *out_len = len;
    return copy_chain(ch, len);
}

static void conway_string_to_it(goog_notation_t* n, const char* s) {
    conway_t* c = (conway_t*)n;
    if (c->chain) { free_chain(c->chain, c->chain_len); c->chain = NULL; c->chain_len = 0; }

    // Normalize: replace UTF-8 arrow "→" with "->", remove spaces
    char* t = strdup(s);
    char* p = t;
    char* q = t;
    while (*p) {
        if ((unsigned char)*p == 0xE2 && (unsigned char)*(p+1) == 0x86 && (unsigned char)*(p+2) == 0x92) {
            *q++ = '-';
            *q++ = '>';
            p += 3;
        } else if (!isspace((unsigned char)*p)) {
            *q++ = *p++;
        } else {
            p++;
        }
    }
    *q = '\0';

    // Count arrows (-> sequences)
    size_t count = 1;
    for (char* pp = t; *pp; pp++) {
        if (*pp == '-' && *(pp+1) == '>') count++;
    }

    // Allocate chain
    c->chain = calloc(count, sizeof(conway_node_t));
    c->chain_len = count;

    // Split on "->" manually (strtok splits on individual chars, not sequence)
    char* start = t;
    size_t i = 0;
    while (*start && i < count) {
        // Find next "->" delimiter
        char* arrow = strstr(start, "->");
        // Extract token (before the arrow, or the rest)
        if (arrow) {
            *arrow = '\0';
            c->chain[i].is_int = 1;
            c->chain[i].val = strtoll(start, NULL, 10);
            c->chain[i].sub = NULL;
            c->chain[i].sub_len = 0;
            i++;
            start = arrow + 2;
        } else {
            c->chain[i].is_int = 1;
            c->chain[i].val = strtoll(start, NULL, 10);
            c->chain[i].sub = NULL;
            c->chain[i].sub_len = 0;
            i++;
            break;
        }
    }
    free(t);
}

static char* conway_to_latex(const goog_notation_t* n) {
    const conway_t* c = (const conway_t*)n;
    if (!c->chain || c->chain_len == 0) return strdup("");
    return ser_chain(c->chain, c->chain_len);
}

static void conway_expand(goog_notation_t* n, goog_int_t steps) {
    conway_t* c = (conway_t*)n;
    for (goog_int_t i = 0; i < steps; i++) {
        size_t new_len;
        conway_node_t* new_chain = step_once(c->chain, c->chain_len, &new_len);
        free_chain(c->chain, c->chain_len);
        c->chain = new_chain;
        c->chain_len = new_len;
    }
}

static void conway_expand_to(goog_notation_t* n, goog_int_t len) {
    char* prev = conway_to_latex(n);
    while (1) {
        conway_expand(n, 1);
        char* cur = conway_to_latex(n);
        if (strlen(cur) >= (size_t)len || strcmp(cur, prev) == 0) {
            free(prev); free(cur);
            return;
        }
        free(prev);
        prev = cur;
    }
}

static int conway_compare(const goog_notation_t* a, const goog_notation_t* b) {
    (void)a; (void)b;
    return GOOG_ERR_NOT_COMPARABLE;
}

static void conway_destroy(goog_notation_t* n) {
    conway_t* c = (conway_t*)n;
    if (c->chain) free_chain(c->chain, c->chain_len);
    free(c);
}

// --- Public API ---
goog_notation_t* goog_conway_new(void) {
    conway_t* c = calloc(1, sizeof(conway_t));
    c->base.vtable = &conway_vtable;
    return (goog_notation_t*)c;
}

goog_notation_t* goog_conway_parse(const char* s) {
    goog_notation_t* n = goog_conway_new();
    conway_string_to_it(n, s);
    return n;
}