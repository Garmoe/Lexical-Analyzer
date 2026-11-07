#include "dfa.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

enum {
    WORD_L = 1,
    WORD_LE = 2,
    WORD_LET = 3,
    WORD_LETR = 4,
    WORD_LETRA = 5,
    WORD_S = 6,
    WORD_SA = 7,
    WORD_SAL = 8,
    WORD_SALI = 9,
    WORD_SALIT = 10,
    WORD_SALITA = 11,
    WORD_B = 12,
    WORD_BU = 13,
    WORD_BUO = 14,
    WORD_N = 15,
    WORD_NU = 16,
    WORD_NUM = 17,
    WORD_NUME = 18,
    WORD_NUMER = 19,
    WORD_NUMERO = 20,
    WORD_K = 21,
    WORD_KU = 22,
    WORD_KUN = 23,
    WORD_KUND = 24,
    WORD_KUNDI = 25,
    WORD_KUNDIS = 26,
    WORD_KUNDISY = 27,
    WORD_KUNDISYO = 28,
    WORD_KUNDISYON = 29,
    WORD_KUNG = 30,
    WORD_U = 31,
    WORD_UL = 32,
    WORD_ULI = 33,
    WORD_ULIT = 34,
    WORD_H = 35,
    WORD_HI = 36,
    WORD_HIN = 37,
    WORD_HINT = 38,
    WORD_HINTO = 39,
    WORD_T = 40,
    WORD_TU = 41,
    WORD_TUL = 42,
    WORD_TULO = 43,
    WORD_TULOY = 44,
    WORD_W = 45,
    WORD_WA = 46,
    WORD_WAL = 47,
    WORD_WALA = 48,
    WORD_TA = 49,
    WORD_TAM = 50,
    WORD_TAMA = 51,
    WORD_M = 52,
    WORD_MA = 53,
    WORD_MAL = 54,
    WORD_MALI = 55,
    WORD_KA = 56,
    WORD_KAT = 57,
    WORD_KATU = 58,
    WORD_KATUM = 59,
    WORD_KATUMB = 60,
    WORD_KATUMBA = 61,
    WORD_KATUMBAS = 62,
    WORD_ULA = 63,
    WORD_ULAM = 64,
    WORD_i = 65,
    WORD_il = 66,
    WORD_ila = 67,
    WORD_ilag = 68,
    WORD_ilaga = 69,
    WORD_ilagay = 70,
    WORD_m = 71,
    WORD_me = 72,
    WORD_mer = 73,
    WORD_mero = 74,
    WORD_meron = 75,
    WORD_k = 76,
    WORD_ku = 77,
    WORD_kum = 78,
    WORD_kumu = 79,
    WORD_kumuh = 80,
    WORD_kumuha = 81,
    WORD_R = 82,
    WORD_RE = 83,
    WORD_RES = 84,
    WORD_RESI = 85,
    WORD_RESIB = 86,
    WORD_RESIBO = 87,
    WORD_d = 88,
    WORD_da = 89,
    WORD_dag = 90,
    WORD_dagd = 91,
    WORD_dagda = 92,
    WORD_dagdag = 93,
    WORD_b = 94,
    WORD_ba = 95,
    WORD_baw = 96,
    WORD_bawa = 97,
    WORD_bawas = 98,
    WORD_ka = 99,
    WORD_kal = 100,
    WORD_kalk = 101,
    WORD_kalku = 102,
    WORD_kalkul = 103,
    WORD_kalkula = 104,
    WORD_di = 105,
    WORD_dis = 106,
    WORD_disk = 107,
    WORD_diskw = 108,
    WORD_diskwe = 109,
    WORD_diskwen = 110,
    WORD_diskwent = 111,
    WORD_diskwento = 112,
    WORD_bay = 113,
    WORD_baya = 114,
    WORD_bayar = 115,
    WORD_bayara = 116,
    WORD_bayaran = 117,
    WORD_ili = 118,
    WORD_ilim = 119,
    WORD_ilimb = 120,
    WORD_ilimba = 121,
    WORD_ilimbag = 122,
    WORD_l = 123,
    WORD_li = 124,
    WORD_lin = 125,
    WORD_lini = 126,
    WORD_linis = 127,
    WORD_linisi = 128,
    WORD_linisin = 129,
    WORD_h = 130,
    WORD_ha = 131,
    WORD_hat = 132,
    WORD_hati = 133,
    WORD_hatia = 134,
    WORD_hatian = 135,
    WORD_O = 136,
    WORD_OR = 137,
    WORD_ORD = 138,
    WORD_ORDE = 139,
    WORD_ORDER = 140,
    WORD_LU = 141,
    WORD_LUT = 142,
    WORD_LUTO = 143,
    WORD_HA = 144,
    WORD_HAI = 145,
    WORD_HAIN = 146,
    WORD_BA = 147,
    WORD_BAY = 148,
    WORD_BAYA = 149,
    WORD_BAYAD = 150,
    WORD_p = 151,
    WORD_po = 152,
    WORD_n = 153,
    WORD_na = 154,
    WORD_la = 155,
    WORD_lan = 156,
    WORD_lang = 157,
    WORD_a = 158,
    WORD_at = 159,
    WORD_nan = 160,
    WORD_nang = 161,
    WORD_ng = 162,
    WORD_an = 163,
    WORD_ang = 164,
    WORD_ay = 165,
    WORD_ni = 166,
    WORD_nit = 167,
    WORD_nito = 168,
    WORD_o = 169,
    WORD_op = 170,
    WORD_opo = 171,
    WORD_nga = 172,
    WORD_ngan = 173,
    WORD_ngani = 174,
    SYMBOL_DOT = 920,
    SYMBOL_COMMA = 921,
    SYMBOL_COLON = 922,
    SYMBOL_LEFT_PAREN = 923,
    SYMBOL_RIGHT_PAREN = 924,
    SYMBOL_LEFT_BRACE = 925,
    SYMBOL_RIGHT_BRACE = 926,
    SYMBOL_PLUS = 927,
    SYMBOL_MINUS = 928,
    SYMBOL_STAR = 929,
    SYMBOL_SLASH = 930,
    SYMBOL_PERCENT = 931,
    SYMBOL_BACKSLASH = 932,
    SYMBOL_CARET = 933,
    SYMBOL_EQUAL = 934,
    SYMBOL_EQUAL_EQUAL = 935,
    SYMBOL_BANG = 936,
    SYMBOL_BANG_EQUAL = 937,
    SYMBOL_GREATER = 938,
    SYMBOL_GREATER_EQUAL = 939,
    SYMBOL_LESS = 940,
    SYMBOL_LESS_EQUAL = 941,
    SYMBOL_AMPERSAND = 942,
    SYMBOL_AMPERSAND_AMPERSAND = 943,
    SYMBOL_PIPE = 944,
    SYMBOL_PIPE_PIPE = 945,
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
    {WORD_po, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_na, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_lang, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_at, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_nang, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_ng, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_ang, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_ay, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_nito, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_opo, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
    {WORD_ngani, TOKEN_KIND_NOISE_WORD, "IGNORED_NOISE_WORD", 0},
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
    {STATE_CHAR, TOKEN_KIND_LETRA_LITERAL, "CHARACTER_LITERAL", 0},
};

/* Recognizes the supported UTF-8 encodings of the Filipino letter enye. */
int dfa_is_filipino_letter(const unsigned char *source, size_t position,
                           size_t length)
{
    return position + 1 < length && source[position] == 0xC3 &&
           (source[position + 1] == 0xB1 || source[position + 1] == 0x91);
}

/* Tests whether a single-byte character may begin an identifier. */
static int is_identifier_start(int character)
{
    return isalpha((unsigned char)character);
}

/* Tests whether a single-byte character may continue an identifier. */
static int is_identifier_part(int character)
{
    return isalnum((unsigned char)character) || character == '_';
}

/* Checks an identifier's first source character, including UTF-8 enye. */
int dfa_is_identifier_start(const char *source, size_t position, size_t length)
{
    return dfa_is_filipino_letter((const unsigned char *)source, position, length) ||
           is_identifier_start((unsigned char)source[position]);
}

/* Checks a later identifier character, including digits and underscores. */
int dfa_is_identifier_part(const char *source, size_t position, size_t length)
{
    return dfa_is_filipino_letter((const unsigned char *)source, position, length) ||
           is_identifier_part((unsigned char)source[position]);
}

/* Returns whether a character is one of the lexer's supported whitespace bytes. */
int dfa_is_whitespace(int character)
{
    return character == ' ' || character == '\t' ||
           character == '\r' || character == '\n';
}

/* Finds the token metadata assigned to an accepting DFA state. */
const AcceptingState *dfa_find_accept(int state)
{
    size_t i;
    for (i = 0; i < sizeof accepting_states / sizeof accepting_states[0]; ++i) {
        if (accepting_states[i].state == state) {
            return &accepting_states[i];
        }
    }
    return NULL;
}

/* Advances through the explicit word-prefix transitions used for reserved words. */
static int dfa_step_word(int state, int character)
{
    switch (state) {
    case WORD_L:
        switch (character) {
        case 'E': return WORD_LE;
        case 'U': return WORD_LU;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LE:
        switch (character) {
        case 'T': return WORD_LET;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LET:
        switch (character) {
        case 'R': return WORD_LETR;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LETR:
        switch (character) {
        case 'A': return WORD_LETRA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LETRA:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_S:
        switch (character) {
        case 'A': return WORD_SA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SA:
        switch (character) {
        case 'L': return WORD_SAL;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SAL:
        switch (character) {
        case 'I': return WORD_SALI;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SALI:
        switch (character) {
        case 'T': return WORD_SALIT;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SALIT:
        switch (character) {
        case 'A': return WORD_SALITA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_SALITA:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_B:
        switch (character) {
        case 'U': return WORD_BU;
        case 'A': return WORD_BA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BU:
        switch (character) {
        case 'O': return WORD_BUO;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BUO:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_N:
        switch (character) {
        case 'U': return WORD_NU;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NU:
        switch (character) {
        case 'M': return WORD_NUM;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUM:
        switch (character) {
        case 'E': return WORD_NUME;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUME:
        switch (character) {
        case 'R': return WORD_NUMER;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUMER:
        switch (character) {
        case 'O': return WORD_NUMERO;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_NUMERO:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_K:
        switch (character) {
        case 'U': return WORD_KU;
        case 'A': return WORD_KA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KU:
        switch (character) {
        case 'N': return WORD_KUN;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUN:
        switch (character) {
        case 'D': return WORD_KUND;
        case 'G': return WORD_KUNG;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUND:
        switch (character) {
        case 'I': return WORD_KUNDI;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDI:
        switch (character) {
        case 'S': return WORD_KUNDIS;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDIS:
        switch (character) {
        case 'Y': return WORD_KUNDISY;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDISY:
        switch (character) {
        case 'O': return WORD_KUNDISYO;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDISYO:
        switch (character) {
        case 'N': return WORD_KUNDISYON;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNDISYON:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KUNG:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_U:
        switch (character) {
        case 'L': return WORD_UL;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_UL:
        switch (character) {
        case 'I': return WORD_ULI;
        case 'A': return WORD_ULA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ULI:
        switch (character) {
        case 'T': return WORD_ULIT;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ULIT:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_H:
        switch (character) {
        case 'I': return WORD_HI;
        case 'A': return WORD_HA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HI:
        switch (character) {
        case 'N': return WORD_HIN;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HIN:
        switch (character) {
        case 'T': return WORD_HINT;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HINT:
        switch (character) {
        case 'O': return WORD_HINTO;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HINTO:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_T:
        switch (character) {
        case 'U': return WORD_TU;
        case 'A': return WORD_TA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TU:
        switch (character) {
        case 'L': return WORD_TUL;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TUL:
        switch (character) {
        case 'O': return WORD_TULO;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TULO:
        switch (character) {
        case 'Y': return WORD_TULOY;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TULOY:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_W:
        switch (character) {
        case 'A': return WORD_WA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_WA:
        switch (character) {
        case 'L': return WORD_WAL;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_WAL:
        switch (character) {
        case 'A': return WORD_WALA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_WALA:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TA:
        switch (character) {
        case 'M': return WORD_TAM;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TAM:
        switch (character) {
        case 'A': return WORD_TAMA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_TAMA:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_M:
        switch (character) {
        case 'A': return WORD_MA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_MA:
        switch (character) {
        case 'L': return WORD_MAL;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_MAL:
        switch (character) {
        case 'I': return WORD_MALI;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_MALI:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KA:
        switch (character) {
        case 'T': return WORD_KAT;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KAT:
        switch (character) {
        case 'U': return WORD_KATU;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATU:
        switch (character) {
        case 'M': return WORD_KATUM;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUM:
        switch (character) {
        case 'B': return WORD_KATUMB;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUMB:
        switch (character) {
        case 'A': return WORD_KATUMBA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUMBA:
        switch (character) {
        case 'S': return WORD_KATUMBAS;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_KATUMBAS:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ULA:
        switch (character) {
        case 'M': return WORD_ULAM;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ULAM:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_i:
        switch (character) {
        case 'l': return WORD_il;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_il:
        switch (character) {
        case 'a': return WORD_ila;
        case 'i': return WORD_ili;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ila:
        switch (character) {
        case 'g': return WORD_ilag;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilag:
        switch (character) {
        case 'a': return WORD_ilaga;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilaga:
        switch (character) {
        case 'y': return WORD_ilagay;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilagay:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_m:
        switch (character) {
        case 'e': return WORD_me;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_me:
        switch (character) {
        case 'r': return WORD_mer;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_mer:
        switch (character) {
        case 'o': return WORD_mero;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_mero:
        switch (character) {
        case 'n': return WORD_meron;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_meron:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_k:
        switch (character) {
        case 'u': return WORD_ku;
        case 'a': return WORD_ka;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ku:
        switch (character) {
        case 'm': return WORD_kum;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kum:
        switch (character) {
        case 'u': return WORD_kumu;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kumu:
        switch (character) {
        case 'h': return WORD_kumuh;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kumuh:
        switch (character) {
        case 'a': return WORD_kumuha;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kumuha:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_R:
        switch (character) {
        case 'E': return WORD_RE;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RE:
        switch (character) {
        case 'S': return WORD_RES;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RES:
        switch (character) {
        case 'I': return WORD_RESI;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RESI:
        switch (character) {
        case 'B': return WORD_RESIB;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RESIB:
        switch (character) {
        case 'O': return WORD_RESIBO;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_RESIBO:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_d:
        switch (character) {
        case 'a': return WORD_da;
        case 'i': return WORD_di;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_da:
        switch (character) {
        case 'g': return WORD_dag;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dag:
        switch (character) {
        case 'd': return WORD_dagd;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dagd:
        switch (character) {
        case 'a': return WORD_dagda;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dagda:
        switch (character) {
        case 'g': return WORD_dagdag;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dagdag:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_b:
        switch (character) {
        case 'a': return WORD_ba;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ba:
        switch (character) {
        case 'w': return WORD_baw;
        case 'y': return WORD_bay;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_baw:
        switch (character) {
        case 'a': return WORD_bawa;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bawa:
        switch (character) {
        case 's': return WORD_bawas;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bawas:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ka:
        switch (character) {
        case 'l': return WORD_kal;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kal:
        switch (character) {
        case 'k': return WORD_kalk;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalk:
        switch (character) {
        case 'u': return WORD_kalku;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalku:
        switch (character) {
        case 'l': return WORD_kalkul;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalkul:
        switch (character) {
        case 'a': return WORD_kalkula;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_kalkula:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_di:
        switch (character) {
        case 's': return WORD_dis;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_dis:
        switch (character) {
        case 'k': return WORD_disk;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_disk:
        switch (character) {
        case 'w': return WORD_diskw;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskw:
        switch (character) {
        case 'e': return WORD_diskwe;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwe:
        switch (character) {
        case 'n': return WORD_diskwen;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwen:
        switch (character) {
        case 't': return WORD_diskwent;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwent:
        switch (character) {
        case 'o': return WORD_diskwento;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_diskwento:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bay:
        switch (character) {
        case 'a': return WORD_baya;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_baya:
        switch (character) {
        case 'r': return WORD_bayar;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bayar:
        switch (character) {
        case 'a': return WORD_bayara;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bayara:
        switch (character) {
        case 'n': return WORD_bayaran;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_bayaran:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ili:
        switch (character) {
        case 'm': return WORD_ilim;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilim:
        switch (character) {
        case 'b': return WORD_ilimb;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilimb:
        switch (character) {
        case 'a': return WORD_ilimba;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilimba:
        switch (character) {
        case 'g': return WORD_ilimbag;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ilimbag:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_l:
        switch (character) {
        case 'i': return WORD_li;
        case 'a': return WORD_la;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_li:
        switch (character) {
        case 'n': return WORD_lin;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lin:
        switch (character) {
        case 'i': return WORD_lini;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lini:
        switch (character) {
        case 's': return WORD_linis;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_linis:
        switch (character) {
        case 'i': return WORD_linisi;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_linisi:
        switch (character) {
        case 'n': return WORD_linisin;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_linisin:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_h:
        switch (character) {
        case 'a': return WORD_ha;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ha:
        switch (character) {
        case 't': return WORD_hat;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hat:
        switch (character) {
        case 'i': return WORD_hati;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hati:
        switch (character) {
        case 'a': return WORD_hatia;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hatia:
        switch (character) {
        case 'n': return WORD_hatian;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_hatian:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_O:
        switch (character) {
        case 'R': return WORD_OR;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_OR:
        switch (character) {
        case 'D': return WORD_ORD;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ORD:
        switch (character) {
        case 'E': return WORD_ORDE;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ORDE:
        switch (character) {
        case 'R': return WORD_ORDER;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ORDER:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LU:
        switch (character) {
        case 'T': return WORD_LUT;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LUT:
        switch (character) {
        case 'O': return WORD_LUTO;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_LUTO:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HA:
        switch (character) {
        case 'I': return WORD_HAI;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HAI:
        switch (character) {
        case 'N': return WORD_HAIN;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_HAIN:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BA:
        switch (character) {
        case 'Y': return WORD_BAY;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BAY:
        switch (character) {
        case 'A': return WORD_BAYA;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BAYA:
        switch (character) {
        case 'D': return WORD_BAYAD;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_BAYAD:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_p:
        switch (character) {
        case 'o': return WORD_po;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_po:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_n:
        switch (character) {
        case 'a': return WORD_na;
        case 'g': return WORD_ng;
        case 'i': return WORD_ni;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_na:
        switch (character) {
        case 'n': return WORD_nan;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_la:
        switch (character) {
        case 'n': return WORD_lan;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lan:
        switch (character) {
        case 'g': return WORD_lang;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_lang:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_a:
        switch (character) {
        case 't': return WORD_at;
        case 'n': return WORD_an;
        case 'y': return WORD_ay;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_at:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nan:
        switch (character) {
        case 'g': return WORD_nang;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nang:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ng:
        switch (character) {
        case 'a': return WORD_nga;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nga:
        switch (character) {
        case 'n': return WORD_ngan;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ngan:
        switch (character) {
        case 'i': return WORD_ngani;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ngani:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_an:
        switch (character) {
        case 'g': return WORD_ang;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ang:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ay:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_ni:
        switch (character) {
        case 't': return WORD_nit;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nit:
        switch (character) {
        case 'o': return WORD_nito;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_nito:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_o:
        switch (character) {
        case 'p': return WORD_op;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_op:
        switch (character) {
        case 'o': return WORD_opo;
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    case WORD_opo:
        switch (character) {
        default:
            return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
        }
    default:
        return -1;
    }
}

/* Advances symbol states, including two-character operators. */
static int dfa_step_symbol(int state, int character)
{
    switch (state) {
    case SYMBOL_EQUAL:
        switch (character) {
        case '=': return SYMBOL_EQUAL_EQUAL;
        default: return -1;
        }
    case SYMBOL_BANG:
        switch (character) {
        case '=': return SYMBOL_BANG_EQUAL;
        default: return -1;
        }
    case SYMBOL_GREATER:
        switch (character) {
        case '=': return SYMBOL_GREATER_EQUAL;
        default: return -1;
        }
    case SYMBOL_LESS:
        switch (character) {
        case '=': return SYMBOL_LESS_EQUAL;
        default: return -1;
        }
    case SYMBOL_AMPERSAND:
        switch (character) {
        case '&': return SYMBOL_AMPERSAND_AMPERSAND;
        default: return -1;
        }
    case SYMBOL_PIPE:
        switch (character) {
        case '|': return SYMBOL_PIPE_PIPE;
        default: return -1;
        }
    default:
        return -1;
    }
}

/* Applies one DFA transition from a word, symbol, or general lexical state. */
int dfa_step(int state, int character)
{
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
            if (isalpha((unsigned char)character)) return STATE_IDENTIFIER;
            if (isdigit((unsigned char)character)) return STATE_INTEGER;
            if (dfa_is_whitespace(character)) return STATE_START;
            return STATE_ERROR;
        }
    case STATE_IDENTIFIER:
        return is_identifier_part(character) ? STATE_IDENTIFIER : -1;
    case STATE_INTEGER:
        if (isdigit((unsigned char)character)) return STATE_INTEGER;
        if (character == '.') return STATE_DECIMAL;
        return -1;
    case STATE_DECIMAL:
        return isdigit((unsigned char)character) ? STATE_DECIMAL : -1;
    case STATE_STRING:
        return character == '"' ? STATE_START : STATE_STRING;
    case STATE_CHAR:
        return character == '\'' ? STATE_START : STATE_CHAR;
    case STATE_COMMENT:
        return STATE_COMMENT;
    default:
        return -1;
    }
}

/* Converts an internal token kind into the label printed in reports. */
const char *dfa_token_kind_name(TokenKind kind)
{
    switch (kind) {
    case TOKEN_KIND_ACCESSOR:
        return "TOKEN_ACCESSOR";
    case TOKEN_KIND_ASSIGNMENT:
        return "TOKEN_ASSIGNMENT";
    case TOKEN_KIND_BUO_LITERAL:
        return "TOKEN_BUO_LITERAL";
    case TOKEN_KIND_IDENTIFIER:
        return "TOKEN_IDENTIFIER";
    case TOKEN_KIND_KW_BUO:
        return "TOKEN_KW_BUO";
    case TOKEN_KIND_KW_HINTO:
        return "TOKEN_KW_HINTO";
    case TOKEN_KIND_KW_KUNDI:
        return "TOKEN_KW_KUNDI";
    case TOKEN_KIND_KW_KUNDISYON:
        return "TOKEN_KW_KUNDISYON";
    case TOKEN_KIND_KW_KUNG:
        return "TOKEN_KW_KUNG";
    case TOKEN_KIND_KW_LETRA:
        return "TOKEN_KW_LETRA";
    case TOKEN_KIND_KW_NUMERO:
        return "TOKEN_KW_NUMERO";
    case TOKEN_KIND_KW_SALITA:
        return "TOKEN_KW_SALITA";
    case TOKEN_KIND_KW_TULOY:
        return "TOKEN_KW_TULOY";
    case TOKEN_KIND_KW_ULIT:
        return "TOKEN_KW_ULIT";
    case TOKEN_KIND_KW_WALA:
        return "TOKEN_KW_WALA";
    case TOKEN_KIND_LBRACE:
        return "TOKEN_LBRACE";
    case TOKEN_KIND_LETRA_LITERAL:
        return "TOKEN_LETRA_LITERAL";
    case TOKEN_KIND_LOGIC_AND:
        return "TOKEN_LOGIC_AND";
    case TOKEN_KIND_LOGIC_NOT:
        return "TOKEN_LOGIC_NOT";
    case TOKEN_KIND_LOGIC_OR:
        return "TOKEN_LOGIC_OR";
    case TOKEN_KIND_LPAREN:
        return "TOKEN_LPAREN";
    case TOKEN_KIND_NOISE_WORD:
        return "TOKEN_NOISE_WORD";
    case TOKEN_KIND_NUMERO_LITERAL:
        return "TOKEN_NUMERO_LITERAL";
    case TOKEN_KIND_OP_ADD:
        return "TOKEN_OP_ADD";
    case TOKEN_KIND_OP_DIV:
        return "TOKEN_OP_DIV";
    case TOKEN_KIND_OP_INTDIV:
        return "TOKEN_OP_INTDIV";
    case TOKEN_KIND_OP_MOD:
        return "TOKEN_OP_MOD";
    case TOKEN_KIND_OP_MUL:
        return "TOKEN_OP_MUL";
    case TOKEN_KIND_OP_POW:
        return "TOKEN_OP_POW";
    case TOKEN_KIND_OP_SUB:
        return "TOKEN_OP_SUB";
    case TOKEN_KIND_RBRACE:
        return "TOKEN_RBRACE";
    case TOKEN_KIND_REL_EQ:
        return "TOKEN_REL_EQ";
    case TOKEN_KIND_REL_GT:
        return "TOKEN_REL_GT";
    case TOKEN_KIND_REL_GTE:
        return "TOKEN_REL_GTE";
    case TOKEN_KIND_REL_LT:
        return "TOKEN_REL_LT";
    case TOKEN_KIND_REL_LTE:
        return "TOKEN_REL_LTE";
    case TOKEN_KIND_REL_NEQ:
        return "TOKEN_REL_NEQ";
    case TOKEN_KIND_RES_BAYAD:
        return "TOKEN_RES_BAYAD";
    case TOKEN_KIND_RES_HAIN:
        return "TOKEN_RES_HAIN";
    case TOKEN_KIND_RES_KATUMBAS:
        return "TOKEN_RES_KATUMBAS";
    case TOKEN_KIND_RES_LUTO:
        return "TOKEN_RES_LUTO";
    case TOKEN_KIND_RES_MALI:
        return "TOKEN_RES_MALI";
    case TOKEN_KIND_RES_ORDER:
        return "TOKEN_RES_ORDER";
    case TOKEN_KIND_RES_RESIBO:
        return "TOKEN_RES_RESIBO";
    case TOKEN_KIND_RES_TAMA:
        return "TOKEN_RES_TAMA";
    case TOKEN_KIND_RES_ULAM:
        return "TOKEN_RES_ULAM";
    case TOKEN_KIND_RES_BAWAS:
        return "TOKEN_RES_bawas";
    case TOKEN_KIND_RES_BAYARAN:
        return "TOKEN_RES_bayaran";
    case TOKEN_KIND_RES_DAGDAG:
        return "TOKEN_RES_dagdag";
    case TOKEN_KIND_RES_DISKWENTO:
        return "TOKEN_RES_diskwento";
    case TOKEN_KIND_RES_HATIAN:
        return "TOKEN_RES_hatian";
    case TOKEN_KIND_RES_ILAGAY:
        return "TOKEN_RES_ilagay";
    case TOKEN_KIND_RES_ILIMBAG:
        return "TOKEN_RES_ilimbag";
    case TOKEN_KIND_RES_KALKULA:
        return "TOKEN_RES_kalkula";
    case TOKEN_KIND_RES_KUMUHA:
        return "TOKEN_RES_kumuha";
    case TOKEN_KIND_RES_LINISIN:
        return "TOKEN_RES_linisin";
    case TOKEN_KIND_RES_MERON:
        return "TOKEN_RES_meron";
    case TOKEN_KIND_RPAREN:
        return "TOKEN_RPAREN";
    case TOKEN_KIND_SALITA_LITERAL:
        return "TOKEN_SALITA_LITERAL";
    case TOKEN_KIND_SEPARATOR:
        return "TOKEN_SEPARATOR";
    case TOKEN_KIND_TERMINATOR:
        return "TOKEN_TERMINATOR";
    case TOKEN_KIND_INVALID:
        return "INVALID";
    case TOKEN_KIND_LINE_COMMENT:
        return "LINE_COMMENT";
    }
    return "TOKEN_UNKNOWN";
}

/* Writes one token-table row from a lexeme span without requiring a terminator. */
void dfa_write_symbol_span(FILE *out, const char *type,
                              const char *lexeme, size_t length)
{
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

/* Writes one token-table row using the token kind's printable name. */
void dfa_write_symbol(FILE *out, TokenKind kind, const char *lexeme)
{
    dfa_write_symbol_span(out, dfa_token_kind_name(kind), lexeme, strlen(lexeme));
}

/* Writes an acceptance record from a lexeme span and its final DFA state. */
void dfa_write_acceptance_span(FILE *out, const char *lexeme,
                                 size_t length, TokenKind kind,
                                 const char *classification, int state)
{
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

/* Writes an acceptance record for a null-terminated lexeme. */
void dfa_write_acceptance(FILE *out, const char *lexeme,
                            TokenKind kind,
                            const char *classification, int state)
{
    dfa_write_acceptance_span(out, lexeme, strlen(lexeme), kind,
                                classification, state);
}

/* Reports a lexical error to stderr and adds an INVALID row to the token stream. */
void dfa_report_error(FILE *out, size_t line, size_t column,
                         const char *message)
{
    fprintf(stderr, "Lexer error at line %zu, column %zu: %s\n",
            line, column, message);
    dfa_write_symbol(out, TOKEN_KIND_INVALID, message);
}
