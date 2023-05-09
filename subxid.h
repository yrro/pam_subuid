// SPDX-License-Identifier: ISC

#pragma once

#include <stdio.h>

#include <unistd.h>

struct xid {
    char c;
    const char *file;
    const char *param_min;
    const char *param_max;
    const char *param_count;
};

extern struct xid xid_u;
extern struct xid xid_g;

struct subxid_entry {
    unsigned int start;
    unsigned int count;
};

int subxid_entry_compare(const void *a, const void *b);

int logindef_uint(FILE *f, const char *name, unsigned int default_, unsigned int *result);

int find_free_range(FILE *f, const char *owner, unsigned int min, unsigned int max, unsigned int count, unsigned int *result);

int find_new_subxid_range(const struct xid *which, const char *user, uid_t *range_start, unsigned int *range_count);
