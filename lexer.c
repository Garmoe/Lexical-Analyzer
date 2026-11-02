#include "lexer.h"
#include "dfa.h"
#include "token.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void write_trace_str(FILE *out, const char *kind, size_t line,
                            size_t column, const char *display,
                            const char *classification)
{
    if (out == NULL) {
        return;
    }
    fprintf(out, "%s | Line %zu, Column %zu | Character: [%s]"
                 " | Class: %s\n",
            kind, line, column, display, classification);
}
static void write_transition_str(FILE *out, size_t line, size_t column,
                                 int from, const char *display, int to)
{
    if (out == NULL) {
        return;
    }
    fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%s]--> q%d\n",
            line, column, from, display, to);
}

static void write_trace(FILE *out, const char *kind, size_t line,
                        size_t column, int c, const char *classification)
{
    if (out == NULL) {
        return;
    }
    const char *display = c == '\n' ? "\\n" :
                          c == '\r' ? "\\r" :
                          c == '\t' ? "\\t" : NULL;
    if (display != NULL) {
        fprintf(out, "%s | Line %zu, Column %zu | Character: [%s]"
                     " | Class: %s\n",
                kind, line, column, display, classification);
    } else {
        fprintf(out, "%s | Line %zu, Column %zu | Character: [%c]"
                     " | Class: %s\n",
                kind, line, column, c, classification);
    }
}

static void write_transition(FILE *out, size_t line, size_t column,
                             int from, int c, int to)
{
    if (out == NULL) {
        return;
    }
    const char *display = c == '\n' ? "\\n" :
                          c == '\r' ? "\\r" :
                          c == '\t' ? "\\t" : NULL;
    if (display != NULL) {
        fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%s]--> q%d\n",
                line, column, from, display, to);
    } else {
        fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%c]--> q%d\n",
                line, column, from, c, to);
    }
}

static int append_char(char **buffer, size_t *length, size_t *capacity,
                       int c)
{
    char *grown;
    size_t new_capacity;
    if (*length + 1 >= *capacity) {
        new_capacity = *capacity == 0 ? 64 : *capacity * 2;
        grown = realloc(*buffer, new_capacity);
        if (grown == NULL) {
            return 0;
        }
        *buffer = grown;
        *capacity = new_capacity;
    }
    (*buffer)[(*length)++] = (char)c;
    (*buffer)[*length] = '\0';
    return 1;
}

static int valid_source_extension(const char *path)
{
    size_t length = strlen(path);
    return length >= 5 &&
           path[length - 5] == '.' &&
           path[length - 4] == 'r' &&
           path[length - 3] == 'i' &&
           path[length - 2] == 'c' &&
           path[length - 1] == 'e';
}

int lexer_read_source(const char *path, char **source, size_t *length)
{
    FILE *file;
    struct stat metadata;
    size_t capacity = 0;
    size_t line = 1;
    size_t column = 1;
    int c;

    if (!valid_source_extension(path)) {
        fprintf(stderr, "Input must use the .rice file extension.\n");
        return 0;
    }
    if (stat(path, &metadata) != 0 || !S_ISREG(metadata.st_mode)) {
        fprintf(stderr, "Input must be an existing regular text file.\n");
        return 0;
    }
    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Cannot open input file '%s': %s\n",
                path, strerror(errno));
        return 0;
    }
    while ((c = fgetc(file)) != EOF) {
        if ((unsigned char)c == 0xC3) {
            int c2 = fgetc(file);
            if (c2 == 0xB1 || c2 == 0x91) {
                if (!append_char(source, length, &capacity, c) ||
                    !append_char(source, length, &capacity, c2)) {
                    fprintf(stderr, "Not enough memory while reading input.\n");
                    fclose(file);
                    free(*source);
                    *source = NULL;
                    *length = 0;
                    return 0;
                }
                ++column;
                continue;
            } else {
                if (c2 != EOF) {
                    ungetc(c2, file);
                }
                fprintf(stderr,
                        "Input contains unsupported character at line %zu, "
                        "column %zu.\n", line, column);
                fclose(file);
                free(*source);
                *source = NULL;
                *length = 0;
                return 0;
            }
        }
        if (c == '\0' || (c < 32 && c != '\t' && c != '\r' && c != '\n') ||
            c > 126) {
            fprintf(stderr,
                    "Input is not supported plain text at line %zu, "
                    "column %zu.\n", line, column);
            fclose(file);
            free(*source);
            *source = NULL;
            *length = 0;
            return 0;
        }
        if (!append_char(source, length, &capacity, c)) {
            fprintf(stderr, "Not enough memory while reading input.\n");
            fclose(file);
            free(*source);
            *source = NULL;
            *length = 0;
            return 0;
        }
        if (c == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
    if (ferror(file)) {
        fprintf(stderr, "Error while reading input file '%s'.\n", path);
        fclose(file);
        free(*source);
        *source = NULL;
        *length = 0;
        return 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Error closing input file '%s'.\n", path);
        return 0;
    }
    return 1;
}

int lexer_scan(const char *source, size_t length, FILE *log,
                       FILE *tokens)
{
    size_t pos = 0;
    size_t line = 1;
    size_t column = 1;
    int errors = 0;

    while (pos < length) {
        size_t token_line = line;
        size_t token_column = column;
        int c = (unsigned char)source[pos];

        if (dfa_is_whitespace(c)) {
            int next = dfa_step(STATE_START, c);
            write_trace(log, "SCAN", line, column, c, "WHITESPACE");
            write_transition(log, line, column, STATE_START, c, next);
            ++pos;
            if (c == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
            continue;
        }

        if (c == '@') {
            size_t comment_start = pos;
            size_t comment_length;
            int state = STATE_START;
            while (pos < length && source[pos] != '\n') {
                if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) {
                    char letter_buf[3] = {source[pos], source[pos + 1], '\0'};
                    int next = dfa_step(state, CLASS_ANY);
                    write_trace_str(log, "SCAN", line, column, letter_buf, "COMMENT");
                    write_transition_str(log, line, column, state, letter_buf, next);
                    state = next;
                    pos += 2;
                    column += 1;
                } else {
                    int current = (unsigned char)source[pos];
                    int next = dfa_step(state, current);
                    write_trace(log, "SCAN", line, column, current,
                                state == STATE_START ? "COMMENT_MARKER" :
                                                       "COMMENT");
                    write_transition(log, line, column, state, current, next);
                    state = next;
                    ++pos;
                    ++column;
                }
            }
            comment_length = pos - comment_start;
            if (comment_length > 0 &&
                source[comment_start + comment_length - 1] == '\r') {
                --comment_length;
            }
            dfa_write_symbol_span(tokens, "LINE_COMMENT",
                              source + comment_start, comment_length);
            dfa_write_acceptance_span(log, source + comment_start, comment_length,
                                  TOKEN_KIND_LINE_COMMENT, "COMMENT", state);
            continue;
        }

        if (c == '_') {
            write_trace(log, "SCAN", line, column, c, "UNDERSCORE");
            write_transition(log, line, column, STATE_START, c, STATE_ERROR);
            while (pos < length && dfa_is_identifier_part(source, pos, length)) {
                if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) {
                    pos += 2;
                } else {
                    pos += 1;
                }
                ++column;
            }
            dfa_report_error(tokens, token_line, token_column,
                         "Identifier must not begin with an underscore.");
            ++errors;
            continue;
        }

        if (dfa_is_identifier_start(source, pos, length)) {
            size_t start = pos;
            int state = STATE_START;
            while (pos < length && dfa_is_identifier_part(source, pos, length)) {
                if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) {
                    int next = STATE_IDENTIFIER;
                    char letter_buf[3] = {source[pos], source[pos + 1], '\0'};
                    write_trace_str(log, "SCAN", line, column, letter_buf, "LETTER");
                    write_transition_str(log, line, column, state, letter_buf, next);
                    state = next;
                    pos += 2;
                    column += 1;
                } else {
                    int current = (unsigned char)source[pos];
                    int next = dfa_step(state, current);
                    if (next < 0) {
                        next = STATE_IDENTIFIER;
                    }
                    write_trace(log, "SCAN", line, column, current,
                                isdigit((unsigned char)current) ? "DIGIT" :
                                current == '_' ? "UNDERSCORE" : "LETTER");
                    write_transition(log, line, column, state, current, next);
                    state = next;
                    pos += 1;
                    column += 1;
                }
            }
            {
                const AcceptingState *accepted = dfa_find_accept(state);
                size_t token_length = pos - start;
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column,
                                 "Out of memory allocating identifier token.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                if (accepted == NULL) {
                    dfa_write_symbol(tokens, TOKEN_KIND_IDENTIFIER, lexeme);
                    dfa_write_acceptance(log, lexeme, TOKEN_KIND_IDENTIFIER,
                                           "IDENTIFIER", state);
                } else {
                    dfa_write_symbol(tokens, accepted->kind, lexeme);
                    dfa_write_acceptance(log, lexeme, accepted->kind,
                                           accepted->classification, state);
                }
                free(lexeme);
            }
            continue;
        }

        if (isdigit((unsigned char)c)) {
            size_t start = pos;
            int state = STATE_START;
            while (pos < length &&
                   isdigit((unsigned char)source[pos])) {
                int current = (unsigned char)source[pos];
                int next = dfa_step(state, current);
                if (next < 0) {
                    next = STATE_INTEGER;
                }
                write_trace(log, "SCAN", line, column, current, "DIGIT");
                write_transition(log, line, column, state, current, next);
                state = next;
                ++pos;
                ++column;
            }
            if (pos + 1 < length && source[pos] == '.' &&
                isdigit((unsigned char)source[pos + 1])) {
                int next = dfa_step(state, '.');
                write_trace(log, "SCAN", line, column, '.', "DECIMAL_POINT");
                write_transition(log, line, column, state, '.', next);
                state = next;
                ++pos;
                ++column;
                while (pos < length &&
                       isdigit((unsigned char)source[pos])) {
                    int current = (unsigned char)source[pos];
                    next = dfa_step(state, current);
                    write_trace(log, "SCAN", line, column, current, "DIGIT");
                    write_transition(log, line, column, state, current, next);
                    state = next;
                    ++pos;
                    ++column;
                }
            }
            {
                size_t token_length = pos - start;
                const AcceptingState *accepted = dfa_find_accept(state);
                if (accepted == NULL) {
                    dfa_report_error(tokens, token_line, token_column,
                                 "Invalid numeric literal.");
                    ++errors;
                    continue;
                }
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column,
                                 "Out of memory allocating numeric literal.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                dfa_write_symbol(tokens, accepted->kind, lexeme);
                dfa_write_acceptance(log, lexeme, accepted->kind,
                                 accepted->classification, state);
                free(lexeme);
            }
            continue;
        }

        if (c == '\'') {
            size_t start = pos;
            int state = STATE_START;
            int next = STATE_CHAR;
            write_trace(log, "SCAN", line, column, c, "CHAR_DELIMITER");
            write_transition(log, line, column, state, c, next);
            state = next;
            ++pos;
            ++column;

            if (pos >= length || source[pos] == '\n' || source[pos] == '\r') {
                dfa_report_error(tokens, token_line, token_column,
                             "Unterminated character literal.");
                ++errors;
                continue;
            }

            if (source[pos] == '\'') {
                write_trace(log, "SCAN", line, column, '\'', "CHAR_DELIMITER");
                write_transition(log, line, column, state, '\'', STATE_ERROR);
                dfa_report_error(tokens, token_line, token_column,
                             "Empty character literal is not allowed.");
                ++errors;
                ++pos;
                ++column;
                continue;
            }

            if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) {
                char letter_buf[3] = {source[pos], source[pos + 1], '\0'};
                write_trace_str(log, "SCAN", line, column, letter_buf, "CHAR_CONTENT");
                write_transition_str(log, line, column, state, letter_buf, state);
                pos += 2;
                column += 1;
            } else {
                int ch = (unsigned char)source[pos];
                write_trace(log, "SCAN", line, column, ch, "CHAR_CONTENT");
                write_transition(log, line, column, state, ch, state);
                pos += 1;
                column += 1;
            }

            if (pos < length && source[pos] == '\'') {
                write_trace(log, "SCAN", line, column, '\'', "CHAR_DELIMITER");
                write_transition(log, line, column, state, '\'', STATE_START);
                ++pos;
                ++column;

                size_t token_length = pos - start;
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column,
                                 "Out of memory allocating character literal.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                dfa_write_symbol(tokens, TOKEN_KIND_LETRA_LITERAL, lexeme);
                dfa_write_acceptance(log, lexeme, TOKEN_KIND_LETRA_LITERAL,
                                 "CHARACTER_LITERAL", state);
                free(lexeme);
            } else {
                while (pos < length && source[pos] != '\'' && source[pos] != '\n') {
                    ++pos;
                    ++column;
                }
                if (pos < length && source[pos] == '\'') {
                    ++pos;
                    ++column;
                    dfa_report_error(tokens, token_line, token_column,
                                 "Character literal may only contain a single character.");
                } else {
                    dfa_report_error(tokens, token_line, token_column,
                                 "Unterminated character literal.");
                }
                ++errors;
            }
            continue;
        }

        if (c == '"') {
            size_t start = pos;
            int state = dfa_step(STATE_START, c);
            int terminated = 0;
            write_trace(log, "SCAN", line, column, c, "STRING_DELIMITER");
            write_transition(log, line, column, STATE_START, c, state);
            ++pos;
            ++column;
            while (pos < length && source[pos] != '\n') {
                if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) {
                    char letter_buf[3] = {source[pos], source[pos + 1], '\0'};
                    write_trace_str(log, "SCAN", line, column, letter_buf, "STRING_CHARACTER");
                    write_transition_str(log, line, column, state, letter_buf, state);
                    pos += 2;
                    column += 1;
                } else {
                    int current = (unsigned char)source[pos];
                    int next = dfa_step(state, current);
                    write_trace(log, "SCAN", line, column, current,
                                current == '"' ? "STRING_DELIMITER" :
                                                 "STRING_CHARACTER");
                    write_transition(log, line, column, state, current, next);
                    state = next;
                    ++pos;
                    ++column;
                    if (current == '"') {
                        terminated = 1;
                        break;
                    }
                }
            }
            if (!terminated) {
                dfa_report_error(tokens, token_line, token_column,
                             "Unterminated string literal.");
                ++errors;
                continue;
            }
            {
                size_t token_length = pos - start;
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column,
                                 "Out of memory allocating string literal.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                dfa_write_symbol(tokens, TOKEN_KIND_SALITA_LITERAL, lexeme);
                dfa_write_acceptance(log, lexeme, TOKEN_KIND_SALITA_LITERAL,
                                 "STRING_LITERAL", state);
                free(lexeme);
            }
            continue;
        }

        {
            int state = dfa_step(STATE_START, c);
            int next_state = state;
            size_t token_length = 1;
            char lexeme[3] = {(char)c, '\0', '\0'};
            const AcceptingState *accepted;

            write_trace(log, "SCAN", line, column, c, "SYMBOL");
            if (state < 0) {
                state = STATE_ERROR;
            }
            if (state == STATE_ERROR) {
                write_transition(log, line, column, STATE_START, c, state);
                dfa_report_error(tokens, token_line, token_column,
                             "Unrecognized character.");
                ++errors;
                ++pos;
                ++column;
                continue;
            }
            write_transition(log, line, column, STATE_START, c, state);
            if (pos + 1 < length) {
                int second = (unsigned char)source[pos + 1];
                int combined = dfa_step(state, second);
                if (combined >= 0 && dfa_find_accept(combined) != NULL) {
                    write_trace(log, "SCAN", line, column + 1, second,
                                "SYMBOL");
                    write_transition(log, line, column + 1, state, second,
                                     combined);
                    next_state = combined;
                    lexeme[1] = (char)second;
                    token_length = 2;
                }
            }
            accepted = dfa_find_accept(next_state);
            if (accepted == NULL) {
                dfa_report_error(tokens, token_line, token_column,
                             "Invalid symbol transition.");
                if (log != NULL) {
                    fprintf(log, "REJECT | [%s] | State: q%d | No accepting token\n",
                            lexeme, next_state);
                }
                ++errors;
            } else {
                dfa_write_symbol(tokens, accepted->kind, lexeme);
                dfa_write_acceptance(log, lexeme, accepted->kind,
                                 accepted->classification, next_state);
            }
            pos += token_length;
            column += token_length;
        }
    }
    return errors;
}
