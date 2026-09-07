#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"


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
