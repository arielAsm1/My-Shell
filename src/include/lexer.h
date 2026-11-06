#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "rc.h"


rc_t LEXER__split(char* command, char*** output, size_t* size);
void LEXER__free_tokens(char** tokens, size_t size);

#endif
