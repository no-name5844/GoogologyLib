#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "googology/core/notation.h"
#include "googology/notations/number/knuth.h"
#include "googology/notations/number/conway.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST %s ... ", name); fflush(stdout); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

// --- Knuth tests ---

static void test_knuth_parse_and_string(void) {
    TEST("knuth_parse_and_string");
    goog_notation_t* k = goog_knuth_parse("2 ^^ 3");
    char* s = goog_to_latex(k);
    if (strstr(s, "uparrow")) PASS();
    else FAIL("expected uparrow in LaTeX output");
    free(s);
    goog_destroy(k);
}

static void test_knuth_expand_once(void) {
    TEST("knuth_expand_once");
    goog_notation_t* k = goog_knuth_parse("2 ^^ 3");
    goog_expand(k, 1);
    char* s = goog_to_latex(k);
    if (strstr(s, "uparrow")) PASS();
    else FAIL("expected uparrow after expand");
    free(s);
    goog_destroy(k);
}

static void test_knuth_capabilities(void) {
    TEST("knuth_capabilities");
    goog_notation_t* k = goog_knuth_parse("2 ^^ 3");
    goog_capabilities_t c = goog_capabilities(k);
    if (goog_capabilities_has(&c, OP_FROM_STRING) &&
        goog_capabilities_has(&c, OP_TO_STRING) &&
        goog_capabilities_has(&c, OP_EXPAND) &&
        goog_capabilities_has(&c, OP_EXPAND_TO) &&
        !goog_capabilities_has(&c, OP_COMPARE)) PASS();
    else FAIL("capabilities mismatch");
    goog_destroy(k);
}

static void test_knuth_parse_unicode(void) {
    TEST("knuth_parse_unicode");
    goog_notation_t* k = goog_knuth_parse("2 \xe2\x86\x91 3");  // UTF-8 up-arrow
    char* s = goog_to_latex(k);
    if (strstr(s, "uparrow")) PASS();
    else FAIL("expected uparrow from unicode input");
    free(s);
    goog_destroy(k);
}

static void test_knuth_expand_to(void) {
    TEST("knuth_expand_to");
    goog_notation_t* k = goog_knuth_parse("2 ^^ 3");
    goog_expand_to(k, 20);
    char* s = goog_to_latex(k);
    if (strlen(s) >= 20) PASS();
    else FAIL("expected length >= 20 after expand_to");
    free(s);
    goog_destroy(k);
}

// --- Conway tests ---

static void test_conway_parse_and_string(void) {
    TEST("conway_parse_and_string");
    goog_notation_t* c = goog_conway_parse("3 -> 3 -> 2");
    char* s = goog_to_latex(c);
    if (strstr(s, "rightarrow")) PASS();
    else FAIL("expected rightarrow in LaTeX output");
    free(s);
    goog_destroy(c);
}

static void test_conway_expand_once(void) {
    TEST("conway_expand_once");
    goog_notation_t* c = goog_conway_parse("3 -> 3 -> 2");
    goog_expand(c, 1);
    char* s = goog_to_latex(c);
    if (strstr(s, "rightarrow")) PASS();
    else FAIL("expected rightarrow after expand");
    free(s);
    goog_destroy(c);
}

static void test_conway_trailing_one(void) {
    TEST("conway_trailing_one");
    goog_notation_t* c = goog_conway_parse("3 -> 4 -> 1");
    goog_expand(c, 1);
    char* s = goog_to_latex(c);
    // 3 -> 4 -> 1 should reduce to 3 -> 4
    if (strstr(s, "4")) PASS();
    else FAIL("expected 4 after expanding trailing 1");
    free(s);
    goog_destroy(c);
}

static void test_conway_capabilities(void) {
    TEST("conway_capabilities");
    goog_notation_t* c = goog_conway_parse("3 -> 3 -> 2");
    goog_capabilities_t cap = goog_capabilities(c);
    if (goog_capabilities_has(&cap, OP_FROM_STRING) &&
        goog_capabilities_has(&cap, OP_TO_STRING) &&
        goog_capabilities_has(&cap, OP_EXPAND) &&
        goog_capabilities_has(&cap, OP_EXPAND_TO) &&
        !goog_capabilities_has(&cap, OP_COMPARE)) PASS();
    else FAIL("capabilities mismatch");
    goog_destroy(c);
}

static void test_conway_parse_unicode(void) {
    TEST("conway_parse_unicode");
    // UTF-8: → = E2 86 92
    const char* input = "3 \xe2\x86\x92 3";
    goog_notation_t* c = goog_conway_parse(input);
    char* s = goog_to_latex(c);
    // 2-element chain renders as a^{b}
    if (strstr(s, "^{")) PASS();
    else FAIL("expected power notation from unicode input");
    free(s);
    goog_destroy(c);
}

static void test_conway_name(void) {
    TEST("conway_name");
    goog_notation_t* c = goog_conway_new();
    const char* n = goog_name(c);
    if (strcmp(n, "conway") == 0) PASS();
    else FAIL("expected name 'conway'");
    goog_destroy(c);
}

static void test_knuth_name(void) {
    TEST("knuth_name");
    goog_notation_t* k = goog_knuth_new();
    const char* n = goog_name(k);
    if (strcmp(n, "knuth") == 0) PASS();
    else FAIL("expected name 'knuth'");
    goog_destroy(k);
}

int main(void) {
    printf("GoogologyLib C tests\n");
    printf("====================\n\n");

    printf("Knuth up-arrow:\n");
    test_knuth_name();
    test_knuth_parse_and_string();
    test_knuth_parse_unicode();
    test_knuth_expand_once();
    test_knuth_expand_to();
    test_knuth_capabilities();

    printf("\nConway chained arrow:\n");
    test_conway_name();
    test_conway_parse_and_string();
    test_conway_parse_unicode();
    test_conway_expand_once();
    test_conway_trailing_one();
    test_conway_capabilities();

    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}