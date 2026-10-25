#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_TRANSITIONS 1024
#define MAX_ACCEPTS 128
#define MAX_TOKEN_LENGTH 4096
#define SYMBOL_TABLE_FILE "Symbol Table.txt"

enum {
    CLASS_ALPHA = 256,
    CLASS_DIGIT,
    CLASS_UNDERSCORE,
    CLASS_WHITESPACE,
    CLASS_ANY
};

enum {
    STATE_START = 0,
    STATE_IDENTIFIER = 900,
    STATE_INTEGER = 901,
    STATE_DECIMAL = 902,
    STATE_STRING = 903,
    STATE_COMMENT = 904,
    STATE_ERROR = 905,
    STATE_ASSIGN = 910,
    STATE_NOT = 911,
    STATE_GREATER = 912,
    STATE_LESS = 913,
    STATE_AMPERSAND = 914,
    STATE_PIPE = 915
};

typedef struct {
    int from;
    int input;
    int to;
} Transition;

typedef struct {
    int state;
    char token_type[40];
    char classification[32];
    int ignored;
} AcceptingState;

typedef struct {
    const char *lexeme;
    const char *token_type;
    const char *classification;
    int ignored;
} WordDefinition;

static Transition transitions[MAX_TRANSITIONS];
static size_t transition_count;
static AcceptingState accepting_states[MAX_ACCEPTS];
static size_t accepting_count;
static int next_word_state = 1;
static int next_symbol_state = 920;

static const WordDefinition words[] = {
    {"LETRA", "TOKEN_KW_LETRA", "KEYWORD", 0},
    {"SALITA", "TOKEN_KW_SALITA", "KEYWORD", 0},
    {"BUO", "TOKEN_KW_BUO", "KEYWORD", 0},
    {"NUMERO", "TOKEN_KW_NUMERO", "KEYWORD", 0},
    {"KUNDISYON", "TOKEN_KW_KUNDISYON", "KEYWORD", 0},
    {"KUNG", "TOKEN_KW_KUNG", "KEYWORD", 0},
    {"KUNDI", "TOKEN_KW_KUNDI", "KEYWORD", 0},
    {"ULIT", "TOKEN_KW_ULIT", "KEYWORD", 0},
    {"HINTO", "TOKEN_KW_HINTO", "KEYWORD", 0},
    {"TULOY", "TOKEN_KW_TULOY", "KEYWORD", 0},
    {"TAMA", "TOKEN_RES_TAMA", "RESERVED_WORD", 0},
    {"MALI", "TOKEN_RES_MALI", "RESERVED_WORD", 0},
    {"KATUMBAS", "TOKEN_RES_KATUMBAS", "RESERVED_WORD", 0},
    {"ULAM", "TOKEN_RES_ULAM", "BUILT_IN_OBJECT", 0},
    {"ilagay", "TOKEN_RES_ilagay", "BUILT_IN_METHOD", 0},
    {"meron", "TOKEN_RES_meron", "BUILT_IN_METHOD", 0},
    {"kumuha", "TOKEN_RES_kumuha", "BUILT_IN_METHOD", 0},
    {"RESIBO", "TOKEN_RES_RESIBO", "BUILT_IN_OBJECT", 0},
    {"dagdag", "TOKEN_RES_dagdag", "BUILT_IN_METHOD", 0},
    {"bawas", "TOKEN_RES_bawas", "BUILT_IN_METHOD", 0},
    {"kalkula", "TOKEN_RES_kalkula", "BUILT_IN_METHOD", 0},
    {"diskwento", "TOKEN_RES_diskwento", "BUILT_IN_METHOD", 0},
    {"bayaran", "TOKEN_RES_bayaran", "BUILT_IN_METHOD", 0},
    {"ilimbag", "TOKEN_RES_ilimbag", "BUILT_IN_METHOD", 0},
    {"linisin", "TOKEN_RES_linisin", "BUILT_IN_METHOD", 0},
    {"hatian", "TOKEN_RES_hatian", "BUILT_IN_METHOD", 0},
    {"ORDER", "TOKEN_RES_ORDER", "TRANSACTION_VERB", 0},
    {"LUTO", "TOKEN_RES_LUTO", "TRANSACTION_VERB", 0},
    {"HAIN", "TOKEN_RES_HAIN", "TRANSACTION_VERB", 0},
    {"BAYAD", "TOKEN_RES_BAYAD", "TRANSACTION_VERB", 0},
    {"po", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"na", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"lang", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"at", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"nang", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"ng", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"ang", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"ay", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"nito", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1},
    {"opo", "TOKEN_NOISE_WORD", "IGNORED_NOISE_WORD", 1}
};

static const WordDefinition symbols[] = {
    {".", "TOKEN_TERMINATOR", "PUNCTUATION", 0},
    {",", "TOKEN_SEPARATOR", "PUNCTUATION", 0},
    {";", "TOKEN_SEMICOLON", "PUNCTUATION", 0},
    {":", "TOKEN_ACCESSOR", "OPERATOR", 0},
    {"(", "TOKEN_LPAREN", "DELIMITER", 0},
    {")", "TOKEN_RPAREN", "DELIMITER", 0},
    {"{", "TOKEN_LBRACE", "DELIMITER", 0},
    {"}", "TOKEN_RBRACE", "DELIMITER", 0},
    {"#", "TOKEN_SPECIAL_SYMBOL", "SPECIAL_SYMBOL", 0},
    {"$", "TOKEN_SPECIAL_SYMBOL", "SPECIAL_SYMBOL", 0},
    {"+", "TOKEN_OP_ADD", "ARITHMETIC_OPERATOR", 0},
    {"-", "TOKEN_OP_SUB", "ARITHMETIC_OPERATOR", 0},
    {"*", "TOKEN_OP_MUL", "ARITHMETIC_OPERATOR", 0},
    {"/", "TOKEN_OP_DIV", "ARITHMETIC_OPERATOR", 0},
    {"%", "TOKEN_OP_MOD", "ARITHMETIC_OPERATOR", 0},
    {"\\", "TOKEN_OP_INTDIV", "ARITHMETIC_OPERATOR", 0},
    {"^", "TOKEN_OP_POW", "ARITHMETIC_OPERATOR", 0},
    {"=", "TOKEN_ASSIGNMENT", "OPERATOR", 0},
    {"==", "TOKEN_REL_EQ", "RELATIONAL_OPERATOR", 0},
    {"!=", "TOKEN_REL_NEQ", "RELATIONAL_OPERATOR", 0},
    {">", "TOKEN_REL_GT", "RELATIONAL_OPERATOR", 0},
    {">=", "TOKEN_REL_GTE", "RELATIONAL_OPERATOR", 0},
    {"<", "TOKEN_REL_LT", "RELATIONAL_OPERATOR", 0},
    {"<=", "TOKEN_REL_LTE", "RELATIONAL_OPERATOR", 0},
    {"!", "TOKEN_LOGIC_NOT", "LOGICAL_OPERATOR", 0},
    {"&&", "TOKEN_LOGIC_AND", "LOGICAL_OPERATOR", 0},
    {"||", "TOKEN_LOGIC_OR", "LOGICAL_OPERATOR", 0}
};

static int is_identifier_start(int c)
{
    return isalpha((unsigned char)c) || c == '_';
}

static int is_identifier_part(int c)
{
    return isalnum((unsigned char)c) || c == '_';
}

static int is_whitespace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static int add_transition(int from, int input, int to)
{
    if (transition_count >= MAX_TRANSITIONS) {
        return 0;
    }
    transitions[transition_count++] = (Transition){from, input, to};
    return 1;
}

static int find_transition(int from, int input)
{
    size_t i;
    for (i = 0; i < transition_count; ++i) {
        if (transitions[i].from == from && transitions[i].input == input) {
            return transitions[i].to;
        }
    }
    return -1;
}

static int add_accept(int state, const char *type,
                      const char *classification, int ignored)
{
    AcceptingState *entry;
    if (accepting_count >= MAX_ACCEPTS) {
        return 0;
    }
    entry = &accepting_states[accepting_count++];
    entry->state = state;
    snprintf(entry->token_type, sizeof entry->token_type, "%s", type);
    snprintf(entry->classification, sizeof entry->classification, "%s",
             classification);
    entry->ignored = ignored;
    return 1;
}

static const AcceptingState *find_accept(int state)
{
    size_t i;
    for (i = 0; i < accepting_count; ++i) {
        if (accepting_states[i].state == state) {
            return &accepting_states[i];
        }
    }
    return NULL;
}

static int insert_word(const WordDefinition *word, int *created_states,
                       size_t *created_count, size_t created_capacity)
{
    int state = STATE_START;
    const unsigned char *p = (const unsigned char *)word->lexeme;
    while (*p != '\0') {
        int next = find_transition(state, *p);
        if (next < 0) {
            if (*created_count >= created_capacity) {
                return 0;
            }
            next = next_word_state++;
            if (!add_transition(state, *p, next)) {
                return 0;
            }
            created_states[(*created_count)++] = next;
        }
        state = next;
        ++p;
    }
    return add_accept(state, word->token_type, word->classification,
                      word->ignored);
}

static int insert_symbol(const WordDefinition *symbol)
{
    int state = STATE_START;
    const unsigned char *p = (const unsigned char *)symbol->lexeme;
    while (*p != '\0') {
        int next = find_transition(state, *p);
        if (next < 0) {
            next = next_symbol_state++;
            if (!add_transition(state, *p, next)) {
                return 0;
            }
        }
        state = next;
        ++p;
    }
    return add_accept(state, symbol->token_type, symbol->classification, 0);
}

static int initialize_automaton(void)
{
    int word_states[MAX_TRANSITIONS];
    size_t word_state_count = 0;
    size_t i;

    for (i = 0; i < sizeof words / sizeof words[0]; ++i) {
        if (!insert_word(&words[i], word_states, &word_state_count,
                         MAX_TRANSITIONS)) {
            return 0;
        }
    }

    for (i = 0; i < sizeof symbols / sizeof symbols[0]; ++i) {
        if (!insert_symbol(&symbols[i])) {
            return 0;
        }
    }

    if (!add_transition(STATE_START, CLASS_ALPHA, STATE_IDENTIFIER) ||
        !add_transition(STATE_START, CLASS_UNDERSCORE, STATE_IDENTIFIER) ||
        !add_transition(STATE_IDENTIFIER, CLASS_ALPHA, STATE_IDENTIFIER) ||
        !add_transition(STATE_IDENTIFIER, CLASS_DIGIT, STATE_IDENTIFIER) ||
        !add_transition(STATE_IDENTIFIER, CLASS_UNDERSCORE,
                        STATE_IDENTIFIER) ||
        !add_transition(STATE_START, CLASS_DIGIT, STATE_INTEGER) ||
        !add_transition(STATE_INTEGER, CLASS_DIGIT, STATE_INTEGER) ||
        !add_transition(STATE_INTEGER, '.', STATE_DECIMAL) ||
        !add_transition(STATE_DECIMAL, CLASS_DIGIT, STATE_DECIMAL) ||
        !add_transition(STATE_START, CLASS_WHITESPACE, STATE_START) ||
        !add_transition(STATE_START, CLASS_ANY, STATE_ERROR) ||
        !add_transition(STATE_START, '"', STATE_STRING) ||
        !add_transition(STATE_START, '@', STATE_COMMENT) ||
        !add_transition(STATE_COMMENT, CLASS_ANY, STATE_COMMENT) ||
        !add_transition(STATE_STRING, '"', STATE_START) ||
        !add_transition(STATE_STRING, CLASS_ANY, STATE_STRING)) {
        return 0;
    }

    for (i = 0; i < word_state_count; ++i) {
        if (!add_transition(word_states[i], CLASS_ALPHA, STATE_IDENTIFIER) ||
            !add_transition(word_states[i], CLASS_DIGIT, STATE_IDENTIFIER) ||
            !add_transition(word_states[i], CLASS_UNDERSCORE,
                            STATE_IDENTIFIER)) {
            return 0;
        }
    }

    return add_accept(STATE_IDENTIFIER, "TOKEN_IDENTIFIER", "IDENTIFIER", 0) &&
           add_accept(STATE_INTEGER, "TOKEN_BUO_LITERAL", "INTEGER_LITERAL",
                      0) &&
           add_accept(STATE_DECIMAL, "TOKEN_NUMERO_LITERAL",
                      "DECIMAL_LITERAL", 0);
}

static int dfa_step(int state, int c)
{
    int exact = find_transition(state, c);
    int input_class;
    int transition;

    if (exact >= 0) {
        return exact;
    }
    if (state == STATE_COMMENT) {
        return find_transition(state, CLASS_ANY);
    }
    if (state == STATE_STRING) {
        return find_transition(state, CLASS_ANY);
    }
    if (isalpha((unsigned char)c)) {
        input_class = CLASS_ALPHA;
    } else if (isdigit((unsigned char)c)) {
        input_class = CLASS_DIGIT;
    } else if (c == '_') {
        input_class = CLASS_UNDERSCORE;
    } else if (is_whitespace(c)) {
        input_class = CLASS_WHITESPACE;
    } else {
        return find_transition(state, CLASS_ANY);
    }
    transition = find_transition(state, input_class);
    return transition;
}

static void write_trace(FILE *out, const char *kind, size_t line,
                        size_t column, int c, const char *classification)
{
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

static void write_symbol(FILE *out, size_t line, size_t column,
                         const char *type, const char *classification,
                         const char *lexeme)
{
    const char *cursor;
    size_t displayed_width = 0;
    fprintf(out, "| %-4zu | %-6zu | ", line, column);
    for (cursor = lexeme; *cursor != '\0'; ++cursor) {
        if (*cursor == '|') {
            fputs("\\|", out);
            displayed_width += 2;
        } else {
            fputc(*cursor, out);
            ++displayed_width;
        }
    }
    while (displayed_width < 40) {
        fputc(' ', out);
        ++displayed_width;
    }
    fprintf(out, " | %-28s | %-22s |\n", type, classification);
}

static void report_error(FILE *out, size_t line, size_t column,
                         const char *message)
{
    fprintf(stderr, "Lexer error at line %zu, column %zu: %s\n",
            line, column, message);
    fprintf(out, "ERROR | Line %zu, Column %zu | %s\n",
            line, column, message);
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

static int read_source(const char *path, char **source, size_t *length)
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

static int scan_source(const char *source, size_t length, FILE *log,
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

        if (is_whitespace(c)) {
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
            int state = STATE_START;
            while (pos < length && source[pos] != '\n') {
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
            continue;
        }

        if (is_identifier_start(c)) {
            size_t start = pos;
            int state = STATE_START;
            while (pos < length &&
                   is_identifier_part((unsigned char)source[pos])) {
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
                ++pos;
                ++column;
            }
            {
                const AcceptingState *accepted = find_accept(state);
                char lexeme[MAX_TOKEN_LENGTH];
                size_t token_length = pos - start;
                if (token_length >= sizeof lexeme) {
                    report_error(log, token_line, token_column,
                                 "Identifier exceeds maximum supported length.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                if (accepted == NULL) {
                    write_symbol(tokens, token_line, token_column,
                                 "TOKEN_IDENTIFIER", "IDENTIFIER", lexeme);
                } else if (accepted->ignored) {
                    write_symbol(tokens, token_line, token_column,
                                 accepted->token_type,
                                 accepted->classification, lexeme);
                } else {
                    write_symbol(tokens, token_line, token_column,
                                 accepted->token_type,
                                 accepted->classification, lexeme);
                }
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
                char lexeme[MAX_TOKEN_LENGTH];
                size_t token_length = pos - start;
                const AcceptingState *accepted = find_accept(state);
                if (token_length >= sizeof lexeme || accepted == NULL) {
                    report_error(log, token_line, token_column,
                                 "Invalid or oversized numeric literal.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                write_symbol(tokens, token_line, token_column,
                             accepted->token_type, accepted->classification,
                             lexeme);
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
            if (!terminated) {
                report_error(log, token_line, token_column,
                             "Unterminated string literal.");
                ++errors;
                continue;
            }
            {
                char lexeme[MAX_TOKEN_LENGTH];
                size_t token_length = pos - start;
                if (token_length >= sizeof lexeme) {
                    report_error(log, token_line, token_column,
                                 "String literal exceeds maximum supported length.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                write_symbol(tokens, token_line, token_column,
                             "TOKEN_SALITA_LITERAL", "STRING_LITERAL",
                             lexeme);
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
                report_error(log, token_line, token_column,
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
                if (combined >= 0 && find_accept(combined) != NULL) {
                    write_trace(log, "SCAN", line, column + 1, second,
                                "SYMBOL");
                    write_transition(log, line, column + 1, state, second,
                                     combined);
                    next_state = combined;
                    lexeme[1] = (char)second;
                    token_length = 2;
                }
            }
            accepted = find_accept(next_state);
            if (accepted == NULL) {
                report_error(log, token_line, token_column,
                             "Invalid symbol transition.");
                ++errors;
            } else {
                write_symbol(tokens, token_line, token_column,
                             accepted->token_type, accepted->classification,
                             lexeme);
            }
            pos += token_length;
            column += token_length;
        }
    }
    return errors;
}

static int copy_stream(FILE *source, FILE *destination)
{
    char buffer[4096];
    size_t bytes_read;

    if (fseek(source, 0, SEEK_SET) != 0) {
        return 0;
    }
    while ((bytes_read = fread(buffer, 1, sizeof buffer, source)) > 0) {
        if (fwrite(buffer, 1, bytes_read, destination) != bytes_read) {
            return 0;
        }
    }
    return !ferror(source);
}

int main(int argc, char **argv)
{
    char *source = NULL;
    size_t source_length = 0;
    FILE *symbol_table;
    FILE *scan_log;
    FILE *token_table;
    int errors;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.rice>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (!initialize_automaton()) {
        fprintf(stderr, "Failed to initialize the lexical DFA.\n");
        return EXIT_FAILURE;
    }
    if (!read_source(argv[1], &source, &source_length)) {
        return EXIT_FAILURE;
    }
    scan_log = tmpfile();
    token_table = tmpfile();
    if (scan_log == NULL || token_table == NULL) {
        fprintf(stderr, "Cannot create temporary output files: %s\n",
                strerror(errno));
        if (scan_log != NULL) {
            fclose(scan_log);
        }
        if (token_table != NULL) {
            fclose(token_table);
        }
        free(source);
        return EXIT_FAILURE;
    }
    errors = scan_source(source, source_length, scan_log, token_table);
    free(source);
    if (fflush(scan_log) != 0 || fflush(token_table) != 0) {
        fprintf(stderr, "Error buffering temporary lexer output.\n");
        fclose(scan_log);
        fclose(token_table);
        return EXIT_FAILURE;
    }
    symbol_table = fopen(SYMBOL_TABLE_FILE, "w");
    if (symbol_table == NULL) {
        fprintf(stderr, "Cannot create '%s': %s\n",
                SYMBOL_TABLE_FILE, strerror(errno));
        fclose(scan_log);
        fclose(token_table);
        return EXIT_FAILURE;
    }
    fprintf(symbol_table, "JAVArice lexical analysis symbol table\n");
    fprintf(symbol_table, "Input: %s\n\n", argv[1]);
    fprintf(symbol_table,
            "TOKENS\n"
            "| Line | Column | Lexeme                                   | Token                        | Category               |\n"
            "|------|--------|------------------------------------------|------------------------------|------------------------|\n");
    if (!copy_stream(token_table, symbol_table)) {
        fprintf(stderr, "Error writing token table to '%s'.\n",
                SYMBOL_TABLE_FILE);
        fclose(scan_log);
        fclose(token_table);
        fclose(symbol_table);
        return EXIT_FAILURE;
    }
    fprintf(symbol_table, "\nCHARACTER-BY-CHARACTER SCAN AND DFA LOG\n");
    if (!copy_stream(scan_log, symbol_table)) {
        fprintf(stderr, "Error writing scan log to '%s'.\n",
                SYMBOL_TABLE_FILE);
        fclose(scan_log);
        fclose(token_table);
        fclose(symbol_table);
        return EXIT_FAILURE;
    }
    {
        int close_error = 0;
        if (fclose(scan_log) != 0) {
            close_error = 1;
        }
        if (fclose(token_table) != 0) {
            close_error = 1;
        }
        if (close_error) {
            fprintf(stderr, "Error closing temporary output files.\n");
            fclose(symbol_table);
            return EXIT_FAILURE;
        }
    }
    if (fclose(symbol_table) != 0) {
        fprintf(stderr, "Error writing '%s'.\n", SYMBOL_TABLE_FILE);
        return EXIT_FAILURE;
    }
    if (errors != 0) {
        fprintf(stderr, "Lexical analysis completed with %d error(s). "
                        "See %s.\n", errors, SYMBOL_TABLE_FILE);
        return EXIT_FAILURE;
    }
    printf("Lexical analysis completed successfully. "
           "Symbol table written to %s.\n", SYMBOL_TABLE_FILE);
    return EXIT_SUCCESS;
}
