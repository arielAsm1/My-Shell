#!/bin/bash

gcc src/main.c src/executor.c src/tokenizer.c src/builtin.c src/shell.c -Isrc/include -o myshell