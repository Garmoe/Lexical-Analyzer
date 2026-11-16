#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#define SYMBOL_TABLE_FILE "Symbol Table"
#define EXTENSION_ERROR_MESSAGE "Input must use the .rice file extension"

/* ================================================================= */
/* 1. TOKEN DEFINITIONS                                              */
/* ================================================================= */

typedef enum {
    TOKEN_KIND_ACCESSOR,
    TOKEN_KIND_ASSIGNMENT,
    TOKEN_KIND_BUO_LITERAL,
    TOKEN_KIND_IDENTIFIER,
    TOKEN_KIND_KW_BUO,
    TOKEN_KIND_KW_HINTO,
    TOKEN_KIND_KW_KUNDI,
    TOKEN_KIND_KW_KUNDISYON,
    TOKEN_KIND_KW_KUNG,
    TOKEN_KIND_KW_LETRA,
    TOKEN_KIND_KW_NUMERO,
    TOKEN_KIND_KW_SALITA,
    TOKEN_KIND_KW_TULOY,
    TOKEN_KIND_KW_ULIT,
    TOKEN_KIND_KW_WALA,
    TOKEN_KIND_LBRACE,
    TOKEN_KIND_LETRA_LITERAL,
    TOKEN_KIND_LOGIC_AND,
    TOKEN_KIND_LOGIC_NOT,
    TOKEN_KIND_LOGIC_OR,
    TOKEN_KIND_LPAREN,
    TOKEN_KIND_NOISE_WORD,
    TOKEN_KIND_NUMERO_LITERAL,
    TOKEN_KIND_OP_ADD,
    TOKEN_KIND_OP_DIV,
    TOKEN_KIND_OP_INTDIV,
    TOKEN_KIND_OP_MOD,
    TOKEN_KIND_OP_MUL,
    TOKEN_KIND_OP_POW,
    TOKEN_KIND_OP_SUB,
    TOKEN_KIND_RBRACE,
    TOKEN_KIND_REL_EQ,
    TOKEN_KIND_REL_GT,
    TOKEN_KIND_REL_GTE,
    TOKEN_KIND_REL_LT,
    TOKEN_KIND_REL_LTE,
    TOKEN_KIND_REL_NEQ,
    TOKEN_KIND_RES_BAYAD,
    TOKEN_KIND_RES_HAIN,
    TOKEN_KIND_RES_KATUMBAS,
    TOKEN_KIND_RES_LUTO,
    TOKEN_KIND_RES_MALI,
    TOKEN_KIND_RES_ORDER,
    TOKEN_KIND_RES_RESIBO,
    TOKEN_KIND_RES_TAMA,
    TOKEN_KIND_RES_ULAM,
    TOKEN_KIND_RES_BAWAS,
    TOKEN_KIND_RES_BAYARAN,
    TOKEN_KIND_RES_DAGDAG,
    TOKEN_KIND_RES_DISKWENTO,
    TOKEN_KIND_RES_HATIAN,
    TOKEN_KIND_RES_ILAGAY,
    TOKEN_KIND_RES_ILIMBAG,
    TOKEN_KIND_RES_KALKULA,
    TOKEN_KIND_RES_KUMUHA,
    TOKEN_KIND_RES_LINISIN,
    TOKEN_KIND_RES_MERON,
    TOKEN_KIND_RPAREN,
    TOKEN_KIND_SALITA_LITERAL,
    TOKEN_KIND_SEPARATOR,
    TOKEN_KIND_TERMINATOR,
    TOKEN_KIND_INVALID,
    TOKEN_KIND_LINE_COMMENT
} TokenKind;

typedef struct {
    int state;
    TokenKind kind;
    char classification[32];
    int ignored;
} AcceptingState;


/* ================================================================= */
/* 2. DFA DECLARATIONS                                               */
/* ================================================================= */

enum {
    CLASS_ALPHA = 256,
    CLASS_DIGIT = 257,
    CLASS_UNDERSCORE = 258,
    CLASS_WHITESPACE = 259,
    CLASS_ANY = 260,
    CLASS_SYMBOL = 261
};

int dfa_char_class(int c);
const char *dfa_char_class_label(int c);

enum {
    STATE_START = 0,
    STATE_IDENTIFIER = 900,
    STATE_INTEGER = 901,
    STATE_DECIMAL = 902,
    STATE_STRING = 903,
    STATE_COMMENT = 904,
    STATE_ERROR = 905,
    STATE_CHAR = 906,
    STATE_STRING_END = 907,
    STATE_CHAR_END = 908
};

int dfa_is_filipino_letter(const unsigned char *source, size_t position, size_t length);

static int is_identifier_start(int character) {
    return dfa_char_class(character) == CLASS_ALPHA;
}

static int is_identifier_part(int character) {
    int cls = dfa_char_class(character);
    return cls == CLASS_ALPHA || cls == CLASS_DIGIT || cls == CLASS_UNDERSCORE;
}

int dfa_is_whitespace(int character);

int dfa_char_class(int c) {
    if (isalpha((unsigned char)c)) return CLASS_ALPHA;
    if (isdigit((unsigned char)c)) return CLASS_DIGIT;
    if (c == '_') return CLASS_UNDERSCORE;
    if (dfa_is_whitespace(c)) return CLASS_WHITESPACE;
    return CLASS_SYMBOL;
}

const char *dfa_char_class_label(int c) {
    switch (dfa_char_class(c)) {
    case CLASS_ALPHA: return "LETTER";
    case CLASS_DIGIT: return "DIGIT";
    case CLASS_UNDERSCORE: return "UNDERSCORE";
    case CLASS_WHITESPACE: return "WHITESPACE";
    default: return "SYMBOL";
    }
}

int dfa_step(int state, int character);

const AcceptingState *dfa_find_accept(int state);
const char *dfa_token_kind_name(TokenKind kind);
void dfa_write_symbol_span(FILE *out, const char *type, const char *lexeme, size_t length);
void dfa_write_symbol(FILE *out, TokenKind kind, const char *lexeme);
void dfa_write_acceptance_span(FILE *out, const char *lexeme, size_t length, TokenKind kind, const char *classification, int state);
void dfa_write_acceptance(FILE *out, const char *lexeme, TokenKind kind, const char *classification, int state);
void dfa_report_error(FILE *out, size_t line, size_t column, const char *message);


/* ================================================================= */
/* 3. LEXER DECLARATIONS                                             */
/* ================================================================= */

int lexer_read_source(const char *path, char **source, size_t *length);
int lexer_scan(const char *source, size_t length, FILE *log, FILE *tokens);

/* ================================================================= */
/* 4. DFA IMPLEMENTATION                                             */
/* ================================================================= */

enum {
    WORD_L = 1, WORD_LE = 2, WORD_LET = 3, WORD_LETR = 4, WORD_LETRA = 5,
    WORD_S = 6, WORD_SA = 7, WORD_SAL = 8, WORD_SALI = 9, WORD_SALIT = 10, WORD_SALITA = 11,
    WORD_B = 12, WORD_BU = 13, WORD_BUO = 14,
    WORD_N = 15, WORD_NU = 16, WORD_NUM = 17, WORD_NUME = 18, WORD_NUMER = 19, WORD_NUMERO = 20,
    WORD_K = 21, WORD_KU = 22, WORD_KUN = 23, WORD_KUND = 24, WORD_KUNDI = 25, WORD_KUNDIS = 26, WORD_KUNDISY = 27, WORD_KUNDISYO = 28, WORD_KUNDISYON = 29, WORD_KUNG = 30,
    WORD_U = 31, WORD_UL = 32, WORD_ULI = 33, WORD_ULIT = 34,
    WORD_H = 35, WORD_HI = 36, WORD_HIN = 37, WORD_HINT = 38, WORD_HINTO = 39,
    WORD_T = 40, WORD_TU = 41, WORD_TUL = 42, WORD_TULO = 43, WORD_TULOY = 44,
    WORD_W = 45, WORD_WA = 46, WORD_WAL = 47, WORD_WALA = 48,
    WORD_TA = 49, WORD_TAM = 50, WORD_TAMA = 51,
    WORD_M = 52, WORD_MA = 53, WORD_MAL = 54, WORD_MALI = 55,
    WORD_KA = 56, WORD_KAT = 57, WORD_KATU = 58, WORD_KATUM = 59, WORD_KATUMB = 60, WORD_KATUMBA = 61, WORD_KATUMBAS = 62,
    WORD_ULA = 63, WORD_ULAM = 64,
    WORD_i = 65, WORD_il = 66, WORD_ila = 67, WORD_ilag = 68, WORD_ilaga = 69, WORD_ilagay = 70,
    WORD_m = 71, WORD_me = 72, WORD_mer = 73, WORD_mero = 74, WORD_meron = 75,
    WORD_k = 76, WORD_ku = 77, WORD_kum = 78, WORD_kumu = 79, WORD_kumuh = 80, WORD_kumuha = 81,
    WORD_R = 82, WORD_RE = 83, WORD_RES = 84, WORD_RESI = 85, WORD_RESIB = 86, WORD_RESIBO = 87,
    WORD_d = 88, WORD_da = 89, WORD_dag = 90, WORD_dagd = 91, WORD_dagda = 92, WORD_dagdag = 93,
    WORD_b = 94, WORD_ba = 95, WORD_baw = 96, WORD_bawa = 97, WORD_bawas = 98,
    WORD_ka = 99, WORD_kal = 100, WORD_kalk = 101, WORD_kalku = 102, WORD_kalkul = 103, WORD_kalkula = 104,
    WORD_di = 105, WORD_dis = 106, WORD_disk = 107, WORD_diskw = 108, WORD_diskwe = 109, WORD_diskwen = 110, WORD_diskwent = 111, WORD_diskwento = 112,
    WORD_bay = 113, WORD_baya = 114, WORD_bayar = 115, WORD_bayara = 116, WORD_bayaran = 117,
    WORD_ili = 118, WORD_ilim = 119, WORD_ilimb = 120, WORD_ilimba = 121, WORD_ilimbag = 122,
    WORD_l = 123, WORD_li = 124, WORD_lin = 125, WORD_lini = 126, WORD_linis = 127, WORD_linisi = 128, WORD_linisin = 129,
    WORD_h = 130, WORD_ha = 131, WORD_hat = 132, WORD_hati = 133, WORD_hatia = 134, WORD_hatian = 135,
    WORD_O = 136, WORD_OR = 137, WORD_ORD = 138, WORD_ORDE = 139, WORD_ORDER = 140,
    WORD_LU = 141, WORD_LUT = 142, WORD_LUTO = 143,
    WORD_HA = 144, WORD_HAI = 145, WORD_HAIN = 146,
    WORD_BA = 147, WORD_BAY = 148, WORD_BAYA = 149, WORD_BAYAD = 150,
    WORD_p = 151, WORD_po = 152,
    WORD_n = 153, WORD_na = 154,
    WORD_la = 155, WORD_lan = 156, WORD_lang = 157,
    WORD_a = 158, WORD_at = 159,
    WORD_nan = 160, WORD_nang = 161, WORD_ng = 162,
    WORD_an = 163, WORD_ang = 164, WORD_ay = 165,
    WORD_ni = 166, WORD_nit = 167, WORD_nito = 168,
    WORD_o = 169, WORD_op = 170, WORD_opo = 171,
    WORD_nga = 172, WORD_ngan = 173, WORD_ngani = 174,
    SYMBOL_DOT = 920, SYMBOL_COMMA = 921, SYMBOL_COLON = 922,
    SYMBOL_LEFT_PAREN = 923, SYMBOL_RIGHT_PAREN = 924,
    SYMBOL_LEFT_BRACE = 925, SYMBOL_RIGHT_BRACE = 926,
    SYMBOL_PLUS = 927, SYMBOL_MINUS = 928, SYMBOL_STAR = 929,
    SYMBOL_SLASH = 930, SYMBOL_PERCENT = 931, SYMBOL_BACKSLASH = 932,
    SYMBOL_CARET = 933, SYMBOL_EQUAL = 934, SYMBOL_EQUAL_EQUAL = 935,
    SYMBOL_BANG = 936, SYMBOL_BANG_EQUAL = 937,
    SYMBOL_GREATER = 938, SYMBOL_GREATER_EQUAL = 939,
    SYMBOL_LESS = 940, SYMBOL_LESS_EQUAL = 941,
    SYMBOL_AMPERSAND = 942, SYMBOL_AMPERSAND_AMPERSAND = 943,
    SYMBOL_PIPE = 944, SYMBOL_PIPE_PIPE = 945
};

static const AcceptingState accepting_states[] = {
    {WORD_LETRA, TOKEN_KIND_KW_LETRA, "KEYWORD", 0},
    {WORD_SALITA, TOKEN_KIND_KW_SALITA, "KEYWORD", 0},
    {WORD_BUO, TOKEN_KIND_KW_BUO, "KEYWORD", 0},
    {WORD_NUMERO, TOKEN_KIND_KW_NUMERO, "KEYWORD", 0},
    {WORD_KUNDISYON, TOKEN_KIND_KW_KUNDISYON, "KEYWORD", 0},
    {WORD_KUNG, TOKEN_KIND_KW_KUNG, "KEYWORD", 0},
    {WORD_KUNDI, TOKEN_KIND_KW_KUNDI, "KEYWORD", 0},
    {WORD_ULIT, TOKEN_KIND_KW_ULIT, "KEYWORD", 0},
    {WORD_HINTO, TOKEN_KIND_KW_HINTO, "KEYWORD", 0},
    {WORD_TULOY, TOKEN_KIND_KW_TULOY, "KEYWORD", 0},
    {WORD_WALA, TOKEN_KIND_KW_WALA, "KEYWORD", 0},
    {WORD_TAMA, TOKEN_KIND_RES_TAMA, "RESERVED_WORD", 0},
    {WORD_MALI, TOKEN_KIND_RES_MALI, "RESERVED_WORD", 0},
    {WORD_KATUMBAS, TOKEN_KIND_RES_KATUMBAS, "RESERVED_WORD", 0},
    {WORD_ULAM, TOKEN_KIND_RES_ULAM, "BUILT_IN_OBJECT", 0},
    {WORD_ilagay, TOKEN_KIND_RES_ILAGAY, "BUILT_IN_METHOD", 0},
    {WORD_meron, TOKEN_KIND_RES_MERON, "BUILT_IN_METHOD", 0},
    {WORD_kumuha, TOKEN_KIND_RES_KUMUHA, "BUILT_IN_METHOD", 0},
    {WORD_RESIBO, TOKEN_KIND_RES_RESIBO, "BUILT_IN_OBJECT", 0},
    {WORD_dagdag, TOKEN_KIND_RES_DAGDAG, "BUILT_IN_METHOD", 0},
    {WORD_bawas, TOKEN_KIND_RES_BAWAS, "BUILT_IN_METHOD", 0},
    {WORD_kalkula, TOKEN_KIND_RES_KALKULA, "BUILT_IN_METHOD", 0},
    {WORD_diskwento, TOKEN_KIND_RES_DISKWENTO, "BUILT_IN_METHOD", 0},
    {WORD_bayaran, TOKEN_KIND_RES_BAYARAN, "BUILT_IN_METHOD", 0},
    {WORD_ilimbag, TOKEN_KIND_RES_ILIMBAG, "BUILT_IN_METHOD", 0},
    {WORD_linisin, TOKEN_KIND_RES_LINISIN, "BUILT_IN_METHOD", 0},
    {WORD_hatian, TOKEN_KIND_RES_HATIAN, "BUILT_IN_METHOD", 0},
    {WORD_ORDER, TOKEN_KIND_RES_ORDER, "TRANSACTION_VERB", 0},
    {WORD_LUTO, TOKEN_KIND_RES_LUTO, "TRANSACTION_VERB", 0},
    {WORD_HAIN, TOKEN_KIND_RES_HAIN, "TRANSACTION_VERB", 0},
    {WORD_BAYAD, TOKEN_KIND_RES_BAYAD, "TRANSACTION_VERB", 0},
    {WORD_po, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_na, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_lang, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_at, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_nang, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_ng, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_ang, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_ay, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_nito, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_opo, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {WORD_ngani, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 1},
    {SYMBOL_DOT, TOKEN_KIND_TERMINATOR, "PUNCTUATION", 0},
    {SYMBOL_COMMA, TOKEN_KIND_SEPARATOR, "PUNCTUATION", 0},
    {SYMBOL_COLON, TOKEN_KIND_ACCESSOR, "OPERATOR", 0},
    {SYMBOL_LEFT_PAREN, TOKEN_KIND_LPAREN, "DELIMITER", 0},
    {SYMBOL_RIGHT_PAREN, TOKEN_KIND_RPAREN, "DELIMITER", 0},
    {SYMBOL_LEFT_BRACE, TOKEN_KIND_LBRACE, "DELIMITER", 0},
    {SYMBOL_RIGHT_BRACE, TOKEN_KIND_RBRACE, "DELIMITER", 0},
    {SYMBOL_PLUS, TOKEN_KIND_OP_ADD, "ARITHMETIC_OPERATOR", 0},
    {SYMBOL_MINUS, TOKEN_KIND_OP_SUB, "ARITHMETIC_OPERATOR", 0},
    {SYMBOL_STAR, TOKEN_KIND_OP_MUL, "ARITHMETIC_OPERATOR", 0},
    {SYMBOL_SLASH, TOKEN_KIND_OP_DIV, "ARITHMETIC_OPERATOR", 0},
    {SYMBOL_PERCENT, TOKEN_KIND_OP_MOD, "ARITHMETIC_OPERATOR", 0},
    {SYMBOL_CARET, TOKEN_KIND_OP_POW, "ARITHMETIC_OPERATOR", 0},
    {SYMBOL_BACKSLASH, TOKEN_KIND_OP_INTDIV, "ARITHMETIC_OPERATOR", 0},
    {SYMBOL_EQUAL, TOKEN_KIND_ASSIGNMENT, "OPERATOR", 0},
    {SYMBOL_EQUAL_EQUAL, TOKEN_KIND_REL_EQ, "RELATIONAL_OPERATOR", 0},
    {SYMBOL_BANG_EQUAL, TOKEN_KIND_REL_NEQ, "RELATIONAL_OPERATOR", 0},
    {SYMBOL_GREATER, TOKEN_KIND_REL_GT, "RELATIONAL_OPERATOR", 0},
    {SYMBOL_GREATER_EQUAL, TOKEN_KIND_REL_GTE, "RELATIONAL_OPERATOR", 0},
    {SYMBOL_LESS, TOKEN_KIND_REL_LT, "RELATIONAL_OPERATOR", 0},
    {SYMBOL_LESS_EQUAL, TOKEN_KIND_REL_LTE, "RELATIONAL_OPERATOR", 0},
    {SYMBOL_BANG, TOKEN_KIND_LOGIC_NOT, "LOGICAL_OPERATOR", 0},
    {SYMBOL_AMPERSAND_AMPERSAND, TOKEN_KIND_LOGIC_AND, "LOGICAL_OPERATOR", 0},
    {SYMBOL_PIPE_PIPE, TOKEN_KIND_LOGIC_OR, "LOGICAL_OPERATOR", 0},
    {STATE_IDENTIFIER, TOKEN_KIND_IDENTIFIER, "IDENTIFIER", 0},
    {STATE_INTEGER, TOKEN_KIND_BUO_LITERAL, "INTEGER_LITERAL", 0},
    {STATE_DECIMAL, TOKEN_KIND_NUMERO_LITERAL, "DECIMAL_LITERAL", 0},
    {STATE_STRING_END, TOKEN_KIND_SALITA_LITERAL, "STRING_LITERAL", 0},
    {STATE_CHAR_END, TOKEN_KIND_LETRA_LITERAL, "CHARACTER_LITERAL", 0}
};

int dfa_is_filipino_letter(const unsigned char *source, size_t position, size_t length) {
    return position + 1 < length && source[position] == 0xC3 &&
           (source[position + 1] == 0xB1 || source[position + 1] == 0x91);
}

int dfa_is_identifier_start(const char *source, size_t position, size_t length) {
    return dfa_is_filipino_letter((const unsigned char *)source, position, length) ||
           is_identifier_start((unsigned char)source[position]);
}

int dfa_is_identifier_part(const char *source, size_t position, size_t length) {
    return dfa_is_filipino_letter((const unsigned char *)source, position, length) ||
           is_identifier_part((unsigned char)source[position]);
}

int dfa_is_whitespace(int character) {
    return character == ' ' || character == '\t' ||
           character == '\r' || character == '\n';
}

const AcceptingState *dfa_find_accept(int state) {
    size_t i;
    for (i = 0; i < sizeof accepting_states / sizeof accepting_states[0]; ++i) {
        if (accepting_states[i].state == state) {
            return &accepting_states[i];
        }
    }
    return NULL;
}

static int dfa_step_word(int state, int character) {
    switch (state) {
    case WORD_L:
        switch (character) {
        case 'E': return WORD_LE;
        case 'U': return WORD_LU;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LE:
        switch (character) {
        case 'T': return WORD_LET;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LET:
        switch (character) {
        case 'R': return WORD_LETR;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LETR:
        switch (character) {
        case 'A': return WORD_LETRA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LETRA:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_S:
        switch (character) {
        case 'A': return WORD_SA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SA:
        switch (character) {
        case 'L': return WORD_SAL;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SAL:
        switch (character) {
        case 'I': return WORD_SALI;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SALI:
        switch (character) {
        case 'T': return WORD_SALIT;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SALIT:
        switch (character) {
        case 'A': return WORD_SALITA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SALITA:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_B:
        switch (character) {
        case 'U': return WORD_BU;
        case 'A': return WORD_BA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BU:
        switch (character) {
        case 'O': return WORD_BUO;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BUO:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_N:
        switch (character) {
        case 'U': return WORD_NU;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NU:
        switch (character) {
        case 'M': return WORD_NUM;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUM:
        switch (character) {
        case 'E': return WORD_NUME;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUME:
        switch (character) {
        case 'R': return WORD_NUMER;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUMER:
        switch (character) {
        case 'O': return WORD_NUMERO;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUMERO:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_K:
        switch (character) {
        case 'U': return WORD_KU;
        case 'A': return WORD_KA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KU:
        switch (character) {
        case 'N': return WORD_KUN;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUN:
        switch (character) {
        case 'D': return WORD_KUND;
        case 'G': return WORD_KUNG;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUND:
        switch (character) {
        case 'I': return WORD_KUNDI;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDI:
        switch (character) {
        case 'S': return WORD_KUNDIS;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDIS:
        switch (character) {
        case 'Y': return WORD_KUNDISY;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDISY:
        switch (character) {
        case 'O': return WORD_KUNDISYO;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDISYO:
        switch (character) {
        case 'N': return WORD_KUNDISYON;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDISYON:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_KUNG:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_U:
        switch (character) {
        case 'L': return WORD_UL;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_UL:
        switch (character) {
        case 'I': return WORD_ULI;
        case 'A': return WORD_ULA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ULI:
        switch (character) {
        case 'T': return WORD_ULIT;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ULIT:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_H:
        switch (character) {
        case 'I': return WORD_HI;
        case 'A': return WORD_HA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HI:
        switch (character) {
        case 'N': return WORD_HIN;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HIN:
        switch (character) {
        case 'T': return WORD_HINT;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HINT:
        switch (character) {
        case 'O': return WORD_HINTO;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HINTO:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_T:
        switch (character) {
        case 'U': return WORD_TU;
        case 'A': return WORD_TA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TU:
        switch (character) {
        case 'L': return WORD_TUL;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TUL:
        switch (character) {
        case 'O': return WORD_TULO;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TULO:
        switch (character) {
        case 'Y': return WORD_TULOY;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TULOY:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_W:
        switch (character) {
        case 'A': return WORD_WA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_WA:
        switch (character) {
        case 'L': return WORD_WAL;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_WAL:
        switch (character) {
        case 'A': return WORD_WALA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_WALA:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_TA:
        switch (character) {
        case 'M': return WORD_TAM;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TAM:
        switch (character) {
        case 'A': return WORD_TAMA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TAMA:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_M:
        switch (character) {
        case 'A': return WORD_MA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_MA:
        switch (character) {
        case 'L': return WORD_MAL;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_MAL:
        switch (character) {
        case 'I': return WORD_MALI;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_MALI:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_KA:
        switch (character) {
        case 'T': return WORD_KAT;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KAT:
        switch (character) {
        case 'U': return WORD_KATU;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATU:
        switch (character) {
        case 'M': return WORD_KATUM;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUM:
        switch (character) {
        case 'B': return WORD_KATUMB;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUMB:
        switch (character) {
        case 'A': return WORD_KATUMBA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUMBA:
        switch (character) {
        case 'S': return WORD_KATUMBAS;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUMBAS:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_ULA:
        switch (character) {
        case 'M': return WORD_ULAM;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ULAM:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_i:
        switch (character) {
        case 'l': return WORD_il;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_il:
        switch (character) {
        case 'a': return WORD_ila;
        case 'i': return WORD_ili;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ila:
        switch (character) {
        case 'g': return WORD_ilag;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilag:
        switch (character) {
        case 'a': return WORD_ilaga;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilaga:
        switch (character) {
        case 'y': return WORD_ilagay;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilagay:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_m:
        switch (character) {
        case 'e': return WORD_me;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_me:
        switch (character) {
        case 'r': return WORD_mer;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_mer:
        switch (character) {
        case 'o': return WORD_mero;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_mero:
        switch (character) {
        case 'n': return WORD_meron;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_meron:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_k:
        switch (character) {
        case 'u': return WORD_ku;
        case 'a': return WORD_ka;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ku:
        switch (character) {
        case 'm': return WORD_kum;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kum:
        switch (character) {
        case 'u': return WORD_kumu;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kumu:
        switch (character) {
        case 'h': return WORD_kumuh;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kumuh:
        switch (character) {
        case 'a': return WORD_kumuha;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kumuha:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_R:
        switch (character) {
        case 'E': return WORD_RE;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RE:
        switch (character) {
        case 'S': return WORD_RES;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RES:
        switch (character) {
        case 'I': return WORD_RESI;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RESI:
        switch (character) {
        case 'B': return WORD_RESIB;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RESIB:
        switch (character) {
        case 'O': return WORD_RESIBO;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RESIBO:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_d:
        switch (character) {
        case 'a': return WORD_da;
        case 'i': return WORD_di;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_da:
        switch (character) {
        case 'g': return WORD_dag;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dag:
        switch (character) {
        case 'd': return WORD_dagd;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dagd:
        switch (character) {
        case 'a': return WORD_dagda;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dagda:
        switch (character) {
        case 'g': return WORD_dagdag;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dagdag:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_b:
        switch (character) {
        case 'a': return WORD_ba;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ba:
        switch (character) {
        case 'w': return WORD_baw;
        case 'y': return WORD_bay;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_baw:
        switch (character) {
        case 'a': return WORD_bawa;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bawa:
        switch (character) {
        case 's': return WORD_bawas;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bawas:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_ka:
        switch (character) {
        case 'l': return WORD_kal;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kal:
        switch (character) {
        case 'k': return WORD_kalk;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalk:
        switch (character) {
        case 'u': return WORD_kalku;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalku:
        switch (character) {
        case 'l': return WORD_kalkul;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalkul:
        switch (character) {
        case 'a': return WORD_kalkula;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalkula:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_di:
        switch (character) {
        case 's': return WORD_dis;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dis:
        switch (character) {
        case 'k': return WORD_disk;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_disk:
        switch (character) {
        case 'w': return WORD_diskw;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskw:
        switch (character) {
        case 'e': return WORD_diskwe;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwe:
        switch (character) {
        case 'n': return WORD_diskwen;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwen:
        switch (character) {
        case 't': return WORD_diskwent;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwent:
        switch (character) {
        case 'o': return WORD_diskwento;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwento:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_bay:
        switch (character) {
        case 'a': return WORD_baya;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_baya:
        switch (character) {
        case 'r': return WORD_bayar;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bayar:
        switch (character) {
        case 'a': return WORD_bayara;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bayara:
        switch (character) {
        case 'n': return WORD_bayaran;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bayaran:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_ili:
        switch (character) {
        case 'm': return WORD_ilim;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilim:
        switch (character) {
        case 'b': return WORD_ilimb;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilimb:
        switch (character) {
        case 'a': return WORD_ilimba;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilimba:
        switch (character) {
        case 'g': return WORD_ilimbag;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilimbag:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_l:
        switch (character) {
        case 'i': return WORD_li;
        case 'a': return WORD_la;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_li:
        switch (character) {
        case 'n': return WORD_lin;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lin:
        switch (character) {
        case 'i': return WORD_lini;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lini:
        switch (character) {
        case 's': return WORD_linis;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_linis:
        switch (character) {
        case 'i': return WORD_linisi;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_linisi:
        switch (character) {
        case 'n': return WORD_linisin;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_linisin:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_h:
        switch (character) {
        case 'a': return WORD_ha;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ha:
        switch (character) {
        case 't': return WORD_hat;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hat:
        switch (character) {
        case 'i': return WORD_hati;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hati:
        switch (character) {
        case 'a': return WORD_hatia;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hatia:
        switch (character) {
        case 'n': return WORD_hatian;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hatian:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_O:
        switch (character) {
        case 'R': return WORD_OR;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_OR:
        switch (character) {
        case 'D': return WORD_ORD;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ORD:
        switch (character) {
        case 'E': return WORD_ORDE;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ORDE:
        switch (character) {
        case 'R': return WORD_ORDER;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ORDER:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_LU:
        switch (character) {
        case 'T': return WORD_LUT;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LUT:
        switch (character) {
        case 'O': return WORD_LUTO;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LUTO:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_HA:
        switch (character) {
        case 'I': return WORD_HAI;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HAI:
        switch (character) {
        case 'N': return WORD_HAIN;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HAIN:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_BA:
        switch (character) {
        case 'Y': return WORD_BAY;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BAY:
        switch (character) {
        case 'A': return WORD_BAYA;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BAYA:
        switch (character) {
        case 'D': return WORD_BAYAD;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BAYAD:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_p:
        switch (character) {
        case 'o': return WORD_po;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_po:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_n:
        switch (character) {
        case 'a': return WORD_na;
        case 'g': return WORD_ng;
        case 'i': return WORD_ni;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_na:
        switch (character) {
        case 'n': return WORD_nan;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_la:
        switch (character) {
        case 'n': return WORD_lan;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lan:
        switch (character) {
        case 'g': return WORD_lang;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lang:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_a:
        switch (character) {
        case 't': return WORD_at;
        case 'n': return WORD_an;
        case 'y': return WORD_ay;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_at:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_nan:
        switch (character) {
        case 'g': return WORD_nang;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nang:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_ng:
        switch (character) {
        case 'a': return WORD_nga;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nga:
        switch (character) {
        case 'n': return WORD_ngan;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ngan:
        switch (character) {
        case 'i': return WORD_ngani;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ngani:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_an:
        switch (character) {
        case 'g': return WORD_ang;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ang:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_ay:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_ni:
        switch (character) {
        case 't': return WORD_nit;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nit:
        switch (character) {
        case 'o': return WORD_nito;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nito:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case WORD_o:
        switch (character) {
        case 'p': return WORD_op;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_op:
        switch (character) {
        case 'o': return WORD_opo;
        default: return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_opo:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    default:
        return -1;
    }
}

static int dfa_step_symbol(int state, int character) {
    switch (state) {
    case SYMBOL_EQUAL:
        if (character == '=') return SYMBOL_EQUAL_EQUAL;
        return -1;
    case SYMBOL_BANG:
        if (character == '=') return SYMBOL_BANG_EQUAL;
        return -1;
    case SYMBOL_GREATER:
        if (character == '=') return SYMBOL_GREATER_EQUAL;
        return -1;
    case SYMBOL_LESS:
        if (character == '=') return SYMBOL_LESS_EQUAL;
        return -1;
    case SYMBOL_AMPERSAND:
        if (character == '&') return SYMBOL_AMPERSAND_AMPERSAND;
        return -1;
    case SYMBOL_PIPE:
        if (character == '|') return SYMBOL_PIPE_PIPE;
        return -1;
    default:
        return -1;
    }
}

int dfa_step(int state, int character) {
    if (state >= WORD_L && state <= WORD_ngani) {
        return dfa_step_word(state, character);
    }
    if (state >= SYMBOL_DOT && state <= SYMBOL_PIPE_PIPE) {
        return dfa_step_symbol(state, character);
    }

    switch (state) {
    case STATE_START:
        switch (character) {
        case 'L': return WORD_L;
        case 'S': return WORD_S;
        case 'B': return WORD_B;
        case 'N': return WORD_N;
        case 'K': return WORD_K;
        case 'U': return WORD_U;
        case 'H': return WORD_H;
        case 'T': return WORD_T;
        case 'W': return WORD_W;
        case 'M': return WORD_M;
        case 'i': return WORD_i;
        case 'm': return WORD_m;
        case 'k': return WORD_k;
        case 'R': return WORD_R;
        case 'd': return WORD_d;
        case 'b': return WORD_b;
        case 'l': return WORD_l;
        case 'h': return WORD_h;
        case 'O': return WORD_O;
        case 'p': return WORD_p;
        case 'n': return WORD_n;
        case 'a': return WORD_a;
        case 'o': return WORD_o;
        case '.': return SYMBOL_DOT;
        case ',': return SYMBOL_COMMA;
        case ':': return SYMBOL_COLON;
        case '(': return SYMBOL_LEFT_PAREN;
        case ')': return SYMBOL_RIGHT_PAREN;
        case '{': return SYMBOL_LEFT_BRACE;
        case '}': return SYMBOL_RIGHT_BRACE;
        case '+': return SYMBOL_PLUS;
        case '-': return SYMBOL_MINUS;
        case '*': return SYMBOL_STAR;
        case '/': return SYMBOL_SLASH;
        case '%': return SYMBOL_PERCENT;
        case '^': return SYMBOL_CARET;
        case '\\': return SYMBOL_BACKSLASH;
        case '=': return SYMBOL_EQUAL;
        case '!': return SYMBOL_BANG;
        case '>': return SYMBOL_GREATER;
        case '<': return SYMBOL_LESS;
        case '&': return SYMBOL_AMPERSAND;
        case '|': return SYMBOL_PIPE;
        case '"': return STATE_STRING;
        case '\'': return STATE_CHAR;
        case '@': return STATE_COMMENT;
        default:
            switch (dfa_char_class(character)) {
            case CLASS_ALPHA:      return STATE_IDENTIFIER;
            case CLASS_DIGIT:      return STATE_INTEGER;
            case CLASS_WHITESPACE: return STATE_START;
            default:               return STATE_ERROR;
            }
        }
    case STATE_IDENTIFIER:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case STATE_INTEGER:
        if (dfa_char_class(character) == CLASS_DIGIT) return STATE_INTEGER;
        if (character == '.') return STATE_DECIMAL;
        return -1;
    case STATE_DECIMAL:
        return dfa_char_class(character) == CLASS_DIGIT ? STATE_DECIMAL : -1;
    case STATE_STRING:
        return character == '"' ? STATE_STRING_END : STATE_STRING;
    case STATE_CHAR:
        return character == '\'' ? STATE_CHAR_END : STATE_CHAR;
    case STATE_STRING_END:
    case STATE_CHAR_END:
        return -1;
    case STATE_COMMENT:
        return STATE_COMMENT;
    default:
        return -1;
    }
}

const char *dfa_token_kind_name(TokenKind kind) {
    switch (kind) {
    case TOKEN_KIND_ACCESSOR: return "TOKEN_ACCESSOR";
    case TOKEN_KIND_ASSIGNMENT: return "TOKEN_ASSIGNMENT";
    case TOKEN_KIND_BUO_LITERAL: return "TOKEN_BUO_LITERAL";
    case TOKEN_KIND_IDENTIFIER: return "TOKEN_IDENTIFIER";
    case TOKEN_KIND_KW_BUO: return "TOKEN_KW_BUO";
    case TOKEN_KIND_KW_HINTO: return "TOKEN_KW_HINTO";
    case TOKEN_KIND_KW_KUNDI: return "TOKEN_KW_KUNDI";
    case TOKEN_KIND_KW_KUNDISYON: return "TOKEN_KW_KUNDISYON";
    case TOKEN_KIND_KW_KUNG: return "TOKEN_KW_KUNG";
    case TOKEN_KIND_KW_LETRA: return "TOKEN_KW_LETRA";
    case TOKEN_KIND_KW_NUMERO: return "TOKEN_KW_NUMERO";
    case TOKEN_KIND_KW_SALITA: return "TOKEN_KW_SALITA";
    case TOKEN_KIND_KW_TULOY: return "TOKEN_KW_TULOY";
    case TOKEN_KIND_KW_ULIT: return "TOKEN_KW_ULIT";
    case TOKEN_KIND_KW_WALA: return "TOKEN_KW_WALA";
    case TOKEN_KIND_LBRACE: return "TOKEN_LBRACE";
    case TOKEN_KIND_LETRA_LITERAL: return "TOKEN_LETRA_LITERAL";
    case TOKEN_KIND_LOGIC_AND: return "TOKEN_LOGIC_AND";
    case TOKEN_KIND_LOGIC_NOT: return "TOKEN_LOGIC_NOT";
    case TOKEN_KIND_LOGIC_OR: return "TOKEN_LOGIC_OR";
    case TOKEN_KIND_LPAREN: return "TOKEN_LPAREN";
    case TOKEN_KIND_NOISE_WORD: return "TOKEN_NOISE_WORD";
    case TOKEN_KIND_NUMERO_LITERAL: return "TOKEN_NUMERO_LITERAL";
    case TOKEN_KIND_OP_ADD: return "TOKEN_OP_ADD";
    case TOKEN_KIND_OP_DIV: return "TOKEN_OP_DIV";
    case TOKEN_KIND_OP_INTDIV: return "TOKEN_OP_INTDIV";
    case TOKEN_KIND_OP_MOD: return "TOKEN_OP_MOD";
    case TOKEN_KIND_OP_MUL: return "TOKEN_OP_MUL";
    case TOKEN_KIND_OP_POW: return "TOKEN_OP_POW";
    case TOKEN_KIND_OP_SUB: return "TOKEN_OP_SUB";
    case TOKEN_KIND_RBRACE: return "TOKEN_RBRACE";
    case TOKEN_KIND_REL_EQ: return "TOKEN_REL_EQ";
    case TOKEN_KIND_REL_GT: return "TOKEN_REL_GT";
    case TOKEN_KIND_REL_GTE: return "TOKEN_REL_GTE";
    case TOKEN_KIND_REL_LT: return "TOKEN_REL_LT";
    case TOKEN_KIND_REL_LTE: return "TOKEN_REL_LTE";
    case TOKEN_KIND_REL_NEQ: return "TOKEN_REL_NEQ";
    case TOKEN_KIND_RES_BAYAD: return "TOKEN_RES_BAYAD";
    case TOKEN_KIND_RES_HAIN: return "TOKEN_RES_HAIN";
    case TOKEN_KIND_RES_KATUMBAS: return "TOKEN_RES_KATUMBAS";
    case TOKEN_KIND_RES_LUTO: return "TOKEN_RES_LUTO";
    case TOKEN_KIND_RES_MALI: return "TOKEN_RES_MALI";
    case TOKEN_KIND_RES_ORDER: return "TOKEN_RES_ORDER";
    case TOKEN_KIND_RES_RESIBO: return "TOKEN_RES_RESIBO";
    case TOKEN_KIND_RES_TAMA: return "TOKEN_RES_TAMA";
    case TOKEN_KIND_RES_ULAM: return "TOKEN_RES_ULAM";
    case TOKEN_KIND_RES_BAWAS: return "TOKEN_RES_bawas";
    case TOKEN_KIND_RES_BAYARAN: return "TOKEN_RES_bayaran";
    case TOKEN_KIND_RES_DAGDAG: return "TOKEN_RES_dagdag";
    case TOKEN_KIND_RES_DISKWENTO: return "TOKEN_RES_diskwento";
    case TOKEN_KIND_RES_HATIAN: return "TOKEN_RES_hatian";
    case TOKEN_KIND_RES_ILAGAY: return "TOKEN_RES_ilagay";
    case TOKEN_KIND_RES_ILIMBAG: return "TOKEN_RES_ilimbag";
    case TOKEN_KIND_RES_KALKULA: return "TOKEN_RES_kalkula";
    case TOKEN_KIND_RES_KUMUHA: return "TOKEN_RES_kumuha";
    case TOKEN_KIND_RES_LINISIN: return "TOKEN_RES_linisin";
    case TOKEN_KIND_RES_MERON: return "TOKEN_RES_meron";
    case TOKEN_KIND_RPAREN: return "TOKEN_RPAREN";
    case TOKEN_KIND_SALITA_LITERAL: return "TOKEN_SALITA_LITERAL";
    case TOKEN_KIND_SEPARATOR: return "TOKEN_SEPARATOR";
    case TOKEN_KIND_TERMINATOR: return "TOKEN_TERMINATOR";
    case TOKEN_KIND_INVALID: return "INVALID";
    case TOKEN_KIND_LINE_COMMENT: return "LINE_COMMENT";
    }
    return "TOKEN_UNKNOWN";
}

void dfa_write_symbol_span(FILE *out, const char *type, const char *lexeme, size_t length) {
    const char *cursor;
    size_t displayed_width = 0;
    size_t i;
    fprintf(out, "%-24s | ", type);
    for (i = 0, cursor = lexeme; i < length; ++i, ++cursor) {
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
    fputc('\n', out);
}

void dfa_write_symbol(FILE *out, TokenKind kind, const char *lexeme) {
    dfa_write_symbol_span(out, dfa_token_kind_name(kind), lexeme, strlen(lexeme));
}

void dfa_write_acceptance_span(FILE *out, const char *lexeme, size_t length, TokenKind kind, const char *classification, int state) {
    if (out != NULL) {
        size_t i;
        fprintf(out, "ACCEPT | [");
        for (i = 0; i < length; ++i) {
            fputc(lexeme[i], out);
        }
        fprintf(out, "] | State: q%d | Token: %s | Class: %s\n",
                state, dfa_token_kind_name(kind), classification);
    }
}

void dfa_write_acceptance(FILE *out, const char *lexeme, TokenKind kind, const char *classification, int state) {
    dfa_write_acceptance_span(out, lexeme, strlen(lexeme), kind, classification, state);
}

void dfa_report_error(FILE *out, size_t line, size_t column, const char *message) {
    char full[512];
    snprintf(full, sizeof full, "Lexical error (line %zu, column %zu): %s", line, column, message);
    fprintf(stderr, "%s\n", full);                    /* terminal */
    if (out != NULL) {
        dfa_write_symbol(out, TOKEN_KIND_INVALID, full); /* Symbol Table */
    }
}


/* ================================================================= */
/* 5. LEXER IMPLEMENTATION                                           */
/* ================================================================= */

static void write_trace_str(FILE *out, const char *kind, size_t line, size_t column, const char *display, const char *classification) {
    if (out == NULL) return;
    fprintf(out, "%s | Line %zu, Column %zu | Character: [%s] | Class: %s\n", kind, line, column, display, classification);
}

static void write_transition_str(FILE *out, size_t line, size_t column, int from, const char *display, int to) {
    if (out == NULL) return;
    fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%s]--> q%d\n", line, column, from, display, to);
}

static void write_trace(FILE *out, const char *kind, size_t line, size_t column, int c, const char *classification) {
    if (out == NULL) return;
    const char *display = c == '\n' ? "\\n" : c == '\r' ? "\\r" : c == '\t' ? "\\t" : NULL;
    if (display != NULL) {
        fprintf(out, "%s | Line %zu, Column %zu | Character: [%s] | Class: %s\n", kind, line, column, display, classification);
    } else {
        fprintf(out, "%s | Line %zu, Column %zu | Character: [%c] | Class: %s\n", kind, line, column, c, classification);
    }
}

static void write_transition(FILE *out, size_t line, size_t column, int from, int c, int to) {
    if (out == NULL) return;
    const char *display = c == '\n' ? "\\n" : c == '\r' ? "\\r" : c == '\t' ? "\\t" : NULL;
    if (display != NULL) {
        fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%s]--> q%d\n", line, column, from, display, to);
    } else {
        fprintf(out, "DFA | Line %zu, Column %zu | q%d --[%c]--> q%d\n", line, column, from, c, to);
    }
}

static int append_char(char **buffer, size_t *length, size_t *capacity, int c) {
    char *grown;
    size_t new_capacity;
    if (*length + 1 >= *capacity) {
        new_capacity = *capacity == 0 ? 64 : *capacity * 2;
        grown = realloc(*buffer, new_capacity);
        if (grown == NULL) return 0;
        *buffer = grown;
        *capacity = new_capacity;
    }
    (*buffer)[(*length)++] = (char)c;
    (*buffer)[*length] = '\0';
    return 1;
}

static int valid_source_extension(const char *path) {
    size_t length = strlen(path);
    return length >= 5 && path[length - 5] == '.' &&
           path[length - 4] == 'r' && path[length - 3] == 'i' &&
           path[length - 2] == 'c' && path[length - 1] == 'e';
}

static char read_error_message[256] = "";

#define READ_FAIL(...) \
    do { \
        snprintf(read_error_message, sizeof read_error_message, __VA_ARGS__); \
        fprintf(stderr, "%s\n", read_error_message); \
    } while (0)

int lexer_read_source(const char *path, char **source, size_t *length) {
    FILE *file;
    struct stat metadata;
    size_t capacity = 0;
    size_t line = 1;
    size_t column = 1;
    int c;

    if (!valid_source_extension(path)) {
        READ_FAIL("Input must use the .rice file extension.");
        return 0;
    }
    if (stat(path, &metadata) != 0 || !S_ISREG(metadata.st_mode)) {
        READ_FAIL("Input must be an existing regular text file.");
        return 0;
    }
    file = fopen(path, "rb");
    if (file == NULL) {
        READ_FAIL("Cannot open input file '%s': %s", path, strerror(errno));
        return 0;
    }
    while ((c = fgetc(file)) != EOF) {
        if ((unsigned char)c == 0xC3) {
            int c2 = fgetc(file);
            if (c2 == 0xB1 || c2 == 0x91) {
                if (!append_char(source, length, &capacity, c) ||
                    !append_char(source, length, &capacity, c2)) {
                    READ_FAIL("Not enough memory while reading input.");
                    fclose(file);
                    free(*source); *source = NULL; *length = 0;
                    return 0;
                }
                ++column;
                continue;
            } else {
                if (c2 != EOF) ungetc(c2, file);
                READ_FAIL("Input contains unsupported character at line %zu, column %zu.", line, column);
                fclose(file);
                free(*source); *source = NULL; *length = 0;
                return 0;
            }
        }
        if (c == '\0' || (c < 32 && c != '\t' && c != '\r' && c != '\n') || c > 126) {
            READ_FAIL("Input is not supported plain text at line %zu, column %zu.", line, column);
            fclose(file);
            free(*source); *source = NULL; *length = 0;
            return 0;
        }
        if (!append_char(source, length, &capacity, c)) {
            READ_FAIL("Not enough memory while reading input.");
            fclose(file);
            free(*source); *source = NULL; *length = 0;
            return 0;
        }
        if (c == '\n') { ++line; column = 1; }
        else { ++column; }
    }
    if (ferror(file)) {
        READ_FAIL("Error while reading input file '%s'.", path);
        fclose(file);
        free(*source); *source = NULL; *length = 0;
        return 0;
    }
    if (fclose(file) != 0) {
        READ_FAIL("Error closing input file '%s'.", path);
        return 0;
    }
    return 1;
}

int lexer_scan(const char *source, size_t length, FILE *log, FILE *tokens) {
    size_t pos = 0;
    size_t line = 1;
    size_t column = 1;
    int errors = 0;

    while (pos < length) {
        size_t token_line = line;
        size_t token_column = column;
        int c = (unsigned char)source[pos];

        if (dfa_char_class(c) == CLASS_WHITESPACE) {
            int next = dfa_step(STATE_START, c);
            write_trace(log, "SCAN", line, column, c, dfa_char_class_label(c));
            write_transition(log, line, column, STATE_START, c, next);
            ++pos;
            if (c == '\n') { ++line; column = 1; }
            else { ++column; }
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
                    state = next; pos += 2; column += 1;
                } else {
                    int current = (unsigned char)source[pos];
                    int next = dfa_step(state, current);
                    write_trace(log, "SCAN", line, column, current, state == STATE_START ? "COMMENT_MARKER" : "COMMENT");
                    write_transition(log, line, column, state, current, next);
                    state = next; ++pos; ++column;
                }
            }
            comment_length = pos - comment_start;
            if (comment_length > 0 && source[comment_start + comment_length - 1] == '\r') {
                --comment_length;
            }
            dfa_write_symbol_span(tokens, "LINE_COMMENT", source + comment_start, comment_length);
            dfa_write_acceptance_span(log, source + comment_start, comment_length, TOKEN_KIND_LINE_COMMENT, "COMMENT", state);
            continue;
        }

        if (dfa_char_class(c) == CLASS_UNDERSCORE) {
            size_t bad_start = pos;
            write_trace(log, "SCAN", line, column, c, dfa_char_class_label(c));
            write_transition(log, line, column, STATE_START, c, STATE_ERROR);
            while (pos < length && dfa_is_identifier_part(source, pos, length)) {
                if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) pos += 2;
                else pos += 1;
                ++column;
            }
            {
                char msg[256];
                snprintf(msg, sizeof msg, "Identifier must not begin with an underscore: '%.*s'",
                         (int)(pos - bad_start), source + bad_start);
                dfa_report_error(tokens, token_line, token_column, msg);
            }
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
                    state = next; pos += 2; column += 1;
                } else {
                    int current = (unsigned char)source[pos];
                    int next = dfa_step(state, current);
                    if (next < 0) next = STATE_IDENTIFIER;
                    write_trace(log, "SCAN", line, column, current, dfa_char_class_label(current));
                    write_transition(log, line, column, state, current, next);
                    state = next; ++pos; ++column;
                }
                // For intermediate accepting state
                if (log != NULL && state != STATE_IDENTIFIER && dfa_find_accept(state) != NULL &&
                    pos < length && dfa_is_identifier_part(source, pos, length)) {
                    const AcceptingState *partial = dfa_find_accept(state);
                    fprintf(log, "PASS   | [%.*s] | State: q%d | Token: %s | Class: %s\n",
                            (int)(pos - start), source + start, state,
                            dfa_token_kind_name(partial->kind), partial->classification);
                }
            }
            {
                const AcceptingState *accepted = dfa_find_accept(state);
                size_t token_length = pos - start;
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column, "Out of memory allocating identifier token.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                if (accepted == NULL) {
                    dfa_write_symbol(tokens, TOKEN_KIND_IDENTIFIER, lexeme);
                    dfa_write_acceptance(log, lexeme, TOKEN_KIND_IDENTIFIER, "IDENTIFIER", state);
                } else {
                    dfa_write_symbol(tokens, accepted->kind, lexeme);
                    dfa_write_acceptance(log, lexeme, accepted->kind, accepted->classification, state);
                }
                free(lexeme);
            }
            continue;
        }

        if (dfa_char_class(c) == CLASS_DIGIT) {
            size_t start = pos;
            int state = STATE_START;
            while (pos < length && dfa_char_class(source[pos]) == CLASS_DIGIT) {
                int current = (unsigned char)source[pos];
                int next = dfa_step(state, current);
                if (next < 0) next = STATE_INTEGER;
                write_trace(log, "SCAN", line, column, current, dfa_char_class_label(current));
                write_transition(log, line, column, state, current, next);
                state = next; ++pos; ++column;
            }
            if (pos + 1 < length && source[pos] == '.' && dfa_char_class(source[pos + 1]) == CLASS_DIGIT) {
                int next = dfa_step(state, '.');
                write_trace(log, "SCAN", line, column, '.', "DECIMAL_POINT");
                write_transition(log, line, column, state, '.', next);
                state = next; ++pos; ++column;
                while (pos < length && dfa_char_class(source[pos]) == CLASS_DIGIT) {
                    int current = (unsigned char)source[pos];
                    next = dfa_step(state, current);
                    write_trace(log, "SCAN", line, column, current, dfa_char_class_label(current));
                    write_transition(log, line, column, state, current, next);
                    state = next; ++pos; ++column;
                }
            }
            if (pos < length && dfa_is_identifier_part(source, pos, length)) {
                /* number immediately followed by letters/underscore: one bad lexeme, e.g. 123user */
                char msg[256];
                while (pos < length && dfa_is_identifier_part(source, pos, length)) {
                    if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) pos += 2;
                    else pos += 1;
                    ++column;
                }
                write_trace(log, "SCAN", token_line, token_column, source[start], "INVALID_IDENTIFIER");
                snprintf(msg, sizeof msg, "Identifier must not begin with a digit: '%.*s'",
                         (int)(pos - start), source + start);
                dfa_report_error(tokens, token_line, token_column, msg);
                if (log != NULL) {
                    fprintf(log, "REJECT | [%.*s] | State: q%d | No accepting token\n",
                            (int)(pos - start), source + start, STATE_ERROR);
                }
                ++errors;
                continue;
            }
            {
                size_t token_length = pos - start;
                const AcceptingState *accepted = dfa_find_accept(state);
                if (accepted == NULL) {
                    dfa_report_error(tokens, token_line, token_column, "Invalid numeric literal.");
                    ++errors;
                    continue;
                }
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column, "Out of memory allocating numeric literal.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                dfa_write_symbol(tokens, accepted->kind, lexeme);
                dfa_write_acceptance(log, lexeme, accepted->kind, accepted->classification, state);
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
            state = next; ++pos; ++column;

            if (pos >= length || source[pos] == '\n' || source[pos] == '\r') {
                dfa_report_error(tokens, token_line, token_column, "Unterminated character literal.");
                ++errors;
                continue;
            }

            if (source[pos] == '\'') {
                write_trace(log, "SCAN", line, column, '\'', "CHAR_DELIMITER");
                write_transition(log, line, column, state, '\'', STATE_ERROR);
                dfa_report_error(tokens, token_line, token_column, "Empty character literal is not allowed.");
                ++errors; ++pos; ++column;
                continue;
            }

            if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) {
                char letter_buf[3] = {source[pos], source[pos + 1], '\0'};
                write_trace_str(log, "SCAN", line, column, letter_buf, "CHAR_CONTENT");
                write_transition_str(log, line, column, state, letter_buf, state);
                pos += 2; column += 1;
            } else {
                int ch = (unsigned char)source[pos];
                write_trace(log, "SCAN", line, column, ch, "CHAR_CONTENT");
                write_transition(log, line, column, state, ch, state);
                pos += 1; column += 1;
            }

            if (pos < length && source[pos] == '\'') {
                int end_state = dfa_step(state, '\'');                  /* STATE_CHAR_END */
                const AcceptingState *accepted = dfa_find_accept(end_state);
                write_trace(log, "SCAN", line, column, '\'', "CHAR_DELIMITER");
                write_transition(log, line, column, state, '\'', end_state);
                ++pos; ++column;

                size_t token_length = pos - start;
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column, "Out of memory allocating character literal.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                dfa_write_symbol(tokens, accepted->kind, lexeme);
                dfa_write_acceptance(log, lexeme, accepted->kind, accepted->classification, end_state);
                free(lexeme);
            } else {
                while (pos < length && source[pos] != '\'' && source[pos] != '\n') {
                    ++pos; ++column;
                }
                if (pos < length && source[pos] == '\'') {
                    ++pos; ++column;
                    dfa_report_error(tokens, token_line, token_column, "Character literal may only contain a single character.");
                } else {
                    dfa_report_error(tokens, token_line, token_column, "Unterminated character literal.");
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
            ++pos; ++column;
            while (pos < length && source[pos] != '\n') {
                if (dfa_is_filipino_letter((const unsigned char *)source, pos, length)) {
                    char letter_buf[3] = {source[pos], source[pos + 1], '\0'};
                    write_trace_str(log, "SCAN", line, column, letter_buf, "STRING_CHARACTER");
                    write_transition_str(log, line, column, state, letter_buf, state);
                    pos += 2; column += 1;
                } else {
                    int current = (unsigned char)source[pos];
                    int next = dfa_step(state, current);
                    write_trace(log, "SCAN", line, column, current, current == '"' ? "STRING_DELIMITER" : "STRING_CHARACTER");
                    write_transition(log, line, column, state, current, next);
                    state = next; ++pos; ++column;
                    if (current == '"') { terminated = 1; break; }
                }
            }
            if (!terminated) {
                dfa_report_error(tokens, token_line, token_column, "Unterminated string literal.");
                ++errors;
                continue;
            }
            {
                size_t token_length = pos - start;
                char *lexeme = malloc(token_length + 1);
                if (lexeme == NULL) {
                    dfa_report_error(tokens, token_line, token_column, "Out of memory allocating string literal.");
                    ++errors;
                    continue;
                }
                memcpy(lexeme, source + start, token_length);
                lexeme[token_length] = '\0';
                dfa_write_symbol(tokens, TOKEN_KIND_SALITA_LITERAL, lexeme);
                dfa_write_acceptance(log, lexeme, TOKEN_KIND_SALITA_LITERAL, "STRING_LITERAL", state);
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

            write_trace(log, "SCAN", line, column, c, dfa_char_class_label(c));
            if (state < 0) state = STATE_ERROR;
            if (state == STATE_ERROR) {
                char msg[96];
                write_transition(log, line, column, STATE_START, c, state);
                if (c >= 32 && c <= 126) {
                    snprintf(msg, sizeof msg, "Unrecognized character '%c' (ASCII %d).", c, c);
                } else {
                    snprintf(msg, sizeof msg, "Unrecognized character (ASCII %d).", c);
                }
                dfa_report_error(tokens, token_line, token_column, msg);
                ++errors; ++pos; ++column;
                continue;
            }

            write_transition(log, line, column, STATE_START, c, state);
            if (pos + 1 < length) {
                int second = (unsigned char)source[pos + 1];
                int combined = dfa_step(state, second);
                if (combined >= 0 && dfa_find_accept(combined) != NULL) {
                    write_trace(log, "SCAN", line, column + 1, second, dfa_char_class_label(second));
                    write_transition(log, line, column + 1, state, second, combined);
                    next_state = combined;
                    lexeme[1] = (char)second;
                    token_length = 2;
                }
            }
            accepted = dfa_find_accept(next_state);
            if (accepted == NULL) {
                dfa_report_error(tokens, token_line, token_column, "Invalid symbol transition.");
                if (log != NULL) {
                    fprintf(log, "REJECT | [%s] | State: q%d | No accepting token\n", lexeme, next_state);
                }
                ++errors;
            } else {
                dfa_write_symbol(tokens, accepted->kind, lexeme);
                dfa_write_acceptance(log, lexeme, accepted->kind, accepted->classification, next_state);
            }
            pos += token_length;
            column += token_length;
        }
    }
    return errors;
}


/* ================================================================= */
/* 6. MAIN PROGRAM                                                   */
/* ================================================================= */

static int copy_stream(FILE *source, FILE *destination) {
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

int main(int argc, char **argv) {
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

    if (!valid_source_extension(argv[1])) {
    fprintf(stderr, "%s\n", EXTENSION_ERROR_MESSAGE);
    symbol_table = fopen(SYMBOL_TABLE_FILE, "w");
    if (symbol_table != NULL) {
        fputs(EXTENSION_ERROR_MESSAGE, symbol_table);
        fputc('\n', symbol_table);
        fclose(symbol_table);
    }
    return EXIT_FAILURE;
    }

    if (!lexer_read_source(argv[1], &source, &source_length)) {
        symbol_table = fopen(SYMBOL_TABLE_FILE, "w");
        if (symbol_table != NULL) {
            // Remove the table headers and dfa_write_symbol, 
            // and write the raw error message directly like the extension check:
            fputs(read_error_message, symbol_table);
            fputc('\n', symbol_table);
            fclose(symbol_table);
        }
        return EXIT_FAILURE;
    }

    scan_log = tmpfile();
    token_table = tmpfile();
    if (scan_log == NULL || token_table == NULL) {
        fprintf(stderr, "Cannot create temporary output files: %s\n", strerror(errno));
        if (scan_log != NULL) fclose(scan_log);
        if (token_table != NULL) fclose(token_table);
        free(source);
        return EXIT_FAILURE;
    }
    errors = lexer_scan(source, source_length, scan_log, token_table);
    free(source);
    if (fflush(scan_log) != 0 || fflush(token_table) != 0) {
        fprintf(stderr, "Error buffering temporary lexer output.\n");
        fclose(scan_log);
        fclose(token_table);
        return EXIT_FAILURE;
    }
    symbol_table = fopen(SYMBOL_TABLE_FILE, "w");
    if (symbol_table == NULL) {
        fprintf(stderr, "Cannot create '%s': %s\n", SYMBOL_TABLE_FILE, strerror(errno));
        fclose(scan_log);
        fclose(token_table);
        return EXIT_FAILURE;
    }
    fprintf(symbol_table, "%-24s | %s\n", "TOKEN TYPE", "LEXEME");
    fprintf(symbol_table, "------------------------ | ----------------------------------------\n");
    if (!copy_stream(token_table, symbol_table)) {
        fprintf(stderr, "Error writing token table to '%s'.\n", SYMBOL_TABLE_FILE);
        fclose(scan_log); fclose(token_table); fclose(symbol_table);
        return EXIT_FAILURE;
    }
    fprintf(symbol_table, "\nCHARACTER-BY-CHARACTER SCAN AND DFA TRANSITIONS\n");
    fprintf(symbol_table, "ACCEPT rows show the final accepting state and token category.\n"
                          "REJECT rows show lexemes that reached no accepting token state.\n"
                          "PASS rows show intermediate accepting states for multi-character lexemes.\n");
    if (!copy_stream(scan_log, symbol_table)) {
        fprintf(stderr, "Error writing DFA trace to '%s'.\n", SYMBOL_TABLE_FILE);
        fclose(scan_log); fclose(token_table); fclose(symbol_table);
        return EXIT_FAILURE;
    }
    if (fclose(scan_log) != 0) {
        fprintf(stderr, "Error closing temporary DFA trace.\n");
        fclose(token_table); fclose(symbol_table);
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
    if (errors != 0) {
        fprintf(stderr, "Lexical analysis completed with %d error(s). See %s.\n", errors, SYMBOL_TABLE_FILE);
        return EXIT_FAILURE;
    }
    printf("Lexical analysis completed successfully. Symbol table written to %s.\n", SYMBOL_TABLE_FILE);
    return EXIT_SUCCESS;
}