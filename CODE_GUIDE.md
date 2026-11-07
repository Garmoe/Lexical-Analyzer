# JAVArice Lexical Analyzer: Functional and Operational Guide

**Project:** JAVArice C Lexer  
**Purpose:** Explain the current implementation, from source-file input to token report  
**Primary modules:** `main.c`, `lexer.c`, and `dfa.c`

> **Scope note:** This guide describes the code currently in this branch. It
> focuses on how the implementation works; it is not a language grammar or a
> parser design.

---

## Contents

1. [Executive summary and architecture](#1-executive-summary-and-architecture)
2. [Function inventory](#2-function-inventory)
3. [Data and state lifecycle](#3-data-and-state-lifecycle)
4. [Operational trace cases](#4-operational-trace-cases)
5. [Token and DFA reference](#5-token-and-dfa-reference)
6. [Build, run, and verify](#6-build-run-and-verify)
7. [Where to make changes](#7-where-to-make-changes)

---

## 1. Executive summary and architecture

The lexer reads a `.rice` file and groups its source characters into tokens:
keywords, identifiers, literals, operators, delimiters, comments, and invalid
input. It does **not** parse statements, build a syntax tree, or validate
program meaning and types.

```text
  .rice source file
         |
         v
  lexer_read_source()
  Validate path, encoding, and text
         |
         v
  lexer_scan()
  Track position and recognize each lexical element
      |                    |
      | words/symbols      | literals/comments/errors
      v                    v
  dfa_step()          lexer_scan() branches
  dfa_find_accept()   Bound and classify the lexeme
      |                    |
      +---------+----------+
                v
    Temporary token and trace streams
                |
                v
    main() assembles "Symbol Table"
```

### Architectural pillars

1. **Explicit input validation.** `lexer_read_source` requires a lowercase
   `.rice` extension and a regular file. It accepts printable ASCII,
   tab/newline/carriage-return whitespace, and supported UTF-8 bytes for
   `ñ`/`Ñ`.
2. **A DFA for words and symbols.** The current state plus next character
   determines the next state. Word prefixes are encoded as named states in
   `dfa.c`; symbol states handle punctuation and compound operators.
3. **Accepting-state metadata.** A state-to-token array maps final DFA states
   to `TokenKind` values and classifications. The lexer does not separately
   compare each completed word against a lexeme lookup table.
4. **Separate token and trace output.** The scanner writes token rows and scan
   records to temporary streams. `main` combines them into the final report.
5. **Explicit error count.** Errors are reported to stderr, represented as
   `INVALID` rows where the scanner reports them, and included in the return
   status.

> **Naming clarification:** `Symbol Table` is the output filename. The file is
> a token-and-trace report, not a compiler symbol table of variables and types.

### Project file map

| File | Responsibility |
|---|---|
| [`main.c`](./main.c) | Command-line entry point, stream setup, final report assembly, exit status. |
| [`lexer.c`](./lexer.c) | Source-file validation/reading and the main scanning loop. |
| [`lexer.h`](./lexer.h) | Public declarations for source loading and scanning. |
| [`dfa.c`](./dfa.c) | DFA states and transitions, accepting metadata, token names, output helpers. |
| [`dfa.h`](./dfa.h) | DFA constants and function declarations shared by modules. |
| [`token.h`](./token.h) | `TokenKind` enumeration and `AcceptingState` structure. |
| `program.rice` | Main example input. |
| `program_error_handling.rice` | Error-handling example input. |
| `program_invalid.rice` | Invalid-token example input. |

---

## 2. Function inventory

The following inventory covers each function defined in the three C modules.
Functions marked **static** are private implementation helpers in their file.

### `main.c` — driver and report assembly

| Function | Signature | Purpose |
|---|---|---|
| `copy_stream` **(static)** | `int copy_stream(FILE *source, FILE *destination)` | Rewinds a temporary stream and copies it in blocks; returns failure on stream I/O errors. |
| `main` | `int main(int argc, char **argv)` | Validates arguments, reads source, runs the scan, assembles the report, and sets process status. |

### `lexer.c` — input and token scanning

| Function | Signature | Purpose |
|---|---|---|
| `write_trace_str` **(static)** | `void write_trace_str(FILE *out, const char *kind, size_t line, size_t column, const char *display, const char *classification)` | Writes a scan record for a display string, including a multi-byte character. |
| `write_transition_str` **(static)** | `void write_transition_str(FILE *out, size_t line, size_t column, int from, const char *display, int to)` | Writes a DFA transition whose displayed input may use multiple bytes. |
| `write_trace` **(static)** | `void write_trace(FILE *out, const char *kind, size_t line, size_t column, int c, const char *classification)` | Writes a one-byte scan record; renders newline, carriage return, and tab visibly. |
| `write_transition` **(static)** | `void write_transition(FILE *out, size_t line, size_t column, int from, int c, int to)` | Writes a one-byte DFA transition with visible whitespace rendering. |
| `append_char` **(static)** | `int append_char(char **buffer, size_t *length, size_t *capacity, int c)` | Grows the source buffer as needed, appends a byte, and maintains a null terminator. |
| `valid_source_extension` **(static)** | `int valid_source_extension(const char *path)` | Checks whether the path ends in lowercase `.rice`. |
| `lexer_read_source` | `int lexer_read_source(const char *path, char **source, size_t *length)` | Validates and reads a source file, returning its allocated buffer and byte length. |
| `lexer_scan` | `int lexer_scan(const char *source, size_t length, FILE *log, FILE *tokens)` | Scans source, writes trace/token streams, and returns the lexical-error count. |

### `dfa.c` — character rules, transitions, and output

| Function | Signature | Purpose |
|---|---|---|
| `dfa_is_filipino_letter` | `int dfa_is_filipino_letter(const unsigned char *source, size_t position, size_t length)` | Recognizes the supported UTF-8 byte sequences for `ñ` and `Ñ`. |
| `is_identifier_start` **(static)** | `int is_identifier_start(int character)` | Tests whether one byte can start an identifier. |
| `is_identifier_part` **(static)** | `int is_identifier_part(int character)` | Tests whether one byte can continue an identifier. |
| `dfa_is_identifier_start` | `int dfa_is_identifier_start(const char *source, size_t position, size_t length)` | Checks an identifier's first source character, including `ñ`/`Ñ`. |
| `dfa_is_identifier_part` | `int dfa_is_identifier_part(const char *source, size_t position, size_t length)` | Checks a following identifier character, including `ñ`/`Ñ`. |
| `dfa_is_whitespace` | `int dfa_is_whitespace(int character)` | Recognizes space, tab, carriage return, and newline. |
| `dfa_find_accept` | `const AcceptingState *dfa_find_accept(int state)` | Finds token metadata for an accepting state, or returns `NULL`. |
| `dfa_step_word` **(static)** | `static int dfa_step_word(int state, int character)` | Advances the explicit word-prefix transitions. |
| `dfa_step_symbol` **(static)** | `static int dfa_step_symbol(int state, int character)` | Advances symbol/operator states, including compound operators. |
| `dfa_step` | `int dfa_step(int state, int character)` | Dispatches one transition to word, symbol, or general-state logic. |
| `dfa_token_kind_name` | `const char *dfa_token_kind_name(TokenKind kind)` | Returns the text label for a token kind. |
| `dfa_write_symbol_span` | `void dfa_write_symbol_span(FILE *out, const char *type, const char *lexeme, size_t length)` | Writes a token-table row from a lexeme span. |
| `dfa_write_symbol` | `void dfa_write_symbol(FILE *out, TokenKind kind, const char *lexeme)` | Writes a token-table row from a null-terminated lexeme. |
| `dfa_write_acceptance_span` | `void dfa_write_acceptance_span(FILE *out, const char *lexeme, size_t length, TokenKind kind, const char *classification, int state)` | Writes an acceptance record from a lexeme span and token metadata. |
| `dfa_write_acceptance` | `void dfa_write_acceptance(FILE *out, const char *lexeme, TokenKind kind, const char *classification, int state)` | Null-terminated-lexeme wrapper for the acceptance-span writer. |
| `dfa_report_error` | `void dfa_report_error(FILE *out, size_t line, size_t column, const char *message)` | Prints a diagnostic to stderr and writes an `INVALID` row. |

---

## 3. Data and state lifecycle

### Input buffer lifecycle

```text
path argument
     |
     v
extension + regular-file checks
     |
     v
fgetc() -> validate each byte -> append_char()
     |
     v
allocated, null-terminated buffer + explicit byte length
     |
     v
lexer_scan() -> free(source) in main()
```

- The buffer grows dynamically, beginning at 64 bytes and doubling when needed.
- The source length is tracked separately from the terminating `'\0'`.
- CR/LF and UTF-8 characters mean byte offsets and displayed columns are not
  always the same measurement. The scanner advances by two bytes for supported
  `ñ`/`Ñ` while advancing the displayed column once.
- If an unsupported byte sequence is found while reading, the source is
  rejected before token scanning begins.

### Scanner-local state

There is no global scanner cursor or global `charClass`. `lexer_scan` keeps
these local values:

| Variable | Meaning |
|---|---|
| `pos` | Current byte offset in the source buffer. |
| `line` | Current one-based source line. |
| `column` | Current one-based displayed column. |
| `errors` | Number of lexical errors detected so far. |
| `token_line`, `token_column` | Start location captured for the current lexical item. |

The DFA state is also maintained locally in the relevant scan branch, rather
than as a single global state for the whole file.

### Output lifecycle

`main` creates two temporary streams:

| Stream | Contents |
|---|---|
| `token_table` | Token type and lexeme rows, including `LINE_COMMENT` and `INVALID` rows. |
| `scan_log` | Character classifications, DFA transitions, and acceptance/rejection records. |

After scanning, `copy_stream` rewinds and copies the token stream to
`Symbol Table`, writes the trace-section heading, then rewinds and copies the
scan log. The temporary streams are closed before the process exits.

---

## 4. Operational trace cases

These are representative walks through the current implementation. State
names/numbers refer to `dfa.c`; token labels come from `dfa_token_kind_name`.

### Trace case 1 — Reading and reporting a source file

**Example command**

```powershell
.\lexer.exe .\program.rice
```

**Execution**

1. `main` verifies that exactly one path argument was supplied.
2. `lexer_read_source` checks the lowercase `.rice` extension and uses
   `stat` to require a regular file.
3. The file is opened in binary mode. Bytes are checked against the supported
   character policy and appended using `append_char`.
4. `main` creates `scan_log` and `token_table` with `tmpfile`.
5. `lexer_scan` processes the returned buffer.
6. `main` writes the report headers and copies both temporary streams into
   `Symbol Table`.
7. The input buffer and streams are released. The final process status reflects
   whether lexical errors occurred.

> Source-file validation errors happen before scanning; lexical errors happen
> during `lexer_scan`. They are different failure stages.

### Trace case 2 — Reserved word `BUO` versus an identifier

**Input**

```rice
BUO BUOLALA
```

For `BUO`, the word-state transitions are:

| Character | State transition | Meaning |
|---|---|---|
| `B` | `q0 -> q12` (`WORD_B`) | Start a word prefix. |
| `U` | `q12 -> q13` (`WORD_BU`) | Continue the reserved-word prefix. |
| `O` | `q13 -> q14` (`WORD_BUO`) | Reach the accepting keyword state. |

When the identifier scan ends, `dfa_find_accept(q14)` returns metadata for
`TOKEN_KIND_KW_BUO`, classified as `KEYWORD`. The token-name helper prints
this as `TOKEN_KW_BUO`.

`BUOLALA` shares the `BUO` prefix. When the following `L` is processed, the
word transition falls back to `STATE_IDENTIFIER` (`q900`); the rest of the
identifier remains in that state. Its final state is accepted as
`TOKEN_KIND_IDENTIFIER`, not `BUO`.

**Key distinction:** There is no final `strcmp("BUO", lexeme)` lookup here.
The scanner recognizes the word by its state path and final accepting state.

### Trace case 3 — Numeric literal and decimal-point lookahead

**Input**

```rice
120.00.
```

1. The digit branch starts from `STATE_START` (`q0`) and transitions to
   `STATE_INTEGER` (`q901`) for the first digit. Additional digits remain in
   `q901`.
2. The scanner treats `.` as part of the number only when there is a following
   digit. Here the next byte is `0`, so the dot transitions to
   `STATE_DECIMAL` (`q902`), followed by the fractional digits.
3. `dfa_find_accept(q902)` identifies a decimal literal, printed as
   `TOKEN_NUMERO_LITERAL`.
4. The final dot is not followed by a digit, so it remains for the next scan
   iteration and is recognized as `TOKEN_TERMINATOR`.

This lookahead allows a statement-ending period to be distinguished from a
decimal point without consuming the terminator as part of the number.

### Trace case 4 — Single and compound symbols

**Input**

```rice
= == && :
```

At each symbol position, `lexer_scan` first calls `dfa_step(STATE_START, c)`.
If a following byte exists, it tries one more transition and uses the
two-character form only if that resulting state is accepting.

| Lexeme | State path | Token |
|---|---|---|
| `=` | `q0 -> q934` | `TOKEN_ASSIGNMENT` |
| `==` | `q0 -> q934 -> q935` | `TOKEN_REL_EQ` |
| `&&` | `q0 -> q942 -> q943` | `TOKEN_LOGIC_AND` |
| `:` | `q0 -> q922` | `TOKEN_ACCESSOR` |

A lone `&` reaches a non-accepting state, so it is reported as an invalid
symbol rather than accepted as the logical-AND operator.

### Trace case 5 — UTF-8 `ñ` in an identifier

The reader accepts `ñ` (`C3 B1`) and `Ñ` (`C3 91`) as supported two-byte
UTF-8 sequences. The identifier helpers check both bytes together so the
letter can participate in identifier recognition.

For a source identifier such as `Kañin`:

1. The scanner reaches the `ñ` byte position while consuming the identifier.
2. `dfa_is_filipino_letter` recognizes the two-byte sequence.
3. The scanner advances `pos` by two bytes but advances `column` by one
   displayed character.
4. The trace helper prints the UTF-8 bytes together as one displayed
   character, and the original source bytes are preserved in the lexeme.

### Trace case 6 — Lexical errors and recovery

**Leading underscore**

```rice
_presyo
```

An identifier cannot start with `_`. The scanner logs the underscore and
transition to `STATE_ERROR` (`q905`), consumes the remainder of the
identifier-like sequence, reports the error at the token's starting
line/column, and increments the error count.

**Unsupported symbol**

```rice
~
```

The start-state transition for `~` is `STATE_ERROR`. The scanner reports an
unrecognized character, advances past it, and continues scanning later input.

**Unterminated string**

```rice
"Adobo
```

The string branch scans until a closing quote or line ending. Reaching the
line ending without a closing quote reports an unterminated string literal.

Errors reported through `dfa_report_error` are printed to stderr and written
as `INVALID` rows containing the diagnostic message. The report row describes
the error; it is not necessarily a copy of the exact source lexeme.

---

## 5. Token and DFA reference

### General and literal states

| State | Value | Role |
|---|---:|---|
| `STATE_START` | `0` | Start state for a lexical item. |
| `STATE_IDENTIFIER` | `900` | Identifier continuation/final state. |
| `STATE_INTEGER` | `901` | Integer literal state. |
| `STATE_DECIMAL` | `902` | Decimal literal state. |
| `STATE_STRING` | `903` | String scanning state. |
| `STATE_COMMENT` | `904` | Comment scanning state. |
| `STATE_ERROR` | `905` | Error transition state. |
| `STATE_CHAR` | `906` | Character literal state. |

Word prefixes have their own states (for example `WORD_B`, `WORD_BU`,
`WORD_BUO`). Symbol states use values in the `920`–`945` range.

### Accepting-state lookup

`accepting_states` in `dfa.c` maps a final state to:

- a `TokenKind`;
- a printable classification such as `KEYWORD`, `IDENTIFIER`,
  `ARITHMETIC_OPERATOR`, or `DECIMAL_LITERAL`;
- the `ignored` metadata field.

`dfa_find_accept` linearly searches this array and returns the matching entry
or `NULL`. The scanner makes decisions based on the accepting entry or its
absence. The `ignored` field is present in the structure but is not used by
the current scanning flow to suppress token output.

### Token categories

The `TokenKind` enumeration includes:

- language keywords and reserved/built-in words;
- identifiers and noise words;
- integer, decimal, character, string, and boolean literal categories;
- arithmetic, relational, logical, assignment, and accessor operators;
- delimiters, punctuation, comments, and invalid tokens.

The exact set of recognized spellings is defined by the word transitions and
accepting-state table in `dfa.c`.

### Output records

| Record | Purpose |
|---|---|
| `SCAN` | Shows the source position, displayed character, and lexical class. |
| `DFA` | Shows a state transition in the form `qFrom --[input]--> qTo`. |
| `ACCEPT` | Shows the lexeme, final state, token label, and classification. |
| `REJECT` | Shows a lexeme/state that did not map to an accepting token (used in symbol handling). |
| `INVALID` | Token-table row written for a reported lexical error. |

---

## 6. Build, run, and verify

### Compile

From the project directory:

```powershell
gcc -std=c11 -Wall -Wextra -pedantic main.c lexer.c dfa.c -o lexer.exe
```

### Run the included examples

```powershell
.\lexer.exe .\program.rice
.\lexer.exe .\program_error_handling.rice
.\lexer.exe .\program_invalid.rice
```

### Expected behavior

- The normal example should complete without lexical errors.
- The error-handling and invalid examples exercise error reporting.
- Each valid `.rice` input produces or replaces `Symbol Table` in the current
  working directory.
- A scan with lexical errors can still produce the report, but exits with
  `EXIT_FAILURE`.

---

## 7. Where to make changes

| Change request | Relevant code |
|---|---|
| Add a token kind | Add it to `TokenKind` in `token.h`; update `accepting_states` and `dfa_token_kind_name` in `dfa.c`. |
| Add/change a reserved word | Update the word-prefix transitions and accepting-state entry in `dfa.c`. |
| Add/change an operator | Update symbol transitions and accepting metadata in `dfa.c`. |
| Change identifier rules | Update the identifier helpers in `dfa.c` and the identifier branch in `lexer_scan`. |
| Change number/string/comment rules | Update the corresponding branch in `lexer_scan`; keep trace and position behavior consistent. |
| Change accepted source encoding | Update `lexer_read_source` and the character helpers in `dfa.c`. |
| Change report formatting | Update the assembly in `main.c` or the `dfa_write_*` helpers in `dfa.c`. |
| Change error diagnostics | Update the relevant `lexer_scan` branch and `dfa_report_error`. |

