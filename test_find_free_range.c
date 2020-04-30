// SPDX-License-Identifier: ISC

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "subxid.h"

#include "ctest.h"

CTEST(find_free_range, count_0) {
    static const char subuid[] = "foo:20:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(-EINVAL, find_free_range(f, "owner", 20, 80, 0, &start));

    fclose(f);
}

CTEST(find_free_range, min_eq_max) {
    static const char subuid[] = "foo:20:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(-EINVAL, find_free_range(f, "owner", 20, 20, 10, &start));

    fclose(f);
}

CTEST(find_free_range, min_gt_max) {
    static const char subuid[] = "foo:20:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(-EINVAL, find_free_range(f, "owner", 80, 20, 10, &start));

    fclose(f);
}

CTEST(find_free_range, parse_failure) {
    static const char subuid[] = "invalid entry\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(-EBADMSG, find_free_range(f, "owner", 30, 50, 5, &start));

    fclose(f);
}

CTEST(find_free_range, entry_present) {
    static const char subuid[] = "owner:10:20\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(-EEXIST, find_free_range(f, "owner", 30, 50, 5, &start));

    fclose(f);
}

CTEST(find_free_range, entry_not_present) {
    static const char subuid[] = "";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(0, find_free_range(f, "owner", 30, 50, 5, &start));
    ASSERT_EQUAL(30, start);

    fclose(f);
}

CTEST(find_free_range, room_before) {
    static const char subuid[] = "other:40:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(0, find_free_range(f, "owner", 30, 50, 10, &start));
    ASSERT_EQUAL(30, start);

    fclose(f);
}

CTEST(find_free_range, full_before) {
    static const char subuid[] = "other:35:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(0, find_free_range(f, "owner", 30, 60, 10, &start));
    ASSERT_EQUAL(45, start);

    fclose(f);
}

CTEST(find_free_range, fill_gap) {
    static const char subuid[] = "other:30:10\nother:50:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(0, find_free_range(f, "owner", 30, 60, 10, &start));
    ASSERT_EQUAL(40, start);

    fclose(f);
}

CTEST(find_free_range, fill_gap_reversed) {
    static const char subuid[] = "other:50:10\nother:30:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(0, find_free_range(f, "owner", 30, 60, 10, &start));
    ASSERT_EQUAL(40, start);

    fclose(f);
}

CTEST(find_free_range, skip_gap_too_small) {
    static const char subuid[] = "other:30:15\nother:50:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(0, find_free_range(f, "owner", 30, 80, 10, &start));
    ASSERT_EQUAL(60, start);

    fclose(f);
}

CTEST(find_free_range, no_room) {
    static const char subuid[] = "other:30:10\nother:50:10\n";
    FILE *f = fmemopen((char*)subuid, strlen(subuid), "r");

    unsigned int start;
    ASSERT_EQUAL(-ERANGE, find_free_range(f, "owner", 30, 60, 20, &start));

    fclose(f);
}
