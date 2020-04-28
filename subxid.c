#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <syslog.h>
#include <sys/param.h>
#include <unistd.h>

#include "subxid.h"

static void cleanup_voidp(void* p) {
    free(*(void**)p);
}

static void cleanup_FILEp(FILE **fp) {
    if (fp) {
        fclose(*fp);
    }
}

int logindef_uint(FILE *f, const char *name, unsigned int default_, unsigned int *result) {
    assert(f);
    assert(name);
    assert(result);

    if (fseeko(f, 0, SEEK_SET) != 0) {
        return -errno;
    }

    size_t line_len = 0;
    __attribute__((__cleanup__(cleanup_voidp)))
    char *line = NULL;

    while (errno = 0, getline(&line, &line_len, f) != -1) {
        __attribute__((__cleanup__(cleanup_voidp)))
        char *param = NULL;
        __attribute__((__cleanup__(cleanup_voidp)))
        char *value = NULL;

        if (sscanf(line, " %ms %ms ", &param, &value) != 2) {
            continue;
        } else if (param[0] == '#') {
            continue;
        } else if (strcmp(param, name) != 0) {
            continue;
        }

        errno = 0;
        char *value_tail;
        long value_num = strtol(value, &value_tail, 10);
        if (*value_tail != '\0') {
            return -EINVAL;
        }
        *result = value_num; // narrowing conversion
        return 0;
    }

    if (errno) {
        // not EOF, but error reading file
        return -errno;
    }

    *result = default_;
    return 0;
}

int subxid_entry_compare(const struct subxid_entry *a, const struct subxid_entry *b) {
    assert(a);
    assert(b);
    return a->start - b->start;
}

int find_free_range(FILE *f, const char* owner, unsigned int min, unsigned int max, unsigned int count, unsigned int* result) {
    assert(f);
    assert(owner);
    assert(result);

    size_t entries_next = 0; 
    size_t entries_max = 10;
    __attribute__((__cleanup__(cleanup_voidp)))
    struct subxid_entry *entries = reallocarray(NULL, 0, sizeof(struct subxid_entry));
    if (!entries) {
        return -ENOMEM;
    }

    size_t line_len = 0;
    __attribute__((__cleanup__(cleanup_voidp)))
    char *line = NULL;

    while (errno = 0, getline(&line, &line_len, f) != -1) {
        __attribute__((__cleanup__(cleanup_voidp)))
        char *sub_xid_owner = NULL;
        unsigned int sub_xid_start, sub_xid_count;

        struct subxid_entry e;

        if (sscanf(line, "%m[^:]:%u:%u", &sub_xid_owner, &e.start, &e.count) != 3) {
            return -EINVAL;
        } else if (strcmp(owner, sub_xid_owner) == 0) {
            return -EEXIST;
        }

        if (entries_next == entries_max) {
            size_t entries_max_new = 100 + entries_max;
            void *p = reallocarray(entries, entries_max_new, sizeof(struct subxid_entry));
            if (p) {
                entries = p;
                entries_max = entries_max_new;
            } else {
                return -ENOMEM;
            }
        }

        entries[entries_next] = e;
        entries_next++;
    }

    if (errno) {
        // not EOF but error reading file
        return -errno;
    }

    qsort(entries, entries_next, sizeof(struct subxid_entry), subxid_entry_compare);

    unsigned int low = min;
    unsigned int high;
    for (size_t i = 0; i < entries_next; i++) {
        unsigned int first = entries[i].start;
        unsigned int last = first + entries[i].count - 1;

        /* Find the top end of the hole before this range */
        high = first;

        /* Don't allocate IDs after max (included) */
        if (high > max + 1) {
            high = max + 1;
        }

        /* Is the hole before this range large enough? */
        if ((high > low) && ((high - low) >= count)) {
            *result = low;
            return 0;
        }

        /* Compute the low end of the next hole */
        if (low < (last + 1)) {
            low = last + 1;
        }
        if (low > max) {
            return -ERANGE;
        }
    }

    /* Is the remaining unclaimed area large enough? */
    if (((max - low) + 1) >= count) {
        *result = low;
        return 0;
    }

    return -ERANGE;
}

// See FreeIPA's ipaserver/install/server/__init__.py
//static const uid_t freeipa_xid_max = 10000UL * 200000UL + 199999UL;

int find_new_subuid_range(const char* user, uid_t *range_start, unsigned int *range_count) {
    assert(range_start);
    assert(range_count);

    uid_t min, max;
    unsigned int count;
    {
        __attribute__((__cleanup__(cleanup_FILEp)))
        FILE *f = fopen("/etc/login.defs", "r");
        if (!f) {
            syslog(LOG_AUTHPRIV | LOG_ALERT, "Could not open /etc/login.defs");
            return -ENOENT;
        }

        if (logindef_uint(f, "SUB_UID_MIN", 100000UL, &min) != 0) {
            syslog(LOG_AUTHPRIV | LOG_ALERT, "Could not retrieve SUB_UID_MIN from /etc/login.defs");
            return -EBADMSG;
        }

        //if (freeipa_xid_max > min) {
        //    min = freeipa_xid_max;
        //}
        
        if (logindef_uint(f, "SUB_UID_MAX", 600100000UL, &max) != 0) {
            syslog(LOG_AUTHPRIV | LOG_ALERT, "Could not retrieve SUB_UID_MAX from /etc/login.defs");
            return -EBADMSG;
        };

        if (logindef_uint(f, "SUB_UID_COUNT", 65536, &count) != 0) {
            syslog(LOG_AUTHPRIV | LOG_ALERT, "Could not retrieve SUB_UID_COUNT from /etc/login.defs");
            return -EBADMSG;
        }
    }

    if (min > max || count >= max || (min + count - 1) > max) {
        syslog(LOG_AUTHPRIV | LOG_ALERT, "Invalid configuration; SUB_UID_MIN=%u SUB_UID_MAX=%u SUB_UID_COUNT=%u",
            min, max, count);
        return -EINVAL;
    }

    uid_t start;
    {
        __attribute__((__cleanup__(cleanup_FILEp)))
        FILE *f = fopen("/etc/subuid", "r");
        if (!f) {
            syslog(LOG_AUTHPRIV | LOG_ALERT, "Could not open /etc/subuid");
            return -ENOENT;
        }

        if (find_free_range(f, user, min, max, count, &start) != 0) {
            syslog(LOG_AUTHPRIV | LOG_ALERT, "Can't find %u free subuids between %u and %u", count, min, max);
            return -ENOMEM;
        }
    }

    *range_start = start;
    *range_count = count;
    return 0;
}
