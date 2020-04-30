// SPDX-License-Identifier: ISC

#include "subxid.h"

#include "ctest.h"

CTEST(subxid_entry_compare, a_lt_b) {
    struct subxid_entry a = {.start = 10, .count = 10};
    struct subxid_entry b = {.start = 20, .count = 10};
    
    ASSERT_EQUAL(-10, subxid_entry_compare(&a, &b));
}

CTEST(subxid_entry_compare, a_gt_b) {
    struct subxid_entry a = {.start = 20, .count = 10};
    struct subxid_entry b = {.start = 10, .count = 10};
    
    ASSERT_EQUAL(10, subxid_entry_compare(&a, &b));
}
