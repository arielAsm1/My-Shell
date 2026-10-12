#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "rc.h"

static struct shell_state_s g_current_state = { 0 };

static rc_t parse_single_env(char * env_str, struct envoirement_s ** out_env)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    struct envoirement_s * env = NULL;
    char * equal = NULL;
    char * str = NULL;
    size_t key_len = 0;
    size_t val_len = 0;

    CLEANUP_IF_TRUE((NULL == env_str || NULL == out_env), RETURN_CODE__INVALID_ARGUMENT);

    equal = strchr(env_str, '=');
    CLEANUP_IF_TRUE((NULL == equal), RETURN_CODE__STRCHR_FAILED);

    env = (struct envoirement_s *)malloc(sizeof(struct envoirement_s));
    CLEANUP_IF_TRUE((NULL == env), RETURN_CODE__MALLOC_FAILED);
    env->key = NULL;
    env->val = NULL;

    key_len = (size_t)(equal - env_str);
    val_len = strlen(equal + 1);

    env->key = (char *)malloc((key_len + 1) * sizeof(char));
    CLEANUP_IF_TRUE((NULL == env->key), RETURN_CODE__MALLOC_FAILED);
    str = strncpy(env->key, env_str, key_len);
    CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);
    env->key[key_len] = '\0';

    env->val = (char *)malloc(val_len + 1);
    CLEANUP_IF_TRUE((NULL == env->val), RETURN_CODE__MALLOC_FAILED);
    str = strncpy(env->val, equal + 1, val_len + 1);
    CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);

    *out_env = env;
    env = NULL;
    rc = RETURN_CODE__SUCCESS;

cleanup:
    if (NULL != env)
    {
        free(env->key);
        free(env->val);
        free(env);
    }
    return rc;
}

rc_t SHELL__init_state(char * env[])
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char * cwd = NULL;
    size_t count = 0;
    size_t i = 0;

    CLEANUP_IF_TRUE((NULL == env), RETURN_CODE__INVALID_ARGUMENT);

    cwd = getcwd(g_current_state.current_working_dir, MAX_PATH_NAME);
    CLEANUP_IF_TRUE((NULL == cwd), RETURN_CODE__GETCWD_FAILED);

    while (NULL != env[count])
    {
        count++;
    }

    g_current_state.env_vars = (struct envoirement_s **)malloc((count) * sizeof(struct envoirement_s *));
    CLEANUP_IF_TRUE((NULL == g_current_state.env_vars), RETURN_CODE__MALLOC_FAILED);
    g_current_state.env_count = count;

    for (i = 0; i < count; i++)
    {
        rc = parse_single_env(env[i], &g_current_state.env_vars[i]);
        CLEANUP_NON_SUCCESS(rc);
    }

    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}

rc_t SHELL__set_cwd(char * path)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char * str = NULL;
    int ret_val = -1;

    str = strncpy(g_current_state.current_working_dir, path, MAX_PATH_NAME - 1);
    CLEANUP_IF_TRUE((NULL == str), RETURN_CODE__STRNCPY_FAILED);
    g_current_state.current_working_dir[MAX_PATH_NAME - 1] = '\0';

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