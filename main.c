#include "lexer.h"
#include "dfa.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYMBOL_TABLE_FILE "Symbol Table"

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
    if (!lexer_read_source(argv[1], &source, &source_length)) {
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
        fprintf(stderr, "Cannot create '%s': %s\n",
                SYMBOL_TABLE_FILE, strerror(errno));
        fclose(scan_log);
        fclose(token_table);
        return EXIT_FAILURE;
    }
    fprintf(symbol_table, "%-24s | %s\n", "TOKEN TYPE", "LEXEME");
    fprintf(symbol_table,
            "------------------------ | ----------------------------------------\n");
    if (!copy_stream(token_table, symbol_table)) {
        fprintf(stderr, "Error writing token table to '%s'.\n",
                SYMBOL_TABLE_FILE);
        fclose(scan_log);
        fclose(token_table);
        fclose(symbol_table);
        return EXIT_FAILURE;
    }
    fprintf(symbol_table,
            "\nCHARACTER-BY-CHARACTER SCAN AND DFA TRANSITIONS\n");
    fprintf(symbol_table,
            "ACCEPT rows show the final accepting state and token category.\n"
            "REJECT rows show lexemes that reached no accepting token state.\n");
    if (!copy_stream(scan_log, symbol_table)) {
        fprintf(stderr, "Error writing DFA trace to '%s'.\n",
                SYMBOL_TABLE_FILE);
        fclose(scan_log);
        fclose(token_table);
        fclose(symbol_table);
        return EXIT_FAILURE;
    }
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
    if (errors != 0) {
        fprintf(stderr, "Lexical analysis completed with %d error(s). "
                        "See %s.\n", errors, SYMBOL_TABLE_FILE);
        return EXIT_FAILURE;
    }
    printf("Lexical analysis completed successfully. "
           "Symbol table written to %s.\n", SYMBOL_TABLE_FILE);
    return EXIT_SUCCESS;
}
