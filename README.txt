JAVARICE C LEXICAL ANALYZER
===========================

This folder contains an independent C implementation of the JAVArice lexer.
It reads the source one character at a time and uses a finite automaton,
starting at state q0, to recognize lexemes and produce tokens.


CLONING THE REPOSITORY
----------------------

Install Git for your operating system, then open a terminal in the folder
where you want the project and run:

Windows: https://git-scm.com/download/win
macOS: Git is included with Apple's Command Line Tools; install them with
       xcode-select --install

    git clone https://github.com/Garmoe/Lexical-Analyzer.git
    cd Lexical-Analyzer

You can then open this folder in VS Code. If you already have the project
files, skip this section.


DEPENDENCIES AND INSTALLATION
-----------------------------

The project has no third-party library dependencies. It requires GCC, a
C compiler, to build lexer.c. GCC's C standard library comes with the
compiler toolchain.

To install GCC on Windows:

1. Download and install MSYS2 from https://www.msys2.org/.
2. Open the MSYS2 UCRT64 terminal from the Start menu.
3. Install the GCC toolchain by running:

       pacman -S --needed mingw-w64-ucrt-x86_64-gcc

4. Add C:\msys64\ucrt64\bin to the Windows PATH if the installer used the
   default MSYS2 location. Restart VS Code or open a new terminal afterward.
5. Confirm that GCC is available:

       gcc --version

If MSYS2 was installed in a different folder, use that installation's
UCRT64\bin folder in PATH instead.

To install the C compiler on macOS:

1. Open Terminal.
2. Install Apple's Command Line Tools, which provide Clang, Git, and the
   standard C development tools:

       xcode-select --install

3. Follow the installer prompts, then confirm the compiler is available:

       clang --version


BUILDING AND RUNNING
--------------------

The source code is in one file: lexer.c.

To compile with GCC, open a terminal in this folder and run:

    gcc -std=c11 -Wall -Wextra -pedantic lexer.c -o lexer.exe

To run the lexer on the sample program:

    .\lexer.exe .\program.rice

To run the error-handling example:

    .\lexer.exe .\program_error_handling.rice

To run the invalid-token demonstration:

    .\lexer.exe .\program_invalid.rice

On macOS, compile with Clang from Terminal in this folder:

    clang -std=c11 -Wall -Wextra -pedantic lexer.c -o lexer

Run the sample program:

    ./lexer ./program.rice

Run the error-handling example:

    ./lexer ./program_error_handling.rice

Run the invalid-token demonstration:

    ./lexer ./program_invalid.rice

The input file must have the .rice extension and contain supported
printable ASCII text. The lexer creates or replaces a file named
"Symbol Table" (with no file extension)
in the current working directory.


FINITE AUTOMATON AND TOKENIZATION
---------------------------------

Words and symbols are recognized through per-character transitions in the
finite automaton. The lexer does not compare complete source words using
direct string-matching functions. Exact transitions for configured words
take precedence over the generic identifier transitions.

When a word reaches a boundary, its accepting state determines its token.
For example, ORDERX is recognized as an identifier, not as the ORDER
reserved word followed by another token.


SYMBOL TABLE OUTPUT
-------------------

The generated "Symbol Table" file starts with the token table, with two
columns:

    TOKEN TYPE               | LEXEME

Recognized comments appear as LINE_COMMENT rows. Lexical errors appear as
INVALID rows. Below the table, the output includes each scanned character,
the DFA state transitions, and ACCEPT or REJECT records. ACCEPT records show
the final state, token, and category, making it visible why a word is
recognized as a keyword, reserved word, or identifier.


LEXEME AND TOKEN TABLES
-----------------------

The counts below match the definitions in lexer.c: 11 keywords, 31 reserved
words, 11 noise words, 17 operation symbols, and 4 delimiters/brackets.

KEYWORDS (11)
Lexeme       Token
------------ ----------------------
LETRA        TOKEN_KW_LETRA
SALITA       TOKEN_KW_SALITA
BUO          TOKEN_KW_BUO
NUMERO       TOKEN_KW_NUMERO
KUNDISYON    TOKEN_KW_KUNDISYON
KUNG         TOKEN_KW_KUNG
KUNDI        TOKEN_KW_KUNDI
ULIT         TOKEN_KW_ULIT
HINTO        TOKEN_KW_HINTO
TULOY        TOKEN_KW_TULOY
WALA         TOKEN_KW_WALA

RESERVED WORDS (11 KEYWORDS + 20 = 31)
Lexeme       Token
------------ ----------------------
TAMA         TOKEN_RES_TAMA
MALI         TOKEN_RES_MALI
KATUMBAS     TOKEN_RES_KATUMBAS
ULAM         TOKEN_RES_ULAM
RESIBO       TOKEN_RES_RESIBO
ilagay       TOKEN_RES_ilagay
meron        TOKEN_RES_meron
kumuha       TOKEN_RES_kumuha
dagdag       TOKEN_RES_dagdag
bawas        TOKEN_RES_bawas
kalkula      TOKEN_RES_kalkula
diskwento    TOKEN_RES_diskwento
bayaran      TOKEN_RES_bayaran
ilimbag      TOKEN_RES_ilimbag
linisin      TOKEN_RES_linisin
hatian       TOKEN_RES_hatian
ORDER        TOKEN_RES_ORDER
LUTO         TOKEN_RES_LUTO
HAIN         TOKEN_RES_HAIN
BAYAD        TOKEN_RES_BAYAD

NOISE WORDS (11)
Lexeme       Token
------------ ----------------------
po           TOKEN_NOISE_WORD
na           TOKEN_NOISE_WORD
lang         TOKEN_NOISE_WORD
at           TOKEN_NOISE_WORD
nang         TOKEN_NOISE_WORD
ng           TOKEN_NOISE_WORD
ngani        TOKEN_NOISE_WORD
ang          TOKEN_NOISE_WORD
ay           TOKEN_NOISE_WORD
nito         TOKEN_NOISE_WORD
opo          TOKEN_NOISE_WORD

OPERATION SYMBOLS (17)
Lexeme       Token
------------ ----------------------
+            TOKEN_OP_ADD
-            TOKEN_OP_SUB
*            TOKEN_OP_MUL
/            TOKEN_OP_DIV
%            TOKEN_OP_MOD
\            TOKEN_OP_INTDIV
^            TOKEN_OP_POW
=            TOKEN_ASSIGNMENT
==           TOKEN_REL_EQ
!=           TOKEN_REL_NEQ
>            TOKEN_REL_GT
>=           TOKEN_REL_GTE
<            TOKEN_REL_LT
<=           TOKEN_REL_LTE
!            TOKEN_LOGIC_NOT
&&           TOKEN_LOGIC_AND
||           TOKEN_LOGIC_OR

DELIMITERS AND BRACKETS (4)
Lexeme       Token
------------ ----------------------
(            TOKEN_LPAREN
)            TOKEN_RPAREN
{            TOKEN_LBRACE
}            TOKEN_RBRACE

The colon (:) is recognized as TOKEN_ACCESSOR.
The punctuation symbols . and , are recognized as tokens (TOKEN_TERMINATOR and TOKEN_SEPARATOR).



OTHER LEXICAL ELEMENTS
----------------------

Identifiers:
    Start with an ASCII letter; subsequent characters may
    also include digits.

Numeric literals:
    Integers and decimal numbers with digits on both sides of the decimal
    point.

String literals:
    Double-quoted text on one source line.

Comments:
    Begin with @ and continue to the end of the line.

Whitespace:
    Spaces, tabs, and line endings.

Unsupported control characters, non-ASCII bytes, unknown symbols, and
unterminated strings are reported as lexical errors. The lexer writes the
symbol-table file when lexical errors are found and exits with a failure
status.
