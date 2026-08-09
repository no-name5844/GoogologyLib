#ifndef GOOGOLOGY_CAPABILITY_H
#define GOOGOLOGY_CAPABILITY_H

#include <stdint.h>
#include <stdbool.h>

// Operations a notation may support (capability model).
typedef enum {
    OP_FROM_STRING = 0,
    OP_TO_STRING = 1,
    OP_NORMALIZE = 2,
    OP_COMPARE = 3,
    OP_EXPAND = 4,
    OP_EXPAND_TO = 5,
    OP_SUCCESSOR = 6,
} goog_op_t;

// Bitset of supported operations.
typedef struct {
    uint8_t bits;
} goog_capabilities_t;

void goog_capabilities_set(goog_capabilities_t* c, goog_op_t op);
bool goog_capabilities_has(const goog_capabilities_t* c, goog_op_t op);

#endif // GOOGOLOGY_CAPABILITY_H