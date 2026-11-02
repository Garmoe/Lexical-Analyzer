#ifndef DFA_H
#define DFA_H

#include <stddef.h>
#include <stdio.h>

#include "token.h"

enum {
    CLASS_ANY = 260
};

enum {
    STATE_START = 0,
    STATE_IDENTIFIER = 900,
    STATE_INTEGER = 901,
    STATE_DECIMAL = 902,
    STATE_STRING = 903,
    STATE_COMMENT = 904,
    STATE_ERROR = 905,
    STATE_CHAR = 906
};

int dfa_is_filipino_letter(const unsigned char *source, size_t position,
                           size_t length);
int dfa_is_identifier_start(const char *source, size_t position,
                            size_t length);
int dfa_is_identifier_part(const char *source, size_t position,
                           size_t length);
int dfa_is_whitespace(int character);
int dfa_step(int state, int character);

const AcceptingState *dfa_find_accept(int state);
const char *dfa_token_kind_name(TokenKind kind);
void dfa_write_symbol_span(FILE *out, const char *type, const char *lexeme,
                           size_t length);
void dfa_write_symbol(FILE *out, TokenKind kind, const char *lexeme);
void dfa_write_acceptance_span(FILE *out, const char *lexeme, size_t length,
                               TokenKind kind, const char *classification,
                               int state);
void dfa_write_acceptance(FILE *out, const char *lexeme, TokenKind kind,
                          const char *classification, int state);
void dfa_report_error(FILE *out, size_t line, size_t column,
                      const char *message);

#endif
