#ifndef GOOGOLOGY_PPS_H
#define GOOGOLOGY_PPS_H

#include "googology/core/notation.h"

// PPS (Parented Predecessor Sequence) — marked-parent ordinal sequence
// notation. Created by 318`4. Variants: PPS1 (original) / PPS2 / PPS4 /
// wPPS4 / tPPS4 / fPPS4. PPS3 / ePPS4 / sPPS4 are N/A (no data) — not
// implemented.
//
// Bad root = the x-th term (x = last value); b = bad-root value; L = y - x
// (y = last column). expandLen replaces the last term (per variant) and
// appends elements from the running sequence. expand(k) = 基本列[k+1] =
// expandLen((k+1)L - 1). compare is lexicographic; cross-variant returns
// GOOG_ERR_NOT_COMPARABLE.
//
// Create with goog_pps_new() / goog_pps_parse(); use the generic goog_*
// functions for operations.

typedef enum {
    PPS_VARIANT_PPS1,
    PPS_VARIANT_PPS2,
    PPS_VARIANT_PPS4,
    PPS_VARIANT_WPPS4,
    PPS_VARIANT_TPPS4,
    PPS_VARIANT_FPPS4,
} goog_pps_variant_t;

goog_notation_t* goog_pps_new(goog_pps_variant_t v);
goog_notation_t* goog_pps_parse(goog_pps_variant_t v, const char* s);
goog_notation_t* goog_pps_limit(goog_pps_variant_t v, goog_int_t n);
goog_notation_t* goog_pps_master_limit(goog_pps_variant_t v);

#endif // GOOGOLOGY_PPS_H
