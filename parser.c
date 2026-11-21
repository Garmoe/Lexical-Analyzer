/*
 * JAVArice Syntax Analyzer (Parser)
 * 
 * Based on Robert W. Sebesta, "Concepts of Programming Languages" (10th Edition),
 * Chapter 4: Lexical and Syntax Analysis (Recursive-Descent LL Parsing).
 *
 * Approach 1: Directly includes lexer.c without requiring header files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARSER_BUILD
#include "lexer.c"

/* ================================================================= */
/* PARSER DATA STRUCTURES                                            */
/* ================================================================= */

typedef struct {
    const Token *tokens;
    size_t count;
    size_t pos;
    int error_count;
    int panic_mode;
    int trace;
} Parser;

/* Forward declarations for recursive descent functions */
static void parse_program(Parser *p);
static void parse_data_section(Parser *p);
static void parse_data_stmt(Parser *p);
static void parse_declaration(Parser *p);
static void parse_declarator(Parser *p);
static void parse_assignment(Parser *p);
static void parse_method_stmt(Parser *p);
static void parse_method_call(Parser *p);
static void parse_ulam_call(Parser *p);
static void parse_resibo_call(Parser *p);
static void parse_arg_list(Parser *p);
static void parse_order_phase(Parser *p);
static void parse_luto_phase(Parser *p);
static void parse_hain_phase(Parser *p);
static void parse_bayad_phase(Parser *p);
static void parse_block(Parser *p);
static void parse_statement(Parser *p);
static void parse_if_stmt(Parser *p);
static void parse_loop_stmt(Parser *p);
static void parse_jump_stmt(Parser *p);
static void parse_null_stmt(Parser *p);
static void parse_expr(Parser *p);
static void parse_or_expr(Parser *p);
static void parse_and_expr(Parser *p);
static void parse_equality(Parser *p);
static void parse_relational(Parser *p);
static void parse_additive(Parser *p);
static void parse_term(Parser *p);
static void parse_unary(Parser *p);
static void parse_power(Parser *p);
static void parse_primary(Parser *p);

/* ================================================================= */
/* TOKEN STREAM NAVIGATION (Sebesta §4.1)                            */
/* Automatically filters out non-syntactic tokens (Comments & Noise) */
/* ================================================================= */

static const Token *current(Parser *p) {
    while (p->pos < p->count &&
          (p->tokens[p->pos].kind == TOKEN_KIND_LINE_COMMENT ||
           p->tokens[p->pos].kind == TOKEN_KIND_NOISE_WORD)) {
        p->pos++;
    }
    if (p->pos < p->count) {
        return &p->tokens[p->pos];
    }
    if (p->count > 0) {
        return &p->tokens[p->count - 1]; /* TOKEN_KIND_EOF */
    }
    static const Token eof_tok = { TOKEN_KIND_EOF, "EOF", 0, 0 };
    return &eof_tok;
}

static void advance(Parser *p) {
    if (p->pos < p->count) {
        p->pos++;
    }
    while (p->pos < p->count &&
          (p->tokens[p->pos].kind == TOKEN_KIND_LINE_COMMENT ||
           p->tokens[p->pos].kind == TOKEN_KIND_NOISE_WORD)) {
        p->pos++;
    }
}

static void trace_enter(Parser *p, const char *nonterminal) {
    if (p->trace) {
        printf("  [ENTER] <%s> at Line %zu, Column %zu (Token: '%s')\n",
               nonterminal, current(p)->line, current(p)->column, current(p)->lexeme);
    }
}

static void trace_exit(Parser *p, const char *nonterminal) {
    if (p->trace) {
        printf("  [EXIT]  <%s>\n", nonterminal);
    }
}

/* ================================================================= */
/* ERROR DETECTION & PANIC-MODE RECOVERY (Sebesta §4.4)              */
/* ================================================================= */

static void syntax_error(Parser *p, const char *msg) {
    if (p->panic_mode) return; /* Suppress cascaded error noise during recovery */
    p->error_count++;
    p->panic_mode = 1;
    const Token *tok = current(p);
    fprintf(stderr, "Syntax Error [Line %zu, Column %zu] near '%s': %s\n",
            tok->line, tok->column, tok->lexeme, msg);
}

static void expect(Parser *p, TokenKind expected, const char *err_msg) {
    if (current(p)->kind == expected) {
        advance(p);
    } else {
        syntax_error(p, err_msg);
    }
}

static int is_type(TokenKind k) {
    return k == TOKEN_KIND_KW_SALITA || k == TOKEN_KIND_KW_NUMERO ||
           k == TOKEN_KIND_KW_BUO    || k == TOKEN_KIND_KW_LETRA  ||
           k == TOKEN_KIND_KW_KUNDISYON;
}

static int is_resibo_method(TokenKind k) {
    return k == TOKEN_KIND_RES_DAGDAG    || k == TOKEN_KIND_RES_BAWAS     ||
           k == TOKEN_KIND_RES_KALKULA   || k == TOKEN_KIND_RES_DISKWENTO ||
           k == TOKEN_KIND_RES_BAYARAN   || k == TOKEN_KIND_RES_ILIMBAG   ||
           k == TOKEN_KIND_RES_LINISIN   || k == TOKEN_KIND_RES_HATIAN;
}

static int is_ulam_method(TokenKind k) {
    return k == TOKEN_KIND_RES_ILAGAY || k == TOKEN_KIND_RES_MERON ||
           k == TOKEN_KIND_RES_KUMUHA || k == TOKEN_KIND_RES_BAWAS;
}

static int is_literal(TokenKind k) {
    return k == TOKEN_KIND_BUO_LITERAL    || k == TOKEN_KIND_NUMERO_LITERAL ||
           k == TOKEN_KIND_SALITA_LITERAL || k == TOKEN_KIND_LETRA_LITERAL  ||
           k == TOKEN_KIND_RES_TAMA       || k == TOKEN_KIND_RES_MALI       ||
           k == TOKEN_KIND_KW_WALA;
}

static int is_statement_start(TokenKind k) {
    return is_type(k) || k == TOKEN_KIND_IDENTIFIER ||
           k == TOKEN_KIND_RES_ULAM || k == TOKEN_KIND_RES_RESIBO ||
           k == TOKEN_KIND_KW_KUNG  || k == TOKEN_KIND_KW_ULIT   ||
           k == TOKEN_KIND_KW_HINTO || k == TOKEN_KIND_KW_TULOY  ||
           k == TOKEN_KIND_KW_WALA  || k == TOKEN_KIND_RES_MALI  ||
           k == TOKEN_KIND_RES_TAMA;
}

static void synchronize(Parser *p) {
    p->panic_mode = 0;
    while (current(p)->kind != TOKEN_KIND_EOF) {
        if (current(p)->kind == TOKEN_KIND_TERMINATOR) {
            advance(p);
            return;
        }
        if (current(p)->kind == TOKEN_KIND_RBRACE) {
            return;
        }
        if (is_statement_start(current(p)->kind) ||
            current(p)->kind == TOKEN_KIND_RES_ORDER ||
            current(p)->kind == TOKEN_KIND_RES_LUTO  ||
            current(p)->kind == TOKEN_KIND_RES_HAIN  ||
            current(p)->kind == TOKEN_KIND_RES_BAYAD) {
            return;
        }
        advance(p);
    }
}

/* ================================================================= */
/* 1. PROGRAM STRUCTURE                                              */
/* ================================================================= */

/* <block> -> "{" { <statement> } "}" (Sebesta Chapter 4 while loop) */
static void parse_block(Parser *p) {
    trace_enter(p, "block");
    expect(p, TOKEN_KIND_LBRACE, "Expected '{' to begin block.");
    while (current(p)->kind != TOKEN_KIND_RBRACE && current(p)->kind != TOKEN_KIND_EOF) {
        parse_statement(p);
    }
    expect(p, TOKEN_KIND_RBRACE, "Expected '}' to close block.");
    trace_exit(p, "block");
}

/* <order_phase> -> ORDER [ <noise> ] <block> */
static void parse_order_phase(Parser *p) {
    trace_enter(p, "order_phase");
    expect(p, TOKEN_KIND_RES_ORDER, "Expected 'ORDER' phase.");
    parse_block(p);
    trace_exit(p, "order_phase");
}

/* <luto_phase> -> LUTO [ <noise> ] <block> */
static void parse_luto_phase(Parser *p) {
    trace_enter(p, "luto_phase");
    expect(p, TOKEN_KIND_RES_LUTO, "Expected 'LUTO' phase.");
    parse_block(p);
    trace_exit(p, "luto_phase");
}

/* <hain_phase> -> HAIN [ <noise> ] <block> */
static void parse_hain_phase(Parser *p) {
    trace_enter(p, "hain_phase");
    expect(p, TOKEN_KIND_RES_HAIN, "Expected 'HAIN' phase.");
    parse_block(p);
    trace_exit(p, "hain_phase");
}

/* <bayad_phase> -> BAYAD [ <noise> ] <block> */
static void parse_bayad_phase(Parser *p) {
    trace_enter(p, "bayad_phase");
    expect(p, TOKEN_KIND_RES_BAYAD, "Expected 'BAYAD' phase.");
    parse_block(p);
    trace_exit(p, "bayad_phase");
}

/* ================================================================= */
/* 2. DECLARATIONS & ASSIGNMENT                                      */
/* ================================================================= */

/* <declarator> -> IDENTIFIER [ "=" <expr> ] */
static void parse_declarator(Parser *p) {
    trace_enter(p, "declarator");
    expect(p, TOKEN_KIND_IDENTIFIER, "Expected identifier in declaration.");
    if (current(p)->kind == TOKEN_KIND_ASSIGNMENT) {
        advance(p); /* consume '=' */
        parse_expr(p);
    }
    trace_exit(p, "declarator");
}

/* <declaration> -> <type> <declarator> { "," <declarator> } "." */
static void parse_declaration(Parser *p) {
    trace_enter(p, "declaration");
    if (!is_type(current(p)->kind)) {
        syntax_error(p, "Expected data type (SALITA, NUMERO, BUO, LETRA, KUNDISYON).");
        synchronize(p);
        return;
    }
    advance(p); /* consume type */
    parse_declarator(p);
    while (current(p)->kind == TOKEN_KIND_SEPARATOR) {
        advance(p); /* consume ',' */
        parse_declarator(p);
    }
    expect(p, TOKEN_KIND_TERMINATOR, "Expected '.' at end of declaration.");
    trace_exit(p, "declaration");
}

/* <assignment> -> IDENTIFIER "=" <expr> "." */
static void parse_assignment(Parser *p) {
    trace_enter(p, "assignment");
    expect(p, TOKEN_KIND_IDENTIFIER, "Expected identifier.");
    expect(p, TOKEN_KIND_ASSIGNMENT, "Expected '=' in assignment.");
    parse_expr(p);
    expect(p, TOKEN_KIND_TERMINATOR, "Expected '.' at end of assignment.");
    trace_exit(p, "assignment");
}

/* ================================================================= */
/* 3. BUILT-IN METHOD CALLS                                          */
/* ================================================================= */

/* <arg_list> -> <expr> { "," <expr> } */
static void parse_arg_list(Parser *p) {
    trace_enter(p, "arg_list");
    parse_expr(p);
    while (current(p)->kind == TOKEN_KIND_SEPARATOR) {
        advance(p); /* consume ',' */
        parse_expr(p);
    }
    trace_exit(p, "arg_list");
}

/* <ulam_call> -> ULAM ":" <ulam_method> "(" [ <arg_list> ] ")" */
static void parse_ulam_call(Parser *p) {
    trace_enter(p, "ulam_call");
    expect(p, TOKEN_KIND_RES_ULAM, "Expected 'ULAM'.");
    expect(p, TOKEN_KIND_ACCESSOR, "Expected ':' after 'ULAM'.");
    if (!is_ulam_method(current(p)->kind)) {
        syntax_error(p, "Expected valid ULAM method (ilagay, meron, kumuha, bawas).");
    } else {
        advance(p);
    }
    expect(p, TOKEN_KIND_LPAREN, "Expected '(' after method name.");
    if (current(p)->kind != TOKEN_KIND_RPAREN) {
        parse_arg_list(p);
    }
    expect(p, TOKEN_KIND_RPAREN, "Expected ')' after method arguments.");
    trace_exit(p, "ulam_call");
}

/* <resibo_call> -> RESIBO ":" <resibo_method> "(" [ <arg_list> ] ")" */
static void parse_resibo_call(Parser *p) {
    trace_enter(p, "resibo_call");
    expect(p, TOKEN_KIND_RES_RESIBO, "Expected 'RESIBO'.");
    expect(p, TOKEN_KIND_ACCESSOR, "Expected ':' after 'RESIBO'.");
    if (!is_resibo_method(current(p)->kind)) {
        syntax_error(p, "Expected valid RESIBO method (dagdag, bawas, kalkula, diskwento, bayaran, ilimbag, linisin, hatian).");
    } else {
        advance(p);
    }
    expect(p, TOKEN_KIND_LPAREN, "Expected '(' after method name.");
    if (current(p)->kind != TOKEN_KIND_RPAREN) {
        parse_arg_list(p);
    }
    expect(p, TOKEN_KIND_RPAREN, "Expected ')' after method arguments.");
    trace_exit(p, "resibo_call");
}

/* <method_call> -> <ulam_call> | <resibo_call> */
static void parse_method_call(Parser *p) {
    trace_enter(p, "method_call");
    if (current(p)->kind == TOKEN_KIND_RES_ULAM) {
        parse_ulam_call(p);
    } else if (current(p)->kind == TOKEN_KIND_RES_RESIBO) {
        parse_resibo_call(p);
    } else {
        syntax_error(p, "Expected 'ULAM' or 'RESIBO'.");
    }
    trace_exit(p, "method_call");
}

/* <method_stmt> -> <method_call> "." */
static void parse_method_stmt(Parser *p) {
    trace_enter(p, "method_stmt");
    parse_method_call(p);
    expect(p, TOKEN_KIND_TERMINATOR, "Expected '.' at end of method statement.");
    trace_exit(p, "method_stmt");
}

/* <data_stmt> -> <declaration> | <assignment> | <method_stmt> */
static void parse_data_stmt(Parser *p) {
    trace_enter(p, "data_stmt");
    TokenKind k = current(p)->kind;
    if (is_type(k)) {
        parse_declaration(p);
    } else if (k == TOKEN_KIND_IDENTIFIER) {
        parse_assignment(p);
    } else if (k == TOKEN_KIND_RES_ULAM || k == TOKEN_KIND_RES_RESIBO) {
        parse_method_stmt(p);
    } else {
        syntax_error(p, "Invalid statement in data section.");
        synchronize(p);
    }
    trace_exit(p, "data_stmt");
}

/* <data_section> -> { <data_stmt> } */
static void parse_data_section(Parser *p) {
    trace_enter(p, "data_section");
    while (is_type(current(p)->kind) ||
           current(p)->kind == TOKEN_KIND_IDENTIFIER ||
           current(p)->kind == TOKEN_KIND_RES_ULAM ||
           current(p)->kind == TOKEN_KIND_RES_RESIBO) {
        parse_data_stmt(p);
    }
    trace_exit(p, "data_section");
}

/* ================================================================= */
/* 4. CONTROL FLOW                                                   */
/* ================================================================= */

/* <if_stmt> -> KUNG <expr> <block> [ KUNDI ( <if_stmt> | <block> ) ] */
static void parse_if_stmt(Parser *p) {
    trace_enter(p, "if_stmt");
    expect(p, TOKEN_KIND_KW_KUNG, "Expected 'KUNG'.");
    parse_expr(p);
    parse_block(p);
    if (current(p)->kind == TOKEN_KIND_KW_KUNDI) {
        advance(p); /* consume KUNDI */
        if (current(p)->kind == TOKEN_KIND_KW_KUNG) {
            parse_if_stmt(p);
        } else {
            parse_block(p);
        }
    }
    trace_exit(p, "if_stmt");
}

/* <loop_stmt> -> ULIT [ <expr> ] <block> */
static void parse_loop_stmt(Parser *p) {
    trace_enter(p, "loop_stmt");
    expect(p, TOKEN_KIND_KW_ULIT, "Expected 'ULIT'.");
    if (current(p)->kind != TOKEN_KIND_LBRACE) {
        parse_expr(p);
    }
    parse_block(p);
    trace_exit(p, "loop_stmt");
}

/* <jump_stmt> -> ( HINTO | TULOY ) "." */
static void parse_jump_stmt(Parser *p) {
    trace_enter(p, "jump_stmt");
    if (current(p)->kind == TOKEN_KIND_KW_HINTO || current(p)->kind == TOKEN_KIND_KW_TULOY) {
        advance(p);
    } else {
        syntax_error(p, "Expected 'HINTO' or 'TULOY'.");
    }
    expect(p, TOKEN_KIND_TERMINATOR, "Expected '.' after jump statement.");
    trace_exit(p, "jump_stmt");
}

/* <null_stmt> -> ( WALA | MALI | TAMA ) "." */
static void parse_null_stmt(Parser *p) {
    trace_enter(p, "null_stmt");
    if (current(p)->kind == TOKEN_KIND_KW_WALA ||
        current(p)->kind == TOKEN_KIND_RES_MALI ||
        current(p)->kind == TOKEN_KIND_RES_TAMA) {
        advance(p);
    } else {
        syntax_error(p, "Expected 'WALA', 'MALI', or 'TAMA'.");
    }
    expect(p, TOKEN_KIND_TERMINATOR, "Expected '.' after statement.");
    trace_exit(p, "null_stmt");
}

/* <statement> -> <declaration> | <assignment> | <method_stmt> | <if_stmt>
                | <loop_stmt> | <jump_stmt> | <null_stmt> */
static void parse_statement(Parser *p) {
    trace_enter(p, "statement");
    TokenKind k = current(p)->kind;
    if (is_type(k)) {
        parse_declaration(p);
    } else if (k == TOKEN_KIND_IDENTIFIER) {
        parse_assignment(p);
    } else if (k == TOKEN_KIND_RES_ULAM || k == TOKEN_KIND_RES_RESIBO) {
        parse_method_stmt(p);
    } else if (k == TOKEN_KIND_KW_KUNG) {
        parse_if_stmt(p);
    } else if (k == TOKEN_KIND_KW_ULIT) {
        parse_loop_stmt(p);
    } else if (k == TOKEN_KIND_KW_HINTO || k == TOKEN_KIND_KW_TULOY) {
        parse_jump_stmt(p);
    } else if (k == TOKEN_KIND_KW_WALA || k == TOKEN_KIND_RES_MALI || k == TOKEN_KIND_RES_TAMA) {
        parse_null_stmt(p);
    } else {
        syntax_error(p, "Expected a valid statement.");
        synchronize(p);
    }
    trace_exit(p, "statement");
}

/* ================================================================= */
/* 5. EXPRESSIONS (Sebesta §4.4 Precedence Stratification)           */
/* ================================================================= */

/* <primary> -> <literal> | IDENTIFIER | <method_call> | "(" <expr> ")" */
static void parse_primary(Parser *p) {
    trace_enter(p, "primary");
    TokenKind k = current(p)->kind;
    if (is_literal(k) || k == TOKEN_KIND_IDENTIFIER) {
        advance(p);
    } else if (k == TOKEN_KIND_RES_ULAM || k == TOKEN_KIND_RES_RESIBO) {
        parse_method_call(p);
    } else if (k == TOKEN_KIND_LPAREN) {
        advance(p);
        parse_expr(p);
        expect(p, TOKEN_KIND_RPAREN, "Expected ')' after expression.");
    } else {
        syntax_error(p, "Expected literal, identifier, method call, or '(' in expression.");
    }
    trace_exit(p, "primary");
}

/* <power> -> <primary> [ "^" <power> ] (Right-associative) */
static void parse_power(Parser *p) {
    trace_enter(p, "power");
    parse_primary(p);
    if (current(p)->kind == TOKEN_KIND_OP_POW) {
        advance(p);
        parse_power(p); /* Right-recursive call */
    }
    trace_exit(p, "power");
}

/* <unary> -> ( "!" | "-" ) <unary> | <power> */
static void parse_unary(Parser *p) {
    trace_enter(p, "unary");
    TokenKind k = current(p)->kind;
    if (k == TOKEN_KIND_LOGIC_NOT || k == TOKEN_KIND_OP_SUB) {
        advance(p);
        parse_unary(p);
    } else {
        parse_power(p);
    }
    trace_exit(p, "unary");
}

/* <term> -> <unary> { ( "*" | "/" | "%" | "\" ) <unary> } */
static void parse_term(Parser *p) {
    trace_enter(p, "term");
    parse_unary(p);
    while (current(p)->kind == TOKEN_KIND_OP_MUL ||
           current(p)->kind == TOKEN_KIND_OP_DIV ||
           current(p)->kind == TOKEN_KIND_OP_MOD ||
           current(p)->kind == TOKEN_KIND_OP_INTDIV) {
        advance(p);
        parse_unary(p);
    }
    trace_exit(p, "term");
}

/* <additive> -> <term> { ( "+" | "-" ) <term> } */
static void parse_additive(Parser *p) {
    trace_enter(p, "additive");
    parse_term(p);
    while (current(p)->kind == TOKEN_KIND_OP_ADD ||
           current(p)->kind == TOKEN_KIND_OP_SUB) {
        advance(p);
        parse_term(p);
    }
    trace_exit(p, "additive");
}

/* <relational> -> <additive> { ( "<" | ">" | "<=" | ">=" ) <additive> } */
static void parse_relational(Parser *p) {
    trace_enter(p, "relational");
    parse_additive(p);
    while (current(p)->kind == TOKEN_KIND_REL_LT  ||
           current(p)->kind == TOKEN_KIND_REL_GT  ||
           current(p)->kind == TOKEN_KIND_REL_LTE ||
           current(p)->kind == TOKEN_KIND_REL_GTE) {
        advance(p);
        parse_additive(p);
    }
    trace_exit(p, "relational");
}

/* <equality> -> <relational> { ( "==" | "!=" | KATUMBAS ) <relational> } */
static void parse_equality(Parser *p) {
    trace_enter(p, "equality");
    parse_relational(p);
    while (current(p)->kind == TOKEN_KIND_REL_EQ  ||
           current(p)->kind == TOKEN_KIND_REL_NEQ ||
           current(p)->kind == TOKEN_KIND_RES_KATUMBAS) {
        advance(p);
        parse_relational(p);
    }
    trace_exit(p, "equality");
}

/* <and_expr> -> <equality> { "&&" <equality> } */
static void parse_and_expr(Parser *p) {
    trace_enter(p, "and_expr");
    parse_equality(p);
    while (current(p)->kind == TOKEN_KIND_LOGIC_AND) {
        advance(p);
        parse_equality(p);
    }
    trace_exit(p, "and_expr");
}

/* <or_expr> -> <and_expr> { "||" <and_expr> } */
static void parse_or_expr(Parser *p) {
    trace_enter(p, "or_expr");
    parse_and_expr(p);
    while (current(p)->kind == TOKEN_KIND_LOGIC_OR) {
        advance(p);
        parse_and_expr(p);
    }
    trace_exit(p, "or_expr");
}

/* <expr> -> <or_expr> */
static void parse_expr(Parser *p) {
    parse_or_expr(p);
}

/* ================================================================= */
/* 6. ENTRY POINT                                                    */
/* ================================================================= */

/* <program> -> <data_section> <order_phase> <luto_phase> <hain_phase> <bayad_phase> [ { <statement> } ] */
static void parse_program(Parser *p) {
    trace_enter(p, "program");
    parse_data_section(p);
    parse_order_phase(p);
    parse_luto_phase(p);
    parse_hain_phase(p);
    parse_bayad_phase(p);

    /* Allow optional cleanup / trailing statements if present */
    while (is_statement_start(current(p)->kind)) {
        parse_statement(p);
    }

    expect(p, TOKEN_KIND_EOF, "Unexpected trailing tokens after program.");
    trace_exit(p, "program");
}

static void parser_init(Parser *p, const TokenList *list, int trace) {
    p->tokens = list->items;
    p->count = list->count;
    p->pos = 0;
    p->error_count = 0;
    p->panic_mode = 0;
    p->trace = trace;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.rice> [--trace]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int trace = 0;
    const char *filepath = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--trace") == 0 || strcmp(argv[i], "-t") == 0) {
            trace = 1;
        } else if (filepath == NULL) {
            filepath = argv[i];
        }
    }

    if (filepath == NULL || !valid_source_extension(filepath)) {
        fprintf(stderr, "%s\n", EXTENSION_ERROR_MESSAGE);
        return EXIT_FAILURE;
    }

    char *source = NULL;
    size_t length = 0;
    if (!lexer_read_source(filepath, &source, &length)) {
        fprintf(stderr, "Error reading source file '%s'.\n", filepath);
        return EXIT_FAILURE;
    }

    TokenList token_list;
    token_list_init(&token_list);
    g_token_collector = &token_list;

    /* Scan tokens using existing lexer DFA */
    int lex_errors = lexer_scan(source, length, NULL, NULL);
    g_token_collector = NULL;
    free(source);

    if (lex_errors > 0) {
        fprintf(stderr, "Lexical analysis failed with %d error(s). Halting parser.\n", lex_errors);
        token_list_free(&token_list);
        return EXIT_FAILURE;
    }

    Parser p;
    parser_init(&p, &token_list, trace);
    parse_program(&p);

    int had_error = (p.error_count > 0);
    if (!had_error) {
        printf("Syntax analysis completed successfully. Program '%s' is syntactically valid.\n", filepath);
    } else {
        fprintf(stderr, "Syntax analysis completed with %d error(s).\n", p.error_count);
    }

    token_list_free(&token_list);
    return had_error ? EXIT_FAILURE : EXIT_SUCCESS;
}
