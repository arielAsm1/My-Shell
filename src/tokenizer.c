#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "shell.h"

#define MAX_EXPANDED_TOKEN_SIZE (4096)
#define MAX_ENV_VAR_NAME_SIZE (256)

rc_t TOKENIZER__split_by_space(char* command, char*** output, size_t* size)
{
	rc_t rc = RETURN_CODE__UNINITIALIZED;
	char** local_output = NULL;
	size_t local_size = 0;
	char* iter = NULL;
	char* start = NULL;
	char * token = NULL;
	size_t current_token_size = 0;
	char** realloc_return_value = NULL;
	
	CLEANUP_IF_TRUE((NULL == command) || (NULL == output) || (NULL == size), RETURN_CODE__NULL_PARAM);

	iter = command;
	while (1)
	{
		start = iter;
		while ((*iter != ' ') && (*iter != '\0'))
		{
			iter++;
		}
		
		current_token_size = iter - start;
		token = (char*)malloc((current_token_size + 1) * sizeof(char));
		CLEANUP_IF_TRUE((NULL == token), RETURN_CODE__MALLOC_FAILED);

		strncpy(token, start, current_token_size);
		token[current_token_size] = '\0';

		realloc_return_value = (char**)realloc(local_output, (local_size + 1) * sizeof(char*));
		CLEANUP_IF_TRUE((NULL == realloc_return_value), RETURN_CODE__REALLOC_FAILED);

		local_output = realloc_return_value;
		local_output[local_size++] = token;

		if ((*iter) == '\0')
		{
			break;
		}
		iter++;
	}

	*output = local_output;
	*size = local_size;

	rc = RETURN_CODE__SUCCESS;

cleanup:
	if (RETURN_CODE__SUCCESS != rc)
	{
		if (NULL != local_output)
		{
			for (size_t i = 0; i < local_size; i++)
			{
				free(local_output[i]);
			}
			free(local_output);
		}
	}
	return rc;
}


void TOKENIZER__free_tokens(char** tokens, size_t size)
{
    if (NULL == tokens)
    {
        return;
    }

    for (size_t i = 0; i < size; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}

rc_t TOKENIZER__env_vars(char ** tokens, size_t size)
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
		tokens[i] = expanded_token;
	}

	rc = RETURN_CODE__SUCCESS;

cleanup:
	return rc;
}