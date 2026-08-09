#ifndef GOOGOLOGY_NOTATION_H
#define GOOGOLOGY_NOTATION_H

#include <stdbool.h>
#include "googology/config.h"
#include "googology/core/capability.h"

// Top-level taxonomy.
typedef enum {
    FAMILY_ORDINAL,
    FAMILY_NUMBER,
    FAMILY_HIERARCHY,
    FAMILY_REAL_SEQUENCE,
} goog_family_t;

// Generic notation interface (vtable-style).
// Each notation type provides a struct with function pointers.
typedef struct goog_notation_t goog_notation_t;

typedef struct {
    const char* (*name)(const goog_notation_t*);
    goog_family_t (*family)(const goog_notation_t*);
    const char* (*creator)(const goog_notation_t*);
    const char* (*version)(const goog_notation_t*);
    goog_capabilities_t (*capabilities)(const goog_notation_t*);
    void (*string_to_it)(goog_notation_t*, const char*);
    char* (*to_latex)(const goog_notation_t*);
    void (*expand)(goog_notation_t*, goog_int_t);
    void (*expand_to)(goog_notation_t*, goog_int_t);
    int (*compare)(const goog_notation_t*, const goog_notation_t*);
    void (*destroy)(goog_notation_t*);
} goog_notation_vtable_t;

struct goog_notation_t {
    const goog_notation_vtable_t* vtable;
};

// Error handling
typedef enum {
    GOOG_OK = 0,
    GOOG_ERR_UNSUPPORTED_OP,
    GOOG_ERR_NOT_COMPARABLE,
    GOOG_ERR_INVALID_INPUT,
} goog_error_t;

// Convenience inline functions
static inline const char* goog_name(const goog_notation_t* n) { return n->vtable->name(n); }
static inline goog_family_t goog_family(const goog_notation_t* n) { return n->vtable->family(n); }
static inline goog_capabilities_t goog_capabilities(const goog_notation_t* n) { return n->vtable->capabilities(n); }
static inline bool goog_can(const goog_notation_t* n, goog_op_t op) {
    goog_capabilities_t c = goog_capabilities(n);
    return goog_capabilities_has(&c, op);
}
static inline void goog_string_to_it(goog_notation_t* n, const char* s) { n->vtable->string_to_it(n, s); }
static inline char* goog_to_latex(const goog_notation_t* n) { return n->vtable->to_latex(n); }
static inline void goog_expand(goog_notation_t* n, goog_int_t steps) { n->vtable->expand(n, steps); }
static inline void goog_expand_to(goog_notation_t* n, goog_int_t len) { n->vtable->expand_to(n, len); }
static inline void goog_destroy(goog_notation_t* n) { n->vtable->destroy(n); }

// Error helpers
const char* goog_notation_strerror(goog_error_t err);
void goog_notation_perror(const char* msg, goog_error_t err);

#endif // GOOGOLOGY_NOTATION_H