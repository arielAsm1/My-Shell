#ifndef RC_H
#define RC_H

enum return_code_e
{
	RETURN_CODE__UNINITIALIZED = -1,
	RETURN_CODE__SUCCESS = 0,

	RETURN_CODE__NULL_PARAM,
	RETURN_CODE__MALLOC_FAILED,
	RETURN_CODE__REALLOC_FAILED,
	RETURN_CODE__COULDNT_FIND_COMMAND,
	RETURN_CODE__COMMAND_DOES_NOT_EXIST,

	RETURN_CODE__GETS_FAILED,
    RETURN_CODE__PIPE_FAILED,
    RETURN_CODE__FORK_FAILED,
    RETURN_CODE__EXEC_FAILED,
    RETURN_CODE__READ_FAILED,
    RETURN_CODE__WAITPID_FAILED,



};

typedef enum return_code_e rc_t;


#define CLEANUP_IF_TRUE(__condition, __return_code_value) \
	do \
	{ \
		if ((__condition)) \
		{ \
			rc = __return_code_value; \
			goto cleanup; \
		} \
	} while (0)

#endif