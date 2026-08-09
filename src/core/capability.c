#include "googology/core/capability.h"

void goog_capabilities_set(goog_capabilities_t* c, goog_op_t op) {
    c->bits |= (uint8_t)(1u << (unsigned int)op);
}

bool goog_capabilities_has(const goog_capabilities_t* c, goog_op_t op) {
    return (c->bits & (uint8_t)(1u << (unsigned int)op)) != 0;
}