#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <security/pam_modules.h>
#include <syslog.h>

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

__attribute__((visibility ("default")))
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    const char* user;
    {
        int r = pam_get_item(pamh, PAM_USER, (void*)&user);
        if (r != PAM_SUCCESS) {
            syslog(LOG_AUTHPRIV | LOG_ERR, "Can't get user name: %s", pam_strerror(pamh, r));
            return PAM_SESSION_ERR;
        }
    }
    return PAM_SUCCESS;
}

__attribute__((visibility ("default")))
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
    return PAM_SUCCESS;
}
