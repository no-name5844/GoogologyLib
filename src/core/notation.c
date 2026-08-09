#include "googology/core/notation.h"
#include <stdio.h>
#include <stdlib.h>

const char* goog_notation_strerror(goog_error_t err) {
    switch (err) {
        case GOOG_OK: return "no error";
        case GOOG_ERR_UNSUPPORTED_OP: return "unsupported operation";
        case GOOG_ERR_NOT_COMPARABLE: return "comparison is undefined for large-number notations";
        case GOOG_ERR_INVALID_INPUT: return "invalid input";
        default: return "unknown error";
    }
}

void goog_notation_perror(const char* msg, goog_error_t err) {
    fprintf(stderr, "%s: %s\n", msg, goog_notation_strerror(err));
}