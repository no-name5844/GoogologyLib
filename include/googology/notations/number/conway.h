#ifndef GOOGOLOGY_CONWAY_H
#define GOOGOLOGY_CONWAY_H

#include "googology/core/notation.h"

// Conway chained-arrow notation. Create with goog_conway_new() or goog_conway_parse().
goog_notation_t* goog_conway_new(void);
goog_notation_t* goog_conway_parse(const char* s);

#endif // GOOGOLOGY_CONWAY_H