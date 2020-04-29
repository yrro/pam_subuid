#define _GNU_SOURCE

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <security/pam_modules.h>
#include <syslog.h>

#include "subxid.h"

__attribute__((visibility ("default")))
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    syslog(LOG_AUTH | LOG_ERR, "pam_subuid is not an auth module");
    return PAM_SERVICE_ERR;
}

__attribute__((visibility ("default")))
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    syslog(LOG_AUTH | LOG_ERR, "pam_subuid is not an auth module");
    return PAM_SERVICE_ERR;
}

__attribute__((visibility ("default")))
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    syslog(LOG_AUTH | LOG_ERR, "pam_subuid is not an account module");
    return PAM_SERVICE_ERR;
}

__attribute__((visibility ("default")))
int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    syslog(LOG_AUTH | LOG_ERR, "pam_subuid is not a passwd module");
    return PAM_SERVICE_ERR;
}

static void cleanup_voidp(void* p) {
    free(*(void**)p);
}

static void cleanup_FILEp(FILE **fp) {
    if (*fp) {
        fclose(*fp);
    }
}

__attribute__((visibility ("default")))
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    bool debug = false;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "debug") == 0) {
            debug = true;
        }
    }

    const char* user;
    {
        int r = pam_get_item(pamh, PAM_USER, (void*)&user);
        if (r != PAM_SUCCESS) {
            syslog(LOG_AUTHPRIV | LOG_ERR, "Can't get user name: %s", pam_strerror(pamh, r));
            return PAM_SESSION_ERR;
        }
    }

    bool run_usermod = false;

    __attribute__((cleanup(cleanup_voidp)))
    char *usermod_cmd = NULL;
    size_t usermod_cmd_len;

    __attribute__((cleanup(cleanup_FILEp)))
    FILE *usermod_cmd_file = open_memstream(&usermod_cmd, &usermod_cmd_len);
    if (usermod_cmd_file == NULL) {
        syslog(LOG_AUTHPRIV | LOG_CRIT, "Can't prepare usermod command buffer: %m");
        return PAM_SESSION_ERR;
    }
    if (fprintf(usermod_cmd_file, "usermod") < 0) {
        syslog(LOG_AUTHPRIV | LOG_CRIT, "Failed to format usermod args: %m");
        return PAM_SESSION_ERR;
    }

    {
        uid_t start;
        unsigned int count;
        int r = find_new_subxid_range(&xid_u, user, &start, &count);
        if (r == -EEXIST) {
            if (debug) {
                syslog(LOG_AUTHPRIV | LOG_DEBUG, "User already has a subuid allocation");
            }
        } else if (r != 0) {
            return PAM_SESSION_ERR;
        } else {
            run_usermod = true;
            if (fprintf(usermod_cmd_file, " -v %u-%u", start, start + count - 1) < 0) {
                syslog(LOG_AUTHPRIV | LOG_CRIT, "Failed to format usermod args: %m");
                return PAM_SESSION_ERR;
            }
        }
    }

    {
        gid_t start;
        unsigned int count;
        int r = find_new_subxid_range(&xid_g, user, &start, &count);
        if (r == -EEXIST) {
            if (debug) {
                syslog(LOG_AUTHPRIV | LOG_DEBUG, "User already has a subgid allocation");
            }
        } else if (r != 0) {
            return PAM_SESSION_ERR;
        } else {
            run_usermod = true;
            if (fprintf(usermod_cmd_file, " -w %u-%u", start, start + count - 1) < 0) {
                syslog(LOG_AUTHPRIV | LOG_CRIT, "Failed to format usermod args: %m");
                return PAM_SESSION_ERR;
            }
        }
    }

    if (fprintf(usermod_cmd_file, " %s", user) < 0) {
        syslog(LOG_AUTHPRIV | LOG_CRIT, "Failed to format usermod args: %m");
        return PAM_SESSION_ERR;
    }

    if (!run_usermod) {
        return PAM_SUCCESS;
    }

    if (fflush(usermod_cmd_file) == EOF) {
        syslog(LOG_AUTHPRIV | LOG_CRIT, "Failed to format usermod args: %m");
        return PAM_SESSION_ERR;
    }

    int r = system(usermod_cmd);
    if (r == -1) {
        syslog(LOG_AUTHPRIV | LOG_CRIT, "Failed to execute '%s': %m", usermod_cmd);
        return PAM_SESSION_ERR;
    } else if (r != 0) {
        syslog(LOG_AUTHPRIV | LOG_ALERT, "Command '%s' failed (%d)", usermod_cmd, r);
        return PAM_SESSION_ERR;
    }

    return PAM_SUCCESS;
}

__attribute__((visibility ("default")))
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    return PAM_SUCCESS;
}
