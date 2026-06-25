
// Test library configuration for selinux.cfg
//
// Usage:
// $ cppcheck --check-library --library=selinux --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/selinux.c
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <stdio.h>

#include <selinux/restorecon.h>
#include <selinux/get_default_type.h>
#include <selinux/label.h>
#include <selinux/context.h>
#include <selinux/get_context_list.h>
#include <selinux/avc.h>

void restorecon(void)
{
    // cppcheck-suppress [ignoredReturnValue, nullPointer, invalidFunctionArgBool]
    selinux_restorecon(NULL, true);

    selinux_restorecon_set_sehandle(NULL);

    // cppcheck-suppress ignoredReturnValue
    selinux_restorecon_default_handle();

    // cppcheck-suppress [ignoredReturnValue, nullPointer]
    selinux_restorecon_set_alt_rootpath(NULL);

    // cppcheck-suppress nullPointer
    selinux_restorecon_set_exclude_list(NULL);

    // cppcheck-suppress ignoredReturnValue
    selinux_restorecon_get_skipped_errors();

    struct dir_xattr **arg3;
    // cppcheck-suppress [ignoredReturnValue, nullPointer, invalidFunctionArgBool, uninitvar]
    selinux_restorecon_xattr(NULL, true, &arg3);
}

void get_default_type_fail(void)
{
    // cppcheck-suppress ignoredReturnValue
    selinux_default_type_path();

    char *type1;
    // FIXME: report ignoredReturnValue
    // cppcheck-suppress [nullPointer]
    get_default_type(NULL, &type1);

    char **type2;
    // FIXME: report ignoredReturnValue
    // cppcheck-suppress [uninitvar]
    get_default_type("object_r", type2);

    // cppcheck-suppress memleak
}

void get_default_type_success(void)
{
    char *type = NULL;
    int err = get_default_type("object_r", &type);
    if (err != 0)
        return;
    free(type);
}

void selabel_fail1(void)
{
    // cppcheck-suppress [unreadVariable, constVariablePointer]
    struct selabel_handle *hnd = selabel_open(SELABEL_CTX_FILE, NULL, 1);

    // cppcheck-suppress resourceLeak
}

void selabel_fail2(void)
{
    struct selabel_handle *hnd = selabel_open(SELABEL_CTX_FILE, NULL, 0);

    char *ctx;
    // cppcheck-suppress nullPointerOutOfResources
    selabel_lookup(hnd, &ctx, "/", 0);

    // cppcheck-suppress nullPointerOutOfResources
    selabel_close(hnd);

    // cppcheck-suppress memleak
}

void selabel_success(void)
{
    struct selabel_handle *hnd = selabel_open(SELABEL_CTX_FILE, NULL, 0);

    char *ctx;
    // cppcheck-suppress nullPointerOutOfResources
    selabel_lookup(hnd, &ctx, "/", 0);

    freecon(ctx);

    // cppcheck-suppress nullPointerOutOfResources
    (void)selabel_cmp(hnd, hnd);

    // cppcheck-suppress nullPointerOutOfResources
    selabel_stats(hnd);

    // cppcheck-suppress nullPointerOutOfResources
    selabel_close(hnd);
}

void context_fail1(void)
{
    // cppcheck-suppress [unreadVariable, nullPointer]
    context_t con = context_new(NULL);

    // cppcheck-suppress memleak
}

void context_fail2(void)
{
    // cppcheck-suppress unreadVariable
    context_t con = context_new("kernel");

    // cppcheck-suppress memleak
}

void context_success(void)
{
    context_t con = context_new("system_u:system_r:kernel_t:s0");

    printf("%s: %s %s %s %s\n", context_str(con),
           context_type_get(con), context_range_get(con),
           context_role_get(con), context_user_get(con));

    (void)context_type_set(con, "init_t");

    context_free(con);
}

void get_ordered_context_list_fail1(void)
{
    char **ret;
    // cppcheck-suppress nullPointer
    get_ordered_context_list(NULL, NULL, &ret);

    // cppcheck-suppress memleak
}

void get_ordered_context_list_fail2(void)
{
    char **ret;
    get_ordered_context_list("root", NULL, &ret);

    // cppcheck-suppress mismatchAllocDealloc
    freecon((void*)ret);
}

void get_ordered_context_list_success1(void)
{
    char **ret;
    get_ordered_context_list("root", NULL, &ret);
    freeconary(ret);
}

void get_default_context_with_rolelevel_fail1(void)
{
    char *ctx;
    // cppcheck-suppress nullPointer
    get_default_context_with_rolelevel("root", NULL, "s0", "system_u:system_r:init_t:s0", &ctx);

    // cppcheck-suppress memleak
}

void get_default_context_with_rolelevel_fail2(void)
{
    char *ctx;
    get_default_context_with_rolelevel("root", "sysadm_r", NULL, NULL, &ctx);

    // cppcheck-suppress mismatchAllocDealloc
    freeconary((void*)ctx);
}

void get_default_context_with_rolelevel_success1(void)
{
    char *ctx;
    get_default_context_with_rolelevel("root", "sysadm_r", NULL, NULL, &ctx);
    freecon(ctx);
}

void selinux_status_fail1(void)
{
    // cppcheck-suppress [invalidFunctionArg, ignoredReturnValue]
    selinux_status_open(-1);
    // TODO: report leak
}

void selinux_status_success1(void)
{
    (void)selinux_status_open(0);
    (void)selinux_status_updated();
    selinux_status_close();
}

void realpath_not_final_fail1(void)
{
    char buf[64];
    // cppcheck-suppress bufferAccessOutOfBounds
    (void)realpath_not_final("/root", buf);
}

void realpath_not_final_success1(void)
{
#define PATH_MAX 4096
    char buf[PATH_MAX + 1];
    // cppcheck-suppress ignoredReturnValue
    realpath_not_final("/root", buf);
}

void selinux_getpolicytype_fail1(void)
{
    // cppcheck-suppress nullPointer
    selinux_getpolicytype(NULL);
}

void selinux_getpolicytype_fail2(void)
{
    char *type;
    (void)selinux_getpolicytype(&type);

    // cppcheck-suppress memleak
}

void selinux_check_access_fail1(void)
{
    const char *msg = "Hello World!";
    // cppcheck-suppress [ignoredReturnValue, nullPointer]
    selinux_check_access("foo", "bar", NULL, "baz", msg);
}

void selinux_check_access_success1(void)
{
    (void)selinux_check_access("kernel", "init", "file", "write", NULL);
}

void selinux_trans_to_raw_context_fail1(void)
{
    // FIXME: report ignoredReturnValue
    // cppcheck-suppress nullPointer
    selinux_trans_to_raw_context("kernel", NULL);
}

void selinux_trans_to_raw_context_fail2(void)
{
    char *ctx;
    // FIXME: report ignoredReturnValue
    selinux_trans_to_raw_context("kernel", &ctx);

    // cppcheck-suppress memleak
}

void selinux_trans_to_raw_context_success1(void)
{
    char *ctx;
    (void)selinux_trans_to_raw_context("kernel", &ctx);
    free(ctx);
}

void getseuserbyname_fail1(void)
{
    char *seuser, *level;
    // cppcheck-suppress nullPointer
    getseuserbyname(NULL, &seuser, &level);
    free(seuser);

    // cppcheck-suppress memleak
}

void getseuserbyname_fail2(void)
{
    char *seuser, *level;
    getseuserbyname("root", &seuser, &level);
    free(level);

    // FIXME: report memleak
}

void getseuserbyname_success1(void)
{
    char *seuser, *level;
    getseuserbyname("root", &seuser, &level);
    free(seuser);
    free(level);
}

void danger1(void)
{
    // cppcheck-suppress selinux_reset_configCalled
    selinux_reset_config();
}

void danger2(void)
{
    // cppcheck-suppress [security_disableCalled, ignoredReturnValue]
    security_disable();
}
