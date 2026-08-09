#ifndef GOOGOLOGY_KNUTH_H
#define GOOGOLOGY_KNUTH_H

#include "googology/core/notation.h"

// Knuth up-arrow notation. Create with goog_knuth_new() or goog_knuth_parse().
// Use the generic goog_* functions for operations.
goog_notation_t* goog_knuth_new(void);
goog_notation_t* goog_knuth_parse(const char* s);

#endif // GOOGOLOGY_KNUTH_H