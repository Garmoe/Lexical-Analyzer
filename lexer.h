#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdio.h>

int lexer_read_source(const char *path, char **source, size_t *length);
int lexer_scan(const char *source, size_t length, FILE *log, FILE *tokens);

#endif
