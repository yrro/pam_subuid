#define PAM_SM_SESSION

#include <security/pam_modules.h>

__attribute__((visibility ("default")))
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
	return 0;
}

__attribute__((visibility ("default")))
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]) {
	return 0;
}
