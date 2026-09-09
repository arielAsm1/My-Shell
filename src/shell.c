#include <unistd.h>
#include <string.h>

#include "shell.h"
#include "rc.h"

static struct shell_state_s g_current_state;

rc_t SHELL__init_state()
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char * cwd = NULL;

    cwd = getcwd(&g_current_state.current_working_dir, MAX_PATH_SZIE);
    CLEANUP_IF_TRUE((NULL == cwd), RETURN_CODE__GETCWD_FAILED);

    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}

rc_t SHELL__set_cwd(char * path)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char * str = NULL;
    int ret_val = -1;

    str = strncpy(g_current_state.current_working_dir, path, MAX_PATH_SZIE - 1);
    CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);
    g_current_state.current_working_dir[MAX_PATH_SZIE - 1] = '\0';

    ret_val = chdir(g_current_state.current_working_dir);
    CLEANUP_IF_TRUE((0 > ret_val), RETURN_CODE__CHDIR_FAILED);

    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}

rc_t SHELL__get_cwd(char * get_cwd) 
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char * str = NULL;
    
    CLEANUP_IF_TRUE((NULL == get_cwd), RETURN_CODE__INVALID_ARGUMENT);
    
    str = strncpy(get_cwd, g_current_state.current_working_dir, strlen(g_current_state.current_working_dir) + 1);
    CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);
    
    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}