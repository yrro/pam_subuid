#pragma once

#include <stdio.h>

#include <unistd.h>

struct subxid_entry {
    unsigned int start;
    unsigned int count;
};

int subxid_entry_compare(const struct subxid_entry *a, const struct subxid_entry *b);

int logindef_uint(FILE *f, const char *name, unsigned int default_, unsigned int *result);

int find_free_range(FILE *f, const char *owner, unsigned int min, unsigned int max, unsigned int count, unsigned int *result);

int find_new_subuid_range(const char *user, uid_t *range_start, unsigned int *range_count);

int find_new_subgid_range(const char *user, gid_t *range_start, unsigned int *range_count);
