#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "rc.h"

#define MAX_EXPANDED_TOKEN_SIZE (4096)
#define MAX_ENV_VAR_NAME_SIZE (256)

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

rc_t SHELL__get_env(char * key, char ** output_val)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    size_t i = 0;

    CLEANUP_IF_TRUE((NULL == key) || (NULL == output_val), RETURN_CODE__NULL_PARAM);    

    *output_val = NULL;

    for(i = 0; i < g_current_state.env_count; i++)
    {
        if(0 == strcmp(g_current_state.env_vars[i]->key, key))
        {
            *output_val = g_current_state.env_vars[i]->val;
            break;
        }
    }

    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}

rc_t SHELL__env_vars(char ** tokens, size_t size)
{
	rc_t rc = RETURN_CODE__UNINITIALIZED;
	size_t i = 0;
	size_t index = 0;

	char * src = NULL;
	char * val = NULL;
	char var_name[MAX_ENV_VAR_NAME_SIZE] = {0};

	char * expanded_token = NULL;
	char * realloc_ptr = NULL;
	size_t current_size = 0;
	size_t val_len = 0;
	
	CLEANUP_IF_TRUE((NULL == tokens) || (0 == size), RETURN_CODE__NULL_PARAM);

	for(i = 0; i < size; i++)
	{
		src = tokens[i];

		current_size = 0;
		expanded_token = (char *)malloc(sizeof(char));
		CLEANUP_IF_TRUE((NULL == expanded_token), RETURN_CODE__MALLOC_FAILED);
		expanded_token[0] = '\0';

		while('\0' != *src)
		{
			if('$' != *src)
			{
				realloc_ptr = (char *)realloc(expanded_token, current_size + 2);
				CLEANUP_IF_TRUE((NULL == realloc_ptr), RETURN_CODE__REALLOC_FAILED);
				expanded_token = realloc_ptr;

				expanded_token[current_size++] = *src;
				src++;
				expanded_token[current_size] = '\0';
				continue;
			}

			src++;
			index = 0;
			memset(var_name, 0, MAX_ENV_VAR_NAME_SIZE);

			while(
				(('a' <= *src) && ('z' >= *src)) ||
				(('A' <= *src) && ('Z' >= *src)) ||
				(('0' <= *src) && ('9' >= *src)) ||
				('_' == *src)
			)
			{
				if(index < MAX_ENV_VAR_NAME_SIZE)
				{
					var_name[index] = *src;
					index++;
				}
				src++;
			}

			rc = SHELL__get_env(var_name, &val);
			CLEANUP_NON_SUCCESS(rc);

			if(NULL != val)
			{
				val_len = strlen(val);
				realloc_ptr = (char *)realloc(expanded_token, current_size + val_len + 1);
				CLEANUP_IF_TRUE((NULL == realloc_ptr), RETURN_CODE__REALLOC_FAILED);
				expanded_token = realloc_ptr;

				strncpy(expanded_token + current_size, val, val_len);
				current_size += val_len;
				expanded_token[current_size] = '\0';
			}
		}
        free(tokens[i]);
		tokens[i] = expanded_token;
	}

	rc = RETURN_CODE__SUCCESS;

cleanup:
	return rc;
}