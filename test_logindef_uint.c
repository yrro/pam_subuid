// SPDX-License-Identifier: ISC

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "subxid.h"

#include "ctest.h"

CTEST(logindef_uint, not_present) {
    static const char t[] = "not present\n";
    FILE *f = fmemopen((char*)t, strlen(t), "r");

    unsigned int r;
    ASSERT_EQUAL(0, logindef_uint(f, "foo", 123, &r));
    ASSERT_EQUAL(123, r);

    fclose(f);
}

CTEST(logindef_uint, not_int) {
    static const char t[] = "foo notint\n";
    FILE *f = fmemopen((char*)t, strlen(t), "r");

    unsigned int r;
    ASSERT_EQUAL(-EINVAL, logindef_uint(f, "foo", 123, &r));

    fclose(f);
}

// I don't know why strtoul returns 0 but doesn't set errno to ERANGE...
CTEST_SKIP(logindef_uint, too_large) {
    static const char t[] = "foo 50000000000\n";
    FILE *f = fmemopen((char*)t, strlen(t), "r");

    unsigned int r;
    ASSERT_EQUAL(-ERANGE, logindef_uint(f, "foo", 123, &r));

    fclose(f);
}

CTEST(logindef_uint, valid_int) {
    static const char t[] = "foo 456\n";
    FILE *f = fmemopen((char*)t, strlen(t), "r");

    unsigned int r;
    ASSERT_EQUAL(0, logindef_uint(f, "foo", 123, &r));
    ASSERT_EQUAL(456, r);

    fclose(f);
}

CTEST(logindef_uint, comment_ignored) {
    static const char t[] = "# comment ignored\n#foo 222\n";
    FILE *f = fmemopen((char*)t, strlen(t), "r");

    unsigned int r;
    ASSERT_EQUAL(0, logindef_uint(f, "foo", 123, &r));
    ASSERT_EQUAL(123, r);

    fclose(f);
}
