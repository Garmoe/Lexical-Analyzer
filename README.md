# JAVArice C Lexical Analyzer

This folder contains an independent C implementation of the JAVArice lexer.
It reads one character at a time, simulates a unified finite automaton whose
start state is `q0`, prints each transition, and writes accepted tokens to
`Symbol Table.txt`.

## Build and run

From this folder, using GCC:

```powershell
gcc -std=c11 -Wall -Wextra -pedantic lexer.c -o lexer.exe
.\lexer.exe .\program.rice
```

The input must be a regular, printable-ASCII text file with the `.rice`
extension. The output is created in the current working directory with the
required exact name `Symbol Table.txt`.

## Automaton and token design

All words and symbols begin at `q0`. The transition table is constructed from
per-character edges; source words are never compared as complete strings.
Exact DFA edges take precedence over the generic identifier transitions.
When a word reaches a boundary, its accepting state determines its token
classification. A failed keyword path transitions to `q_id`, so a word such
as `ORDERX` is an identifier, not the `ORDER` keyword followed by another
token.

The symbol table includes scan records, DFA state transitions, and grouped
token records. Noise words are classified as `IGNORED_NOISE_WORD` so later
compiler phases can omit them from the syntax stream.

### Keywords and reserved words

`LETRA`, `SALITA`, `BUO`, `NUMERO`, `KUNDISYON`, `KUNG`, `KUNDI`, `ULIT`,
`HINTO`, `TULOY`, `WALA`, `TAMA`, `MALI`, `KATUMBAS`

### Built-in objects, methods, and transaction verbs

Objects: `ULAM`, `RESIBO`

Methods: `ilagay`, `meron`, `kumuha`, `dagdag`, `bawas`, `kalkula`,
`diskwento`, `bayaran`, `ilimbag`, `linisin`, `hatian`

Transaction verbs: `ORDER`, `LUTO`, `HAIN`, `BAYAD`

### Noise words

`po`, `na`, `lang`, `at`, `nang`, `ng`, `ang`, `ay`, `nito`, `opo`

### Other lexical elements

- Identifiers: ASCII letters or `_` to start; letters, digits, and `_` after.
- Numeric literals: integers and decimals with digits on both sides of `.`.
- String literals: double-quoted text on one source line.
- Punctuation and delimiters: `. , : ( ) { }`.
- Arithmetic operators: `+ - * / % \ ^`.
- Assignment and relational operators: `= == != > >= < <=`.
- Logical operators: `! && ||`.
- Comments: `@` through the end of the line.
- Whitespace: spaces, tabs, and line endings.

Unsupported control characters, non-ASCII bytes, unknown symbols, and
unterminated strings are reported as lexical errors. The analyzer still writes
the symbol-table file when lexical errors are found and exits with a failure
status.
