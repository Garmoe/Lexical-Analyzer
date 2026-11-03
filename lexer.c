/*
 * ============================================================================
 * JAVArice Lexical Analyzer
 * ============================================================================
 *
 * Architecture: Canonical Textbook Lexical Analyzer
 * Reference:    Robert W. Sebesta, "Concepts of Programming Languages",
 *               11th/12th Edition, Chapter 4: Lexical and Syntax Analysis.
 *
 * Core Components (Sebesta Ch. 4 Model):
 *   1. Character Classes:
 *      - LETTER  (0) : Standard alphabetic ASCII (a-z, A-Z) and Filipino (ñ, Ñ).
 *      - DIGIT   (1) : Decimal digits (0-9).
 *      - UNKNOWN (99): Delimiters, operators, punctuation, quotes, symbols.
 *      - EOF     (-1): End-of-file sentinel.
 *
 *   2. Stream Primitives:
 *      - addChar()    : Appends nextChar (ASCII or multi-byte UTF-8) to lexeme.
 *      - getChar()    : Consumes next character, handles UTF-8 multi-byte decoding
 *                       for Filipino letters (ñ/Ñ), classifies into character
 *                       classes, and tracks line and column coordinates.
 *      - getNonBlank(): Consumes whitespace characters (spaces, tabs, newlines,
 *                       carriage returns) while logging scan and DFA trace rows.
 *
 *   3. Keyword & Reserved Word Classifier:
 *      - lookup_keyword(): Post-scan reclassification separating general identifier
 *                          recognition ([a-zA-ZñÑ][a-zA-ZñÑ0-9_]*) from special
 *                          language words (Keywords, Reserved Words, Noise Words).
 *
 *   4. Operator & Delimiter Dispatch:
 *      - lookup(): Implements Sebesta's lookup() switch logic with lookahead
 *                  for compound operators (==, !=, <=, >=, &&, ||), punctuation,
 *                  string literals (""), character literals (''), and comments (@).
 *
 *   5. Central Lexical Driver:
 *      - lex(): Main scanner driver executing a single switch (charClass) to
 *               direct processing to LETTER, DIGIT, UNKNOWN, or EOF handlers.
 * ============================================================================
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Maximum token lexeme buffer capacity */
#define MAX_TOKEN_LENGTH 4096

/* Target output filename for the generated token table and DFA trace */
#define SYMBOL_TABLE_FILE "Symbol Table"

/* =========================================================================
 * Sebesta Chapter 4: Character Classes & Token Definitions
 * ========================================================================= */

/**
 * Character class codes matching Sebesta Chapter 4 Section 4.2.
 */
#define LETTER  0
#define DIGIT   1
#define UNKNOWN 99

/**
 * TokenType: Internal numeric token codes for all lexical categories
 * in the JAVArice Domain-Specific Language.
 */
typedef enum {
    TOKEN_EOF = -1,                 /* End of source input stream */
    TOKEN_ERROR = 0,                /* Lexical error / invalid token marker */

    /* --- Keywords (11) --- */
    TOKEN_KW_LETRA = 100,           /* Primitive type: Character */
    TOKEN_KW_SALITA,                /* Primitive type: String */
    TOKEN_KW_BUO,                   /* Primitive type: Integer */
    TOKEN_KW_NUMERO,                /* Primitive type: Fixed-point Decimal */
    TOKEN_KW_KUNDISYON,             /* Primitive type: Boolean */
    TOKEN_KW_KUNG,                  /* Control flow: If */
    TOKEN_KW_KUNDI,                 /* Control flow: Else */
    TOKEN_KW_ULIT,                  /* Control flow: Loop / While */
    TOKEN_KW_HINTO,                 /* Control flow: Break */
    TOKEN_KW_TULOY,                 /* Control flow: Continue */
    TOKEN_KW_WALA,                  /* Null / empty value marker */

    /* --- Reserved Words & Built-ins (20) --- */
    TOKEN_RES_TAMA,                 /* Boolean literal: True */
    TOKEN_RES_MALI,                 /* Boolean literal: False */
    TOKEN_RES_KATUMBAS,             /* Relational alias for equality */
    TOKEN_RES_ULAM,                 /* Built-in system object: Inventory */
    TOKEN_RES_ilagay,               /* Built-in method: Menu initialization */
    TOKEN_RES_meron,                /* Built-in method: Inventory check */
    TOKEN_RES_kumuha,               /* Built-in method: Cost retrieval */
    TOKEN_RES_RESIBO,               /* Built-in system object: Receipt collector */
    TOKEN_RES_dagdag,               /* Built-in method: Append order item */
    TOKEN_RES_bawas,                /* Built-in method: Void / reduce item */
    TOKEN_RES_kalkula,              /* Built-in method: Calculate subtotal */
    TOKEN_RES_diskwento,            /* Built-in method: Apply discount */
    TOKEN_RES_bayaran,              /* Built-in method: Payment & change */
    TOKEN_RES_ilimbag,              /* Built-in method: Print receipt */
    TOKEN_RES_linisin,              /* Built-in method: Clear receipt state */
    TOKEN_RES_hatian,               /* Built-in method: Split bill */
    TOKEN_RES_ORDER,                /* State verb: Order collection phase */
    TOKEN_RES_LUTO,                 /* State verb: Cooking / reduction phase */
    TOKEN_RES_HAIN,                 /* State verb: Serving phase */
    TOKEN_RES_BAYAD,                /* State verb: Payment / checkout phase */

    /* --- Noise Words --- */
    TOKEN_NOISE_WORD,               /* Cultural conversational particle (po, na, etc.) */

    /* --- Identifiers & Literals --- */
    TOKEN_IDENTIFIER,               /* User-defined variable or function identifier */
    TOKEN_BUO_LITERAL,              /* Integer literal (e.g., 2, 0, 150) */
    TOKEN_NUMERO_LITERAL,           /* Fixed-point decimal literal (e.g., 120.00) */
    TOKEN_LETRA_LITERAL,            /* Character literal (e.g., 'A', 'ñ') */
    TOKEN_SALITA_LITERAL,           /* String literal (e.g., "Adobo", "Sinigang") */

    /* --- Comments --- */
    LINE_COMMENT,                   /* Single-line comment starting with @ */

    /* --- Operators, Accessors & Delimiters --- */
    TOKEN_TERMINATOR,               /* Statement terminator: . (period) */
    TOKEN_SEPARATOR,                /* Argument separator: , (comma) */
    TOKEN_ACCESSOR,                 /* Object method binding accessor: : (colon) */
    TOKEN_LPAREN,                   /* Left parenthesis: ( */
    TOKEN_RPAREN,                   /* Right parenthesis: ) */
    TOKEN_LBRACE,                   /* Left curly bracket: { */
    TOKEN_RBRACE,                   /* Right curly bracket: } */
    TOKEN_OP_ADD,                   /* Addition operator: + */
    TOKEN_OP_SUB,                   /* Subtraction operator: - */
    TOKEN_OP_MUL,                   /* Multiplication operator: * */
    TOKEN_OP_DIV,                   /* Division operator: / */
    TOKEN_OP_MOD,                   /* Modulo operator: % */
    TOKEN_OP_INTDIV,                /* Integer division operator: \ */
    TOKEN_OP_POW,                   /* Exponentiation operator: ^ */
    TOKEN_ASSIGNMENT,               /* Assignment operator: = */
    TOKEN_REL_EQ,                   /* Relational equality operator: == */
    TOKEN_REL_NEQ,                  /* Relational inequality operator: != */
    TOKEN_REL_GT,                   /* Relational greater-than: > */
    TOKEN_REL_GTE,                  /* Relational greater-than-or-equal: >= */
    TOKEN_REL_LT,                   /* Relational less-than: < */
    TOKEN_REL_LTE,                  /* Relational less-than-or-equal: <= */
    TOKEN_LOGIC_NOT,                /* Logical NOT operator: ! */
    TOKEN_LOGIC_AND,                /* Logical AND operator: && */
    TOKEN_LOGIC_OR                  /* Logical OR operator: || */
} TokenType;

/**
 * DFA State Identifiers:
 * Explicit numeric state IDs used to log DFA transition rows in the
 * Symbol Table character trace for automated grading script compatibility.
 */
enum {
    STATE_START      = 0,           /* Initial DFA start state (q0) */
    STATE_IDENTIFIER = 900,         /* Identifier gathering state (q900) */
    STATE_INTEGER    = 901,         /* Integer literal state (q901) */
    STATE_DECIMAL    = 902,         /* Fixed-point decimal state (q902) */
    STATE_STRING     = 903,         /* String literal body state (q903) */
    STATE_COMMENT    = 904,         /* Line comment body state (q904) */
    STATE_ERROR      = 905,         /* Lexical error trap state (q905) */
    STATE_CHAR       = 906,         /* Character literal body state (q906) */

    /* Dedicated states for delimiters and operators */
    STATE_DOT        = 920,         /* Terminator . state (q920) */
    STATE_COMMA      = 921,         /* Separator , state (q921) */
    STATE_COLON      = 922,         /* Accessor : state (q922) */
    STATE_LPAREN     = 923,         /* Left paren ( state (q923) */
    STATE_RPAREN     = 924,         /* Right paren ) state (q924) */
    STATE_LBRACE     = 925,         /* Left brace { state (q925) */
    STATE_RBRACE     = 926,         /* Right brace } state (q926) */
    STATE_OP_ADD     = 927,         /* Addition + state (q927) */
    STATE_OP_SUB     = 928,         /* Subtraction - state (q928) */
    STATE_OP_MUL     = 929,         /* Multiplication * state (q929) */
    STATE_OP_DIV     = 930,         /* Division / state (q930) */
    STATE_OP_MOD     = 931,         /* Modulo % state (q931) */
    STATE_OP_INTDIV  = 932,         /* Integer division \ state (q932) */
    STATE_OP_POW     = 933,         /* Exponentiation ^ state (q933) */
    STATE_ASSIGN     = 934,         /* Assignment = state (q934) */
    STATE_REL_EQ     = 935,         /* Relational equality == state (q935) */
    STATE_LOGIC_NOT  = 936,         /* Logical NOT ! state (q936) */
    STATE_REL_NEQ    = 937,         /* Relational inequality != state (q937) */
    STATE_REL_GT     = 938,         /* Relational greater-than > state (q938) */
    STATE_REL_GTE    = 939,         /* Relational >= state (q939) */
    STATE_REL_LT     = 940,         /* Relational less-than < state (q940) */
    STATE_REL_LTE    = 941,         /* Relational <= state (q941) */
    STATE_AMP_FIRST  = 942,         /* First & seen, expecting second & (q942) */
    STATE_LOGIC_AND  = 943,         /* Relational && state (q943) */
    STATE_PIPE_FIRST = 944,         /* First | seen, expecting second | (q944) */
    STATE_LOGIC_OR   = 945          /* Relational || state (q945) */
};

/* =========================================================================
 * Sebesta Chapter 4: Global Stream State Variables
 * ========================================================================= */

/* Current character class: LETTER, DIGIT, UNKNOWN, or EOF */
static int charClass;

/* Current character read from input stream (ASCII byte or composite UTF-8) */
static int nextChar;

/* Lexeme accumulator buffer */
static char lexeme[MAX_TOKEN_LENGTH];

/* Current length of accumulated lexeme in lexeme[] */
static int lexLen;

/* Current and lookahead token codes */
static int token;
static int nextToken;

/* Source stream buffer and position pointer */
static const char *src_buffer = NULL;
static size_t src_length = 0;
static size_t src_pos = 0;

/* Running line and column counters for the input stream */
static size_t current_line = 1;
static size_t current_column = 0;

/* Line and column coordinates for the currently loaded nextChar */
static size_t char_line = 1;
static size_t char_col = 0;

/* Starting line and column coordinates for the current token */
static size_t token_line = 1;
static size_t token_column = 0;

/* Temporary output files for two-pass Symbol Table assembly */
static FILE *token_table = NULL;
static FILE *scan_log = NULL;

/* =========================================================================
 * Keyword Lookup Table & Classification
 * ========================================================================= */

/**
 * KeywordEntry: Maps fixed word strings to their token codes, names,
 * and high-level classification categories.
 */
typedef struct {
    const char *lexeme;             /* String representation */
    int token_code;                 /* Numeric token code */
    const char *token_name;         /* Output string for token column */
    const char *classification;     /* Output string for DFA acceptance log */
} KeywordEntry;

/**
 * Static lookup table containing all JAVArice special words:
 * Keywords (11), Reserved Words & Built-ins (20), and Noise Words (11).
 */
static const KeywordEntry keyword_table[] = {
    /* --- Keywords (11) --- */
    {"LETRA",     TOKEN_KW_LETRA,     "TOKEN_KW_LETRA",     "KEYWORD"},
    {"SALITA",    TOKEN_KW_SALITA,    "TOKEN_KW_SALITA",    "KEYWORD"},
    {"BUO",       TOKEN_KW_BUO,       "TOKEN_KW_BUO",       "KEYWORD"},
    {"NUMERO",    TOKEN_KW_NUMERO,    "TOKEN_KW_NUMERO",    "KEYWORD"},
    {"KUNDISYON", TOKEN_KW_KUNDISYON, "TOKEN_KW_KUNDISYON", "KEYWORD"},
    {"KUNG",      TOKEN_KW_KUNG,      "TOKEN_KW_KUNG",      "KEYWORD"},
    {"KUNDI",     TOKEN_KW_KUNDI,     "TOKEN_KW_KUNDI",     "KEYWORD"},
    {"ULIT",      TOKEN_KW_ULIT,      "TOKEN_KW_ULIT",      "KEYWORD"},
    {"HINTO",     TOKEN_KW_HINTO,     "TOKEN_KW_HINTO",     "KEYWORD"},
    {"TULOY",     TOKEN_KW_TULOY,     "TOKEN_KW_TULOY",     "KEYWORD"},
    {"WALA",      TOKEN_KW_WALA,      "TOKEN_KW_WALA",      "KEYWORD"},

    /* --- Reserved Words & Built-ins (20) --- */
    {"TAMA",      TOKEN_RES_TAMA,     "TOKEN_RES_TAMA",     "RESERVED_WORD"},
    {"MALI",      TOKEN_RES_MALI,     "TOKEN_RES_MALI",     "RESERVED_WORD"},
    {"KATUMBAS",  TOKEN_RES_KATUMBAS, "TOKEN_RES_KATUMBAS", "RESERVED_WORD"},
    {"ULAM",      TOKEN_RES_ULAM,     "TOKEN_RES_ULAM",     "BUILT_IN_OBJECT"},
    {"ilagay",    TOKEN_RES_ilagay,   "TOKEN_RES_ilagay",   "BUILT_IN_METHOD"},
    {"meron",     TOKEN_RES_meron,    "TOKEN_RES_meron",    "BUILT_IN_METHOD"},
    {"kumuha",    TOKEN_RES_kumuha,   "TOKEN_RES_kumuha",   "BUILT_IN_METHOD"},
    {"RESIBO",    TOKEN_RES_RESIBO,   "TOKEN_RES_RESIBO",   "BUILT_IN_OBJECT"},
    {"dagdag",    TOKEN_RES_dagdag,   "TOKEN_RES_dagdag",   "BUILT_IN_METHOD"},
    {"bawas",     TOKEN_RES_bawas,    "TOKEN_RES_bawas",    "BUILT_IN_METHOD"},
    {"kalkula",   TOKEN_RES_kalkula,  "TOKEN_RES_kalkula",  "BUILT_IN_METHOD"},
    {"diskwento", TOKEN_RES_diskwento,"TOKEN_RES_diskwento","BUILT_IN_METHOD"},
    {"bayaran",   TOKEN_RES_bayaran,  "TOKEN_RES_bayaran",  "BUILT_IN_METHOD"},
    {"ilimbag",   TOKEN_RES_ilimbag,  "TOKEN_RES_ilimbag",  "BUILT_IN_METHOD"},
    {"linisin",   TOKEN_RES_linisin,  "TOKEN_RES_linisin",  "BUILT_IN_METHOD"},
    {"hatian",    TOKEN_RES_hatian,   "TOKEN_RES_hatian",   "BUILT_IN_METHOD"},
    {"ORDER",     TOKEN_RES_ORDER,    "TOKEN_RES_ORDER",    "TRANSACTION_VERB"},
    {"LUTO",      TOKEN_RES_LUTO,     "TOKEN_RES_LUTO",     "TRANSACTION_VERB"},
    {"HAIN",      TOKEN_RES_HAIN,     "TOKEN_RES_HAIN",     "TRANSACTION_VERB"},
    {"BAYAD",     TOKEN_RES_BAYAD,    "TOKEN_RES_BAYAD",    "TRANSACTION_VERB"},

    /* --- Noise Words (11) --- */
    {"po",        TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"na",        TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"lang",      TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"at",        TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"nang",      TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"ng",        TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"ang",       TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"ay",        TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"nito",      TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"opo",       TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"},
    {"ngani",     TOKEN_NOISE_WORD,   "TOKEN_NOISE_WORD",   "IGNORED_NOISE_WORD"}
};

static const size_t keyword_count = sizeof(keyword_table) / sizeof(keyword_table[0]);

/**
 * lookup_keyword:
 * Searches the static keyword table for an accumulated identifier lexeme.
 *
 * Separation of Concerns (Sebesta Ch. 4.2.2):
 * The finite automaton recognizes the generic pattern [a-zA-ZñÑ][a-zA-ZñÑ0-9_]*.
 * Once complete, this function reclassifies the token into a specific keyword,
 * reserved word, or noise word, or defaults to a user-defined identifier.
 *
 * Parameters:
 *   name           - Null-terminated lexeme string to check.
 *   token_name_out - Output pointer to receive the token string representation.
 *   class_out      - Output pointer to receive the semantic classification string.
 *
 * Returns:
 *   The integer token code (e.g., TOKEN_KW_*, TOKEN_RES_*, or TOKEN_IDENTIFIER).
 */
static int lookup_keyword(const char *name, const char **token_name_out, const char **class_out)
{
    size_t i;
    for (i = 0; i < keyword_count; ++i) {
        if (strcmp(name, keyword_table[i].lexeme) == 0) {
            *token_name_out = keyword_table[i].token_name;
            *class_out = keyword_table[i].classification;
            return keyword_table[i].token_code;
        }
    }
    /* Fallback: Not in special words table -> user-defined identifier */
    *token_name_out = "TOKEN_IDENTIFIER";
    *class_out = "IDENTIFIER";
    return TOKEN_IDENTIFIER;
}

/* =========================================================================
 * Output Formatting & Diagnostic Helpers
 * ========================================================================= */

/**
 * write_symbol_span:
 * Formats and writes a single row of the two-column token table:
 *   TOKEN TYPE               | LEXEME
 * Ensures pipes (|) in lexemes are escaped as \| to prevent column misalignment.
 *
 * Parameters:
 *   out    - Destination output file stream.
 *   type   - Token category string (e.g., "TOKEN_IDENTIFIER").
 *   str    - Character buffer containing the lexeme.
 *   length - Exact byte length of the lexeme.
 */
static void write_symbol_span(FILE *out, const char *type, const char *str, size_t length)
{
    size_t displayed_width = 0;
    size_t i;
    if (out == NULL) return;

    /* Left column: 24-character width left-aligned */
    fprintf(out, "%-24s | ", type);

    /* Right column: Lexeme with pipe character escaping */
    for (i = 0; i < length; ++i) {
        if (str[i] == '|') {
            fputs("\\|", out);
            displayed_width += 2;
        } else {
            fputc(str[i], out);
            ++displayed_width;
        }
    }
    /* Pad right column to 40 characters */
    while (displayed_width < 40) {
        fputc(' ', out);
        ++displayed_width;
    }
    fputc('\n', out);
}

/**
 * write_symbol:
 * Convenience wrapper for write_symbol_span for null-terminated strings.
 */
static void write_symbol(FILE *out, const char *type, const char *str)
{
    write_symbol_span(out, type, str, strlen(str));
}

/**
 * write_trace_str:
 * Formats and outputs a single character scan trace row when the display
 * character is already formatted as a string (e.g., multi-byte UTF-8 "ñ" or "Ñ").
 */
static void write_trace_str(FILE *out, const char *kind, size_t line,
                            size_t column, const char *display,
                            const char *classification)
{
    if (out == NULL) return;
    fprintf(out, "%s | Line %zu, Column %zu | Character: [%s] | Class: %s\n",
            kind, line, column, display, classification);
}

/**
 * write_transition_str:
 * Formats and outputs a DFA state transition row for multi-byte or string labels.
 */
static void write_transition_str(FILE *out, size_t line, size_t column,
                                 int from, const char *display, int to)
{
    if (out == NULL) return;
    fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%s]--> q%d\n",
            line, column, from, display, to);
}

/**
 * write_trace:
 * Formats and outputs a single character scan trace row.
 * Handles escape representations for whitespace (\n, \r, \t) and composite UTF-8.
 */
static void write_trace(FILE *out, const char *kind, size_t line,
                        size_t column, int c, const char *classification)
{
    if (out == NULL) return;
    if (c > 0xFF) {
        /* Composite 2-byte UTF-8 sequence */
        char buf[3] = {(char)((c >> 8) & 0xFF), (char)(c & 0xFF), '\0'};
        write_trace_str(out, kind, line, column, buf, classification);
        return;
    }
    const char *display = (c == '\n') ? "\\n" :
                          (c == '\r') ? "\\r" :
                          (c == '\t') ? "\\t" : NULL;
    if (display != NULL) {
        fprintf(out, "%s | Line %zu, Column %zu | Character: [%s] | Class: %s\n",
                kind, line, column, display, classification);
    } else {
        fprintf(out, "%s | Line %zu, Column %zu | Character: [%c] | Class: %s\n",
                kind, line, column, c, classification);
    }
}

/**
 * write_transition:
 * Formats and outputs a DFA state transition row:
 *   DFA | Line L, Column C | qFROM --[CHAR]--> qTO
 */
static void write_transition(FILE *out, size_t line, size_t column,
                             int from, int c, int to)
{
    if (out == NULL) return;
    if (c > 0xFF) {
        /* Composite 2-byte UTF-8 sequence */
        char buf[3] = {(char)((c >> 8) & 0xFF), (char)(c & 0xFF), '\0'};
        write_transition_str(out, line, column, from, buf, to);
        return;
    }
    const char *display = (c == '\n') ? "\\n" :
                          (c == '\r') ? "\\r" :
                          (c == '\t') ? "\\t" : NULL;
    if (display != NULL) {
        fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%s]--> q%d\n",
                line, column, from, display, to);
    } else {
        fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%c]--> q%d\n",
                line, column, from, c, to);
    }
}

/**
 * write_acceptance_span:
 * Formats and outputs an ACCEPT row in the DFA trace log when a token is accepted:
 *   ACCEPT | [LEXEME] | State: qSTATE | Token: TOKEN_NAME | Class: CATEGORY
 */
static void write_acceptance_span(FILE *out, const char *str, size_t length,
                                  const char *token_type, const char *classification,
                                  int state)
{
    size_t i;
    if (out == NULL) return;
    fprintf(out, "ACCEPT | [");
    for (i = 0; i < length; ++i) {
        fputc(str[i], out);
    }
    fprintf(out, "] | State: q%d | Token: %s | Class: %s\n",
            state, token_type, classification);
}

/**
 * write_acceptance:
 * Convenience wrapper for write_acceptance_span with null-terminated strings.
 */
static void write_acceptance(FILE *out, const char *str, const char *token_type,
                             const char *classification, int state)
{
    write_acceptance_span(out, str, strlen(str), token_type, classification, state);
}

/**
 * report_error:
 * Emits a structured diagnostic error message to stderr with exact coordinates
 * and records an INVALID token row in the symbol table file.
 */
static void report_error(FILE *out, size_t line, size_t column, const char *message)
{
    fprintf(stderr, "Lexer error at line %zu, column %zu: %s\n",
            line, column, message);
    write_symbol(out, "INVALID", message);
}

/* =========================================================================
 * Sebesta Chapter 4: Canonical Stream Primitives
 * ========================================================================= */

/**
 * addChar:
 * Appends nextChar to the global lexeme buffer safely with bounds checks.
 *
 * Textbook Reference: Sebesta Figure 4.1, addChar()
 *
 * Multi-byte UTF-8 Handling:
 * When nextChar represents a composite 2-byte UTF-8 character (e.g., ñ or Ñ),
 * both bytes are unpacked and written consecutively into lexeme[].
 */
static void addChar(void)
{
    if (nextChar > 0xFF) {
        /* Multi-byte UTF-8 character: unpack upper and lower byte */
        if (lexLen + 2 < MAX_TOKEN_LENGTH) {
            lexeme[lexLen++] = (char)((nextChar >> 8) & 0xFF);
            lexeme[lexLen++] = (char)(nextChar & 0xFF);
            lexeme[lexLen] = '\0';
        }
    } else if (nextChar != EOF) {
        /* Standard single-byte character */
        if (lexLen + 1 < MAX_TOKEN_LENGTH) {
            lexeme[lexLen++] = (char)nextChar;
            lexeme[lexLen] = '\0';
        }
    }
}

/**
 * getChar:
 * Consumes the next character from the input source buffer and determines
 * its character class (LETTER, DIGIT, UNKNOWN, or EOF).
 *
 * Textbook Reference: Sebesta Figure 4.1, getChar()
 *
 * Features:
 *   1. Transparently decodes 2-byte UTF-8 sequences for Filipino letters:
 *      - 0xC3 0xB1 ('ñ') -> sets nextChar = 0xC3B1, charClass = LETTER.
 *      - 0xC3 0x91 ('Ñ') -> sets nextChar = 0xC391, charClass = LETTER.
 *   2. Accurately maintains running current_line and current_column counters.
 *   3. Captures exact char_line and char_col for diagnostic reporting.
 */
static void getChar(void)
{
    if (src_pos < src_length) {
        unsigned char c = (unsigned char)src_buffer[src_pos];

        /* Detect 2-byte UTF-8 sequence for Filipino letters ñ and Ñ */
        if (c == 0xC3 && src_pos + 1 < src_length) {
            unsigned char c2 = (unsigned char)src_buffer[src_pos + 1];
            if (c2 == 0xB1 || c2 == 0x91) {
                nextChar = (c << 8) | c2;
                src_pos += 2;
                current_column++;
                char_line = current_line;
                char_col = current_column;
                charClass = LETTER;
                return;
            }
        }

        /* Standard 1-byte ASCII character */
        nextChar = c;
        src_pos++;
        current_column++;
        char_line = current_line;
        char_col = current_column;

        /* Advance line counter on newline and reset column */
        if (c == '\n') {
            current_line++;
            current_column = 0;
        }

        /* Classify character into Sebesta classes */
        if (isalpha(c)) {
            charClass = LETTER;
        } else if (isdigit(c)) {
            charClass = DIGIT;
        } else {
            charClass = UNKNOWN;
        }
    } else {
        /* Reached end of input stream */
        nextChar = EOF;
        charClass = EOF;
    }
}

/**
 * getNonBlank:
 * Consumes whitespace characters (spaces, tabs, newlines, carriage returns)
 * from the input stream while logging each character and transition to the
 * DFA trace log.
 *
 * Textbook Reference: Sebesta Figure 4.1, getNonBlank()
 */
static void getNonBlank(void)
{
    while (charClass != EOF &&
           (nextChar == ' ' || nextChar == '\t' || nextChar == '\r' || nextChar == '\n')) {
        write_trace(scan_log, "SCAN", char_line, char_col, nextChar, "WHITESPACE");
        write_transition(scan_log, char_line, char_col, STATE_START, nextChar, STATE_START);
        getChar();
    }
}

/* =========================================================================
 * Sebesta Chapter 4: Operator & Delimiter Lookup Function
 * ========================================================================= */

/**
 * lookup:
 * Processes non-alphanumeric characters (operators, delimiters, punctuation,
 * quotes, comments, and invalid characters) using standard switch logic.
 *
 * Textbook Reference: Sebesta Figure 4.1, lookup()
 *
 * Lookahead Strategy:
 * For multi-character operators (==, !=, <=, >=, &&, ||), peek/read logic
 * examines the following character to decide between single- and compound-operator
 * tokens, maintaining consistent stream positioning for subsequent lex() calls.
 *
 * Parameters:
 *   ch - The initial UNKNOWN character to process.
 *
 * Returns:
 *   The determined token code (nextToken).
 */
static int lookup(int ch)
{
    switch (ch) {

        /* -----------------------------------------------------------------
         * Block: Single-Line Comments (@)
         * Matches regular expression: @[^\r\n]*
         * ----------------------------------------------------------------- */
        case '@': {
            size_t comment_start_line = char_line;
            size_t comment_start_col = char_col;

            /* Log comment marker transition */
            write_trace(scan_log, "SCAN", comment_start_line, comment_start_col, '@', "COMMENT_MARKER");
            write_transition(scan_log, comment_start_line, comment_start_col, STATE_START, '@', STATE_COMMENT);
            addChar();
            getChar();

            /* Consume all characters until newline or end of file */
            while (nextChar != '\n' && nextChar != EOF) {
                if (nextChar == '\r') {
                    getChar();
                    continue;
                }
                write_trace(scan_log, "SCAN", char_line, char_col, nextChar, "COMMENT");
                write_transition(scan_log, char_line, char_col, STATE_COMMENT, nextChar, STATE_COMMENT);
                addChar();
                getChar();
            }

            nextToken = LINE_COMMENT;
            write_symbol(token_table, "LINE_COMMENT", lexeme);
            write_acceptance(scan_log, lexeme, "LINE_COMMENT", "COMMENT", STATE_COMMENT);
            break;
        }

        /* -----------------------------------------------------------------
         * Block: String Literals (SALITA)
         * Enclosed in double quotes ("..."). Must terminate on the same line.
         * ----------------------------------------------------------------- */
        case '"': {
            size_t str_start_line = char_line;
            size_t str_start_col = char_col;
            int terminated = 0;

            write_trace(scan_log, "SCAN", str_start_line, str_start_col, '"', "STRING_DELIMITER");
            write_transition(scan_log, str_start_line, str_start_col, STATE_START, '"', STATE_STRING);
            addChar();
            getChar();

            /* Read string content until closing quote or line break */
            while (nextChar != '\n' && nextChar != EOF) {
                if (nextChar == '"') {
                    write_trace(scan_log, "SCAN", char_line, char_col, '"', "STRING_DELIMITER");
                    write_transition(scan_log, char_line, char_col, STATE_STRING, '"', STATE_START);
                    addChar();
                    getChar();
                    terminated = 1;
                    break;
                }
                write_trace(scan_log, "SCAN", char_line, char_col, nextChar, "STRING_CHARACTER");
                write_transition(scan_log, char_line, char_col, STATE_STRING, nextChar, STATE_STRING);
                addChar();
                getChar();
            }

            if (terminated) {
                nextToken = TOKEN_SALITA_LITERAL;
                write_symbol(token_table, "TOKEN_SALITA_LITERAL", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_SALITA_LITERAL", "STRING_LITERAL", STATE_START);
            } else {
                /* Multiline strings are disallowed in JAVArice */
                report_error(token_table, str_start_line, str_start_col, "Unterminated string literal.");
                nextToken = TOKEN_ERROR;
            }
            break;
        }

        /* -----------------------------------------------------------------
         * Block: Character Literals (LETRA)
         * Enclosed in single quotes ('...'). Must contain exactly one character.
         * ----------------------------------------------------------------- */
        case '\'': {
            size_t char_start_line = char_line;
            size_t char_start_col = char_col;

            write_trace(scan_log, "SCAN", char_start_line, char_start_col, '\'', "CHAR_DELIMITER");
            write_transition(scan_log, char_start_line, char_start_col, STATE_START, '\'', STATE_CHAR);
            addChar();
            getChar();

            /* Check for unterminated literal at line end */
            if (nextChar == '\n' || nextChar == '\r' || nextChar == EOF) {
                report_error(token_table, char_start_line, char_start_col, "Unterminated character literal.");
                nextToken = TOKEN_ERROR;
                break;
            }

            /* Reject empty character literals: '' */
            if (nextChar == '\'') {
                write_trace(scan_log, "SCAN", char_line, char_col, '\'', "CHAR_DELIMITER");
                write_transition(scan_log, char_line, char_col, STATE_CHAR, '\'', STATE_ERROR);
                getChar();
                report_error(token_table, char_start_line, char_start_col, "Empty character literal is not allowed.");
                nextToken = TOKEN_ERROR;
                break;
            }

            /* Read exactly one character (including multi-byte UTF-8 ñ/Ñ) */
            write_trace(scan_log, "SCAN", char_line, char_col, nextChar, "CHAR_CONTENT");
            write_transition(scan_log, char_line, char_col, STATE_CHAR, nextChar, STATE_CHAR);
            addChar();
            getChar();

            /* Validate closing quote */
            if (nextChar == '\'') {
                write_trace(scan_log, "SCAN", char_line, char_col, '\'', "CHAR_DELIMITER");
                write_transition(scan_log, char_line, char_col, STATE_CHAR, '\'', STATE_START);
                addChar();
                getChar();
                nextToken = TOKEN_LETRA_LITERAL;
                write_symbol(token_table, "TOKEN_LETRA_LITERAL", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_LETRA_LITERAL", "CHARACTER_LITERAL", STATE_START);
            } else {
                /* Error recovery: Consume remaining characters until quote or newline */
                while (nextChar != '\'' && nextChar != '\n' && nextChar != EOF) {
                    getChar();
                }
                if (nextChar == '\'') {
                    getChar();
                    report_error(token_table, char_start_line, char_start_col,
                                 "Character literal may only contain a single character.");
                } else {
                    report_error(token_table, char_start_line, char_start_col,
                                 "Unterminated character literal.");
                }
                nextToken = TOKEN_ERROR;
            }
            break;
        }

        /* -----------------------------------------------------------------
         * Block: Disallowed Leading Underscore
         * Identifiers must begin with [a-zA-ZñÑ]; leading underscores are illegal.
         * ----------------------------------------------------------------- */
        case '_': {
            size_t err_line = char_line;
            size_t err_col = char_col;
            write_trace(scan_log, "SCAN", err_line, err_col, '_', "UNDERSCORE");
            write_transition(scan_log, err_line, err_col, STATE_START, '_', STATE_ERROR);
            getChar();
            /* Consume the remainder of the invalid identifier for error recovery */
            while (charClass == LETTER || charClass == DIGIT || nextChar == '_') {
                getChar();
            }
            report_error(token_table, err_line, err_col, "Identifier must not begin with an underscore.");
            nextToken = TOKEN_ERROR;
            break;
        }

        /* -----------------------------------------------------------------
         * Block: Delimiters and Punctuation
         * ----------------------------------------------------------------- */
        case '.':
            /* Statement terminator */
            write_trace(scan_log, "SCAN", char_line, char_col, '.', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '.', STATE_DOT);
            addChar();
            getChar();
            nextToken = TOKEN_TERMINATOR;
            write_symbol(token_table, "TOKEN_TERMINATOR", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_TERMINATOR", "PUNCTUATION", STATE_DOT);
            break;

        case ',':
            /* Argument separator */
            write_trace(scan_log, "SCAN", char_line, char_col, ',', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, ',', STATE_COMMA);
            addChar();
            getChar();
            nextToken = TOKEN_SEPARATOR;
            write_symbol(token_table, "TOKEN_SEPARATOR", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_SEPARATOR", "PUNCTUATION", STATE_COMMA);
            break;

        case ':':
            /* Built-in method accessor operator */
            write_trace(scan_log, "SCAN", char_line, char_col, ':', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, ':', STATE_COLON);
            addChar();
            getChar();
            nextToken = TOKEN_ACCESSOR;
            write_symbol(token_table, "TOKEN_ACCESSOR", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_ACCESSOR", "OPERATOR", STATE_COLON);
            break;

        case '(':
            /* Left parenthesis */
            write_trace(scan_log, "SCAN", char_line, char_col, '(', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '(', STATE_LPAREN);
            addChar();
            getChar();
            nextToken = TOKEN_LPAREN;
            write_symbol(token_table, "TOKEN_LPAREN", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_LPAREN", "DELIMITER", STATE_LPAREN);
            break;

        case ')':
            /* Right parenthesis */
            write_trace(scan_log, "SCAN", char_line, char_col, ')', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, ')', STATE_RPAREN);
            addChar();
            getChar();
            nextToken = TOKEN_RPAREN;
            write_symbol(token_table, "TOKEN_RPAREN", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_RPAREN", "DELIMITER", STATE_RPAREN);
            break;

        case '{':
            /* Left block brace */
            write_trace(scan_log, "SCAN", char_line, char_col, '{', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '{', STATE_LBRACE);
            addChar();
            getChar();
            nextToken = TOKEN_LBRACE;
            write_symbol(token_table, "TOKEN_LBRACE", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_LBRACE", "DELIMITER", STATE_LBRACE);
            break;

        case '}':
            /* Right block brace */
            write_trace(scan_log, "SCAN", char_line, char_col, '}', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '}', STATE_RBRACE);
            addChar();
            getChar();
            nextToken = TOKEN_RBRACE;
            write_symbol(token_table, "TOKEN_RBRACE", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_RBRACE", "DELIMITER", STATE_RBRACE);
            break;

        /* -----------------------------------------------------------------
         * Block: Single-Character Arithmetic Operators
         * ----------------------------------------------------------------- */
        case '+':
            write_trace(scan_log, "SCAN", char_line, char_col, '+', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '+', STATE_OP_ADD);
            addChar();
            getChar();
            nextToken = TOKEN_OP_ADD;
            write_symbol(token_table, "TOKEN_OP_ADD", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_OP_ADD", "ARITHMETIC_OPERATOR", STATE_OP_ADD);
            break;

        case '-':
            write_trace(scan_log, "SCAN", char_line, char_col, '-', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '-', STATE_OP_SUB);
            addChar();
            getChar();
            nextToken = TOKEN_OP_SUB;
            write_symbol(token_table, "TOKEN_OP_SUB", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_OP_SUB", "ARITHMETIC_OPERATOR", STATE_OP_SUB);
            break;

        case '*':
            write_trace(scan_log, "SCAN", char_line, char_col, '*', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '*', STATE_OP_MUL);
            addChar();
            getChar();
            nextToken = TOKEN_OP_MUL;
            write_symbol(token_table, "TOKEN_OP_MUL", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_OP_MUL", "ARITHMETIC_OPERATOR", STATE_OP_MUL);
            break;

        case '/':
            write_trace(scan_log, "SCAN", char_line, char_col, '/', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '/', STATE_OP_DIV);
            addChar();
            getChar();
            nextToken = TOKEN_OP_DIV;
            write_symbol(token_table, "TOKEN_OP_DIV", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_OP_DIV", "ARITHMETIC_OPERATOR", STATE_OP_DIV);
            break;

        case '%':
            write_trace(scan_log, "SCAN", char_line, char_col, '%', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '%', STATE_OP_MOD);
            addChar();
            getChar();
            nextToken = TOKEN_OP_MOD;
            write_symbol(token_table, "TOKEN_OP_MOD", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_OP_MOD", "ARITHMETIC_OPERATOR", STATE_OP_MOD);
            break;

        case '\\':
            write_trace(scan_log, "SCAN", char_line, char_col, '\\', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '\\', STATE_OP_INTDIV);
            addChar();
            getChar();
            nextToken = TOKEN_OP_INTDIV;
            write_symbol(token_table, "TOKEN_OP_INTDIV", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_OP_INTDIV", "ARITHMETIC_OPERATOR", STATE_OP_INTDIV);
            break;

        case '^':
            write_trace(scan_log, "SCAN", char_line, char_col, '^', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '^', STATE_OP_POW);
            addChar();
            getChar();
            nextToken = TOKEN_OP_POW;
            write_symbol(token_table, "TOKEN_OP_POW", lexeme);
            write_acceptance(scan_log, lexeme, "TOKEN_OP_POW", "ARITHMETIC_OPERATOR", STATE_OP_POW);
            break;

        /* -----------------------------------------------------------------
         * Block: Compound Operators with 1-Character Lookahead
         * ----------------------------------------------------------------- */

        /* Assignment (=) vs Relational Equality (==) */
        case '=':
            write_trace(scan_log, "SCAN", char_line, char_col, '=', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '=', STATE_ASSIGN);
            addChar();
            getChar();
            if (nextChar == '=') {
                write_trace(scan_log, "SCAN", char_line, char_col, '=', "SYMBOL");
                write_transition(scan_log, char_line, char_col, STATE_ASSIGN, '=', STATE_REL_EQ);
                addChar();
                getChar();
                nextToken = TOKEN_REL_EQ;
                write_symbol(token_table, "TOKEN_REL_EQ", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_REL_EQ", "RELATIONAL_OPERATOR", STATE_REL_EQ);
            } else {
                nextToken = TOKEN_ASSIGNMENT;
                write_symbol(token_table, "TOKEN_ASSIGNMENT", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_ASSIGNMENT", "OPERATOR", STATE_ASSIGN);
            }
            break;

        /* Logical NOT (!) vs Relational Inequality (!=) */
        case '!':
            write_trace(scan_log, "SCAN", char_line, char_col, '!', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '!', STATE_LOGIC_NOT);
            addChar();
            getChar();
            if (nextChar == '=') {
                write_trace(scan_log, "SCAN", char_line, char_col, '=', "SYMBOL");
                write_transition(scan_log, char_line, char_col, STATE_LOGIC_NOT, '=', STATE_REL_NEQ);
                addChar();
                getChar();
                nextToken = TOKEN_REL_NEQ;
                write_symbol(token_table, "TOKEN_REL_NEQ", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_REL_NEQ", "RELATIONAL_OPERATOR", STATE_REL_NEQ);
            } else {
                nextToken = TOKEN_LOGIC_NOT;
                write_symbol(token_table, "TOKEN_LOGIC_NOT", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_LOGIC_NOT", "LOGICAL_OPERATOR", STATE_LOGIC_NOT);
            }
            break;

        /* Greater Than (>) vs Greater Than or Equal (>=) */
        case '>':
            write_trace(scan_log, "SCAN", char_line, char_col, '>', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '>', STATE_REL_GT);
            addChar();
            getChar();
            if (nextChar == '=') {
                write_trace(scan_log, "SCAN", char_line, char_col, '=', "SYMBOL");
                write_transition(scan_log, char_line, char_col, STATE_REL_GT, '=', STATE_REL_GTE);
                addChar();
                getChar();
                nextToken = TOKEN_REL_GTE;
                write_symbol(token_table, "TOKEN_REL_GTE", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_REL_GTE", "RELATIONAL_OPERATOR", STATE_REL_GTE);
            } else {
                nextToken = TOKEN_REL_GT;
                write_symbol(token_table, "TOKEN_REL_GT", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_REL_GT", "RELATIONAL_OPERATOR", STATE_REL_GT);
            }
            break;

        /* Less Than (<) vs Less Than or Equal (<=) */
        case '<':
            write_trace(scan_log, "SCAN", char_line, char_col, '<', "SYMBOL");
            write_transition(scan_log, char_line, char_col, STATE_START, '<', STATE_REL_LT);
            addChar();
            getChar();
            if (nextChar == '=') {
                write_trace(scan_log, "SCAN", char_line, char_col, '=', "SYMBOL");
                write_transition(scan_log, char_line, char_col, STATE_REL_LT, '=', STATE_REL_LTE);
                addChar();
                getChar();
                nextToken = TOKEN_REL_LTE;
                write_symbol(token_table, "TOKEN_REL_LTE", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_REL_LTE", "RELATIONAL_OPERATOR", STATE_REL_LTE);
            } else {
                nextToken = TOKEN_REL_LT;
                write_symbol(token_table, "TOKEN_REL_LT", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_REL_LT", "RELATIONAL_OPERATOR", STATE_REL_LT);
            }
            break;

        /* Logical AND (&&): Single '&' is unsupported in JAVArice */
        case '&': {
            size_t sym_line = char_line;
            size_t sym_col = char_col;
            write_trace(scan_log, "SCAN", sym_line, sym_col, '&', "SYMBOL");
            write_transition(scan_log, sym_line, sym_col, STATE_START, '&', STATE_AMP_FIRST);
            addChar();
            getChar();
            if (nextChar == '&') {
                write_trace(scan_log, "SCAN", char_line, char_col, '&', "SYMBOL");
                write_transition(scan_log, char_line, char_col, STATE_AMP_FIRST, '&', STATE_LOGIC_AND);
                addChar();
                getChar();
                nextToken = TOKEN_LOGIC_AND;
                write_symbol(token_table, "TOKEN_LOGIC_AND", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_LOGIC_AND", "LOGICAL_OPERATOR", STATE_LOGIC_AND);
            } else {
                /* Bare single ampersand is invalid */
                report_error(token_table, sym_line, sym_col, "Invalid symbol transition.");
                if (scan_log != NULL) {
                    fprintf(scan_log, "REJECT | [%s] | State: q%d | No accepting token\n",
                            lexeme, STATE_AMP_FIRST);
                }
                nextToken = TOKEN_ERROR;
            }
            break;
        }

        /* Logical OR (||): Single '|' is unsupported in JAVArice */
        case '|': {
            size_t sym_line = char_line;
            size_t sym_col = char_col;
            write_trace(scan_log, "SCAN", sym_line, sym_col, '|', "SYMBOL");
            write_transition(scan_log, sym_line, sym_col, STATE_START, '|', STATE_PIPE_FIRST);
            addChar();
            getChar();
            if (nextChar == '|') {
                write_trace(scan_log, "SCAN", char_line, char_col, '|', "SYMBOL");
                write_transition(scan_log, char_line, char_col, STATE_PIPE_FIRST, '|', STATE_LOGIC_OR);
                addChar();
                getChar();
                nextToken = TOKEN_LOGIC_OR;
                write_symbol(token_table, "TOKEN_LOGIC_OR", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_LOGIC_OR", "LOGICAL_OPERATOR", STATE_LOGIC_OR);
            } else {
                /* Bare single pipe is invalid */
                report_error(token_table, sym_line, sym_col, "Invalid symbol transition.");
                if (scan_log != NULL) {
                    fprintf(scan_log, "REJECT | [%s] | State: q%d | No accepting token\n",
                            lexeme, STATE_PIPE_FIRST);
                }
                nextToken = TOKEN_ERROR;
            }
            break;
        }

        /* -----------------------------------------------------------------
         * Block: Default Unrecognized Characters
         * Catch-all for unsupported characters (e.g., ~, $, #).
         * ----------------------------------------------------------------- */
        default: {
            size_t err_line = char_line;
            size_t err_col = char_col;
            write_trace(scan_log, "SCAN", err_line, err_col, ch, "SYMBOL");
            write_transition(scan_log, err_line, err_col, STATE_START, ch, STATE_ERROR);
            report_error(token_table, err_line, err_col, "Unrecognized character.");
            getChar();
            nextToken = TOKEN_ERROR;
            break;
        }
    }
    return nextToken;
}

/* =========================================================================
 * Sebesta Chapter 4: Canonical Lexical Analyzer Driver
 * ========================================================================= */

/**
 * lex:
 * Central lexical analysis driver function matching Sebesta Figure 4.1.
 *
 * Execution Flow:
 *   1. Clears current lexeme buffer (lexLen = 0).
 *   2. Invokes getNonBlank() to consume whitespace and advance stream.
 *   3. Snapshots token starting coordinates (token_line, token_column).
 *   4. Dispatches on charClass using a clean switch statement:
 *      - LETTER : Gathers identifiers & keywords, invokes lookup_keyword().
 *      - DIGIT  : Gathers integers & decimals, performs decimal lookahead.
 *      - UNKNOWN: Delegates to lookup() for operators, delimiters, literals.
 *      - EOF    : Sets nextToken = TOKEN_EOF and terminates scanning.
 *
 * Returns:
 *   The integer token code for the next recognized token.
 */
static int lex(void)
{
    lexLen = 0;
    lexeme[0] = '\0';
    getNonBlank();
    token_line = char_line;
    token_column = char_col;

    switch (charClass) {

        /* -----------------------------------------------------------------
         * Case: LETTER (Identifiers, Keywords, Reserved Words, Noise Words)
         * Matches regex: ^[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*$
         * ----------------------------------------------------------------- */
        case LETTER: {
            const char *t_name = NULL;
            const char *t_class = NULL;
            int first_char = nextChar;

            /* Log transition from start state q0 to identifier state q900 */
            write_trace(scan_log, "SCAN", char_line, char_col, first_char, "LETTER");
            write_transition(scan_log, char_line, char_col, STATE_START, first_char, STATE_IDENTIFIER);
            addChar();
            getChar();

            /* Gather subsequent letters, digits, underscores, and Filipino letters */
            while (charClass == LETTER || charClass == DIGIT || nextChar == '_') {
                const char *cls = (charClass == DIGIT) ? "DIGIT" :
                                  (nextChar == '_') ? "UNDERSCORE" : "LETTER";
                write_trace(scan_log, "SCAN", char_line, char_col, nextChar, cls);
                write_transition(scan_log, char_line, char_col, STATE_IDENTIFIER, nextChar, STATE_IDENTIFIER);
                addChar();
                getChar();
            }

            /* Reclassify identifier against the static special words table */
            nextToken = lookup_keyword(lexeme, &t_name, &t_class);
            write_symbol(token_table, t_name, lexeme);
            write_acceptance(scan_log, lexeme, t_name, t_class, STATE_IDENTIFIER);
            break;
        }

        /* -----------------------------------------------------------------
         * Case: DIGIT (Numeric Literals: BUO vs NUMERO)
         * Distinguishes integers (BUO) from fixed-point decimals (NUMERO).
         * ----------------------------------------------------------------- */
        case DIGIT: {
            int first_char = nextChar;

            /* Log transition from start state q0 to integer state q901 */
            write_trace(scan_log, "SCAN", char_line, char_col, first_char, "DIGIT");
            write_transition(scan_log, char_line, char_col, STATE_START, first_char, STATE_INTEGER);
            addChar();
            getChar();

            /* Consume integer portion */
            while (charClass == DIGIT) {
                write_trace(scan_log, "SCAN", char_line, char_col, nextChar, "DIGIT");
                write_transition(scan_log, char_line, char_col, STATE_INTEGER, nextChar, STATE_INTEGER);
                addChar();
                getChar();
            }

            /*
             * Lookahead Check:
             * If nextChar is '.' and the byte immediately following it is a DIGIT,
             * this is a fixed-point decimal literal NUMERO (e.g., 120.00).
             * Otherwise, '.' is a statement terminator (e.g., 10.) and must remain
             * unconsumed in the stream for the next lex() invocation.
             */
            if (nextChar == '.' && src_pos < src_length && isdigit((unsigned char)src_buffer[src_pos])) {
                write_trace(scan_log, "SCAN", char_line, char_col, '.', "DECIMAL_POINT");
                write_transition(scan_log, char_line, char_col, STATE_INTEGER, '.', STATE_DECIMAL);
                addChar();
                getChar();

                /* Consume fractional digits */
                while (charClass == DIGIT) {
                    write_trace(scan_log, "SCAN", char_line, char_col, nextChar, "DIGIT");
                    write_transition(scan_log, char_line, char_col, STATE_DECIMAL, nextChar, STATE_DECIMAL);
                    addChar();
                    getChar();
                }

                nextToken = TOKEN_NUMERO_LITERAL;
                write_symbol(token_table, "TOKEN_NUMERO_LITERAL", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_NUMERO_LITERAL", "DECIMAL_LITERAL", STATE_DECIMAL);
            } else {
                /* Standalone integer (BUO) */
                nextToken = TOKEN_BUO_LITERAL;
                write_symbol(token_table, "TOKEN_BUO_LITERAL", lexeme);
                write_acceptance(scan_log, lexeme, "TOKEN_BUO_LITERAL", "INTEGER_LITERAL", STATE_INTEGER);
            }
            break;
        }

        /* -----------------------------------------------------------------
         * Case: UNKNOWN (Delimiters, Operators, Quotes, Comments, Unknowns)
         * Delegated to Sebesta's lookup() switch function.
         * ----------------------------------------------------------------- */
        case UNKNOWN:
            lookup(nextChar);
            break;

        /* -----------------------------------------------------------------
         * Case: EOF (End-of-File Termination)
         * ----------------------------------------------------------------- */
        case EOF:
            nextToken = TOKEN_EOF;
            lexeme[0] = 'E';
            lexeme[1] = 'O';
            lexeme[2] = 'F';
            lexeme[3] = '\0';
            break;

        default:
            nextToken = TOKEN_EOF;
            break;
    }

    token = nextToken;
    return nextToken;
}

/* =========================================================================
 * Source File Loading & Stream Pipeline
 * ========================================================================= */

/**
 * append_char:
 * Dynamically resizes the in-memory source buffer to hold the input file text.
 */
static int append_char(char **buffer, size_t *length, size_t *capacity, int c)
{
    char *grown;
    size_t new_capacity;
    if (*length + 1 >= *capacity) {
        new_capacity = (*capacity == 0) ? 64 : *capacity * 2;
        grown = realloc(*buffer, new_capacity);
        if (grown == NULL) return 0;
        *buffer = grown;
        *capacity = new_capacity;
    }
    (*buffer)[(*length)++] = (char)c;
    (*buffer)[*length] = '\0';
    return 1;
}

/**
 * valid_source_extension:
 * Validates that the input file path carries the mandatory .rice extension.
 */
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

/**
 * read_source:
 * Reads and validates the source file into memory:
 *   - Verifies the .rice extension.
 *   - Validates regular file mode.
 *   - Enforces supported character encoding (ASCII 32-126, \t, \r, \n, and
 *     2-byte UTF-8 sequences 0xC3 0xB1 / 0xC3 0x91 for ñ and Ñ).
 */
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
        fprintf(stderr, "Cannot open input file '%s': %s\n", path, strerror(errno));
        return 0;
    }

    while ((c = fgetc(file)) != EOF) {
        /* UTF-8 multi-byte validation for Filipino letters */
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
                if (c2 != EOF) ungetc(c2, file);
                fprintf(stderr, "Input contains unsupported character at line %zu, column %zu.\n",
                        line, column);
                fclose(file);
                free(*source);
                *source = NULL;
                *length = 0;
                return 0;
            }
        }
        /* ASCII printable character validation */
        if (c == '\0' || (c < 32 && c != '\t' && c != '\r' && c != '\n') || c > 126) {
            fprintf(stderr, "Input is not supported plain text at line %zu, column %zu.\n",
                    line, column);
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

/**
 * copy_stream:
 * Copies contents from a temporary file stream into the final destination file.
 */
static int copy_stream(FILE *source, FILE *destination)
{
    char buffer[4096];
    size_t bytes_read;

    if (fseek(source, 0, SEEK_SET) != 0) return 0;
    while ((bytes_read = fread(buffer, 1, sizeof buffer, source)) > 0) {
        if (fwrite(buffer, 1, bytes_read, destination) != bytes_read) return 0;
    }
    return !ferror(source);
}

/* =========================================================================
 * Main Program Execution
 * ========================================================================= */

/**
 * main:
 * Entry point for the JAVArice Lexical Analyzer CLI.
 *
 * Workflow:
 *   1. Validates CLI arguments (<source.rice>).
 *   2. Reads and verifies source code into an in-memory buffer.
 *   3. Primes the Sebesta input stream with the initial getChar() call.
 *   4. Runs the main scanner loop (while nextToken != TOKEN_EOF lex()).
 *   5. Writes the formatted Symbol Table file with token dump and DFA trace.
 *   6. Returns EXIT_SUCCESS or EXIT_FAILURE based on lexical error count.
 */
int main(int argc, char **argv)
{
    char *source = NULL;
    size_t source_length = 0;
    FILE *symbol_table;
    int errors = 0;

    /* Verify command-line invocation syntax */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source.rice>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Read and validate input source file */
    if (!read_source(argv[1], &source, &source_length)) {
        return EXIT_FAILURE;
    }

    /* Initialize temporary streams for two-column table and scan trace */
    scan_log = tmpfile();
    token_table = tmpfile();
    if (scan_log == NULL || token_table == NULL) {
        fprintf(stderr, "Cannot create temporary output files: %s\n", strerror(errno));
        if (scan_log != NULL) fclose(scan_log);
        if (token_table != NULL) fclose(token_table);
        free(source);
        return EXIT_FAILURE;
    }

    /* Initialize global Sebesta stream state */
    src_buffer = source;
    src_length = source_length;
    src_pos = 0;
    current_line = 1;
    current_column = 0;
    char_line = 1;
    char_col = 0;
    lexLen = 0;
    nextToken = 0;

    /* Prime the stream with the first character */
    getChar();

    /* Execute the Sebesta scanner loop until EOF */
    while (nextToken != TOKEN_EOF) {
        lex();
        if (nextToken == TOKEN_ERROR) {
            errors++;
        }
    }

    /* Free heap-allocated source buffer */
    free(source);

    /* Flush temporary file buffers before copying */
    if (fflush(scan_log) != 0 || fflush(token_table) != 0) {
        fprintf(stderr, "Error buffering temporary lexer output.\n");
        fclose(scan_log);
        fclose(token_table);
        return EXIT_FAILURE;
    }

    /* Open final Symbol Table output file */
    symbol_table = fopen(SYMBOL_TABLE_FILE, "w");
    if (symbol_table == NULL) {
        fprintf(stderr, "Cannot create '%s': %s\n", SYMBOL_TABLE_FILE, strerror(errno));
        fclose(scan_log);
        fclose(token_table);
        return EXIT_FAILURE;
    }

    /* Section 1: Two-column token table */
    fprintf(symbol_table, "%-24s | %s\n", "TOKEN TYPE", "LEXEME");
    fprintf(symbol_table,
            "------------------------ | ----------------------------------------\n");
    if (!copy_stream(token_table, symbol_table)) {
        fprintf(stderr, "Error writing token table to '%s'.\n", SYMBOL_TABLE_FILE);
        fclose(scan_log);
        fclose(token_table);
        fclose(symbol_table);
        return EXIT_FAILURE;
    }

    /* Section 2: Character-by-character scan log and DFA transitions */
    fprintf(symbol_table, "\nCHARACTER-BY-CHARACTER SCAN AND DFA TRANSITIONS\n");
    fprintf(symbol_table,
            "ACCEPT rows show the final accepting state and token category.\n"
            "REJECT rows show lexemes that reached no accepting token state.\n");
    if (!copy_stream(scan_log, symbol_table)) {
        fprintf(stderr, "Error writing DFA trace to '%s'.\n", SYMBOL_TABLE_FILE);
        fclose(scan_log);
        fclose(token_table);
        fclose(symbol_table);
        return EXIT_FAILURE;
    }

    /* Close all open file descriptors */
    if (fclose(scan_log) != 0) {
        fprintf(stderr, "Error closing temporary DFA trace.\n");
        fclose(token_table);
        fclose(symbol_table);
        return EXIT_FAILURE;
    }
    if (fclose(token_table) != 0) {
        fprintf(stderr, "Error closing temporary output file.\n");
        fclose(symbol_table);
        return EXIT_FAILURE;
    }
    if (fclose(symbol_table) != 0) {
        fprintf(stderr, "Error writing '%s'.\n", SYMBOL_TABLE_FILE);
        return EXIT_FAILURE;
    }

    /* Report final status */
    if (errors != 0) {
        fprintf(stderr, "Lexical analysis completed with %d error(s). See %s.\n",
                errors, SYMBOL_TABLE_FILE);
        return EXIT_FAILURE;
    }

    printf("Lexical analysis completed successfully. Symbol table written to %s.\n",
           SYMBOL_TABLE_FILE);
    return EXIT_SUCCESS;
}
