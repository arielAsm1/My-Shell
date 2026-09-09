#include "builtin.h"
#include "rc.h"
#include "shell.h"

#define MAX_PATH_NAME (64)

struct cd_params_s
{
    char change_dir[MAX_PATH_NAME];
};

rc_t handle_cd(void * param)
{
    rc_t rc = RETURN_CODE__UNINITIALIZED;
    struct cd_params_s * cd_params = (struct cd_params_s *)param;
    
    rc = SHELL__set_cwd(cd_params->change_dir);
    
    return rc;
}