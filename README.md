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

The symbol table starts with a clean token table (line, column, lexeme, token,
and category), followed by the character-by-character scan records and DFA
state transitions. Noise words are included in the token table as
`IGNORED_NOISE_WORD` so later compiler phases can omit them from the syntax
stream.

### Lexeme & token tables

The counts below match the lexeme definitions in `lexer.c`: 10 keywords,
20 reserved words, 10 noise words, 17 operation symbols, and 4 delimiters
and brackets.

#### Keywords (10)

| Lexeme | Token |
| --- | --- |
| `LETRA` | `TOKEN_KW_LETRA` |
| `SALITA` | `TOKEN_KW_SALITA` |
| `BUO` | `TOKEN_KW_BUO` |
| `NUMERO` | `TOKEN_KW_NUMERO` |
| `KUNDISYON` | `TOKEN_KW_KUNDISYON` |
| `KUNG` | `TOKEN_KW_KUNG` |
| `KUNDI` | `TOKEN_KW_KUNDI` |
| `ULIT` | `TOKEN_KW_ULIT` |
| `HINTO` | `TOKEN_KW_HINTO` |
| `TULOY` | `TOKEN_KW_TULOY` |

#### Reserved keywords (20)

| Lexeme | Token |
| --- | --- |
| `TAMA` | `TOKEN_RES_TAMA` |
| `MALI` | `TOKEN_RES_MALI` |
| `KATUMBAS` | `TOKEN_RES_KATUMBAS` |
| `ULAM` | `TOKEN_RES_ULAM` |
| `RESIBO` | `TOKEN_RES_RESIBO` |
| `ilagay` | `TOKEN_RES_ilagay` |
| `meron` | `TOKEN_RES_meron` |
| `kumuha` | `TOKEN_RES_kumuha` |
| `dagdag` | `TOKEN_RES_dagdag` |
| `bawas` | `TOKEN_RES_bawas` |
| `kalkula` | `TOKEN_RES_kalkula` |
| `diskwento` | `TOKEN_RES_diskwento` |
| `bayaran` | `TOKEN_RES_bayaran` |
| `ilimbag` | `TOKEN_RES_ilimbag` |
| `linisin` | `TOKEN_RES_linisin` |
| `hatian` | `TOKEN_RES_hatian` |
| `ORDER` | `TOKEN_RES_ORDER` |
| `LUTO` | `TOKEN_RES_LUTO` |
| `HAIN` | `TOKEN_RES_HAIN` |
| `BAYAD` | `TOKEN_RES_BAYAD` |

#### Noise words (10)

| Lexeme | Token |
| --- | --- |
| `po` | `TOKEN_NOISE_WORD` |
| `na` | `TOKEN_NOISE_WORD` |
| `lang` | `TOKEN_NOISE_WORD` |
| `at` | `TOKEN_NOISE_WORD` |
| `nang` | `TOKEN_NOISE_WORD` |
| `ng` | `TOKEN_NOISE_WORD` |
| `ang` | `TOKEN_NOISE_WORD` |
| `ay` | `TOKEN_NOISE_WORD` |
| `nito` | `TOKEN_NOISE_WORD` |
| `opo` | `TOKEN_NOISE_WORD` |

#### Operation symbols (17)

| Lexeme | Token |
| --- | --- |
| `+` | `TOKEN_OP_ADD` |
| `-` | `TOKEN_OP_SUB` |
| `*` | `TOKEN_OP_MUL` |
| `/` | `TOKEN_OP_DIV` |
| `%` | `TOKEN_OP_MOD` |
| `\` | `TOKEN_OP_INTDIV` |
| `^` | `TOKEN_OP_POW` |
| `=` | `TOKEN_ASSIGNMENT` |
| `==` | `TOKEN_REL_EQ` |
| `!=` | `TOKEN_REL_NEQ` |
| `>` | `TOKEN_REL_GT` |
| `>=` | `TOKEN_REL_GTE` |
| `<` | `TOKEN_REL_LT` |
| `<=` | `TOKEN_REL_LTE` |
| `!` | `TOKEN_LOGIC_NOT` |
| `&&` | `TOKEN_LOGIC_AND` |
| `||` | `TOKEN_LOGIC_OR` |

#### Delimiters & brackets (4)

| Lexeme | Token |
| --- | --- |
| `(` | `TOKEN_LPAREN` |
| `)` | `TOKEN_RPAREN` |
| `{` | `TOKEN_LBRACE` |
| `}` | `TOKEN_RBRACE` |

The colon (`:`) is separately recognized as `TOKEN_ACCESSOR`. Punctuation
tokens are `.`, `,`, and `;`; `#` and `$` are recognized as
`TOKEN_SPECIAL_SYMBOL`.

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
