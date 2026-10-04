#include <string.h>
#include <stdlib.h>

#include "builtin.h"
#include "rc.h"
#include "shell.h"

struct cd_params_s
{
    char change_dir[MAX_PATH_NAME];
};

// builds the struct
rc_t prepare_cd_params(char ** tokens, size_t tokens_size, void ** parsed_parameters)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    struct cd_params_s * cd_params = NULL;

    CLEANUP_IF_TRUE((NULL == tokens) || (NULL == parsed_parameters) , RETURN_CODE__NULL_PARAM);

    CLEANUP_IF_TRUE((2 != tokens_size), RETURN_CODE__WRONG_NUMBER_OF_PARAMS);

    cd_params = (struct cd_params_s *)malloc(sizeof(struct cd_params_s));
    CLEANUP_IF_TRUE((NULL == cd_params), RETURN_CODE__MALLOC_FAILED);

    (void)strncpy(cd_params->change_dir, tokens[1], MAX_PATH_NAME - 1);
    cd_params->change_dir[MAX_PATH_NAME - 1] = '\0';
    
    *parsed_parameters = (void *)cd_params;

    rc = RETURN_CODE__SUCCESS;
    
cleanup:
    return rc;
}

rc_t handle_cd(void * param)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    struct cd_params_s * cd_params = NULL;
    char * path = NULL;
    char resolved_path[MAX_PATH_NAME] = {0};
    char * tmp = NULL;
    char cwd[MAX_PATH_NAME] = {0};

    CLEANUP_IF_TRUE((NULL == param), RETURN_CODE__NULL_PARAM);

    cd_params = (struct cd_params_s *)param;
    path = cd_params->change_dir;

    if(path[0] != '/')
    {
        rc = SHELL__get_cwd(cwd);
        CLEANUP_NON_SUCCESS(rc);
        (void)strncat(cwd, "/", MAX_PATH_NAME - strlen(cwd) - 1);
        (void)strncat(cwd, path, MAX_PATH_NAME - strlen(cwd) - 1);
    
        path = cwd;
    }

    tmp = realpath(cwd, resolved_path);
    CLEANUP_IF_TRUE((NULL == tmp), RETURN_CODE__REALPATH_FAILED);

    rc = SHELL__set_cwd(resolved_path);
    CLEANUP_NON_SUCCESS(rc);

    rc = RETURN_CODE__SUCCESS;

cleanup:
    if(RETURN_CODE__SUCCESS != rc)
    {
        printf("Directory does not exist\n");
    }

    return rc;
}

rc_t prepare_pwd_params(char ** tokens, size_t tokens_size, void ** parsed_parameters)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;

    CLEANUP_IF_TRUE((NULL == tokens), RETURN_CODE__NULL_PARAM);

    CLEANUP_IF_TRUE((1 != tokens_size), RETURN_CODE__WRONG_NUMBER_OF_PARAMS);

    *parsed_parameters = NULL;
    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}

rc_t handle_pwd(void * param)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char cwd[MAX_PATH_NAME] = {0};

    rc = SHELL__get_cwd(cwd);
    CLEANUP_NON_SUCCESS(rc);

    (void)puts(cwd);
    
    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}

struct builtin_command_s g_commands[] = {
    {
        .name = "cd",
        .execute_command = handle_cd,
        .prepare_arguments = prepare_cd_params,
    },
    {
        .name = "pwd",
        .execute_command = handle_pwd,
        .prepare_arguments = prepare_pwd_params,
    }
};

rc_t BUILTIN__execute_command(char ** tokens, size_t tokens_size)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    size_t i = 0;
    void * generic_param = NULL;

    CLEANUP_IF_TRUE((NULL == tokens), RETURN_CODE__NULL_PARAM);

    for(i = 0; i < (sizeof(g_commands) / sizeof(g_commands[0])); i++)
    {
        if(!strcmp(tokens[0], g_commands[i].name))
        {
            rc = g_commands[i].prepare_arguments(tokens, tokens_size, &generic_param);
            CLEANUP_NON_SUCCESS(rc);

            rc = g_commands[i].execute_command(generic_param);
            goto cleanup;
        }
    }

    rc = RETURN_CODE__COMMAND_DOES_NOT_EXIST;

cleanup:
    return rc;
}