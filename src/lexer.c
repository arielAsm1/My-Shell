#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "lexer.h"
#include "shell.h"

rc_t LEXER__extract_clean_token(const char* start, const char* end, char** output)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    char* local_output = NULL;
    size_t max_size = 0;
    size_t i = 0;
    const char* iter = NULL;

    CLEANUP_IF_TRUE((NULL == start) || (NULL == end) || (NULL == output) || (start > end), RETURN_CODE__NULL_PARAM);

    max_size = end - start;
    local_output = (char*)malloc((max_size + 1) * sizeof(char));
    CLEANUP_IF_TRUE((NULL == local_output), RETURN_CODE__MALLOC_FAILED);

    iter = start;
    while (iter < end)
    {
        if (*iter != '"')
        {
            local_output[i] = *iter;
            i++;
        }
        iter++;
    }
    local_output[i] = '\0';

    *output = local_output;
    rc = RETURN_CODE__SUCCESS;

cleanup:
    return rc;
}

rc_t LEXER__split(char* command, char*** output, size_t* size)
{
	rc_t rc = RETURN_CODE__UNINITIALIZED;
	size_t i = 0;
	size_t local_size = 0;
	ptrdiff_t current_token_size = 0;
	char * local_token = NULL;
	char ** realloc_ptr = NULL;
	char * tmp = NULL;
	char * iter = NULL;
	char * token_start = NULL;
	char * token = NULL;
	char ** local_output = NULL;
	char * first_char_after_quote_pos = NULL;
	int is_quote = 0;

	token_start = command;
	iter = command;
	while(1)
	{
		
	}

	*size = local_size;
	*output = local_output;

	rc = RETURN_CODE__SUCCESS;

cleanup:
	return rc;
}

rc_t LEXER__split(char* command, char*** output, size_t* size)
{
	rc_t rc = RETURN_CODE__UNINITIALIZED;
	size_t i = 0;
	size_t local_size = 0;
	ptrdiff_t current_token_size = 0;
	char * local_token = NULL;
	char ** realloc_ptr = NULL;
	char * tmp = NULL;
	char * iter = NULL;
	char * token_start = NULL;
	char * token = NULL;
	char ** local_output = NULL;
	char * first_char_after_quote_pos = NULL;
	int is_quote = 0;

	token_start = command;
	iter = command;
	while(*iter != '\0')
	{
		if(!is_quote)
		{
			while(*iter != ' ' && *iter != '\0')
			{
				if(*iter == '"')
				{
					is_quote = 1 - is_quote;
					break;
				}
				iter++;	
			}
			if(is_quote)
			{
				continue;
			}

			current_token_size = iter - token_start;

			token = (char *)malloc((current_token_size + 1) * sizeof(char));
			memcpy(token, token_start, current_token_size);
			token[current_token_size] = '\0';

			realloc_ptr = (char **)realloc(local_output, (local_size + 1) * sizeof(char*));
			local_output = realloc_ptr;
			local_output[local_size] = token;
			local_size++;

			if(*iter == '\0')
			{
				continue;
			}

			iter++;
			token_start = iter;
		}
		else
		{
			current_token_size = iter - token_start;
			token = (char *)malloc((current_token_size + 1) * sizeof(char));
			memcpy(token, token_start, current_token_size);
			token[current_token_size] = '\0';

			iter++;
			first_char_after_quote_pos = iter;
			while(*iter != '"')
			{
				iter++;
			}

			current_token_size += iter - first_char_after_quote_pos;
			tmp = (char *)realloc(token, (current_token_size + 1) * sizeof(char));
			token = tmp;
			memcpy(token + (current_token_size - (iter - first_char_after_quote_pos)), first_char_after_quote_pos, iter - first_char_after_quote_pos);
			token[current_token_size] = '\0'; 
			
			iter++;
			token_start = iter;
			while(*iter != ' ' && *iter != '\0')
			{
				iter++;
			}

			current_token_size += iter - token_start;
			tmp = (char *)realloc(token, (current_token_size + 1) * sizeof(char));
			token = tmp;
			memcpy(token + (current_token_size - (iter - token_start)), token_start, iter - token_start);
			token[current_token_size] = '\0';
			
			realloc_ptr = (char **)realloc(local_output, (local_size + 1) * sizeof(char*));
			local_output = realloc_ptr;
			local_output[local_size] = token;
			local_size++;

			token_start = iter + 1;
		}
	}

	*size = local_size;
	*output = local_output;

	rc = RETURN_CODE__SUCCESS;

cleanup:
	return rc;
}


void LEXER__free_tokens(char** tokens, size_t size)
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

