#ifndef SHELL_H
#define SHELL_H

#include "rc.h"

#define MAX_PATH_NAME (2048)


struct shell_state_s
{
    char current_working_dir[MAX_PATH_NAME];
    struct envoirement_s ** env_vars;
    size_t env_count;
};

struct envoirement_s
{
    char * key;
    char * val;
};

rc_t SHELL__init_state(char * env[]);
rc_t SHELL__set_cwd(char * path);
rc_t SHELL__get_cwd(char * get_cwd);
rc_t SHELL__get_env(char * key, char ** output_val);

#endif