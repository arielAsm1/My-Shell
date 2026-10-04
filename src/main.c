#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "rc.h"
#include "tokenizer.h"
#include "executor.h"
#include "builtin.h"

#define READ_BUFFER_SIZE (1024)

bool process_single_line()
{
    char buff[READ_BUFFER_SIZE] = { 0 };
    char** tokens = NULL;
    size_t tokens_size = 0;
    char* gets_ret = NULL;
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    bool should_continue = true;

    gets_ret = gets(buff);
    CLEANUP_IF_TRUE((NULL == gets_ret), RETURN_CODE__GETS_FAILED);

    if (strcmp(buff, "exit") == 0)
    {
        should_continue = false;
        goto cleanup;
    }

    rc = TOKENIZER__split_by_space(buff, &tokens, &tokens_size);
    CLEANUP_NON_SUCCESS(rc);

    if (tokens_size > 0)
        {
            rc = BUILTIN__execute_command(tokens, tokens_size);
            if (RETURN_CODE__COMMAND_DOES_NOT_EXIST == rc)
            {
                rc = EXECUTOR__execute_command(tokens, tokens_size);
                CLEANUP_NON_SUCCESS(rc);
            }
            else
            {
                CLEANUP_NON_SUCCESS(rc);
            }
        }

    rc = RETURN_CODE__SUCCESS;

cleanup:
    if (RETURN_CODE__SUCCESS != rc && RETURN_CODE__GETS_FAILED != rc)
    {
        printf("error in shell loop rc = %d\n", rc);
    }

    if (RETURN_CODE__GETS_FAILED == rc)
    {
        should_continue = false;
    }

    TOKENIZER__free_tokens(tokens, tokens_size);

    return should_continue;
}

int main()
{
    rc_t rc = SHELL__init_state();
    CLEANUP_NON_SUCCESS(rc);

    while (process_single_line())
    {
    }

cleanup:
    return rc;
}