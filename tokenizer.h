#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdint.h>
#include "rc.h"


rc_t TOKENIZER__split_by_space(char* command, char*** output, size_t* size);

void TOKENIZER__free_tokens(char** tokens, size_t size);

#endif
