#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "rc.h"
#include "tokenizer.h"
#include "executor.h"

#define BUFFER_SIZE 1024

bool process_single_line()
{
    char buff[BUFFER_SIZE] = { 0 };
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
    CLEANUP_IF_TRUE((RETURN_CODE__SUCCESS != rc), rc);

    if (tokens_size > 0)
    {
        rc = EXECUTOR__execute_command(tokens, tokens_size);
        CLEANUP_IF_TRUE((RETURN_CODE__SUCCESS != rc), rc);
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
    while (process_single_line())
    {
    }

    return 0;
}