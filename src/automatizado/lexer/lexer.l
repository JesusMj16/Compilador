%option noyywrap
%option nounput noinput

%{
/*
 * Lexer Flex para el lenguaje reducido.
 * - Alineado con docs/Analizador-Lexico/docs/reglas.md y tokens.md
 * - Integrado con Bison mediante parser.tab.h (tokens y YYSTYPE).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* En esta rama automatizada, el lexer depende del header generado por Bison. */
#include "parser.tab.h"

int yycolumn = 1;
int yylineno = 1;

static int comment_start_line = 1;
static int comment_start_col  = 1;

static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

static void advance_position(const char *text, int len) {
    /* Actualiza yylineno/yycolumn para cualquier lexema (incl. comentarios). */
    for (int i = 0; i < len; i++) {
        if (text[i] == '\n') {
            yylineno++;
            yycolumn = 1;
        } else {
            yycolumn++;
        }
    }
}

static void lex_error_at(int line, int col, const char *msg, const char *lexeme) {
    if (lexeme) {
        fprintf(stderr, "Error léxico [%d:%d]: %s: '%s'\n", line, col, msg, lexeme);
    } else {
        fprintf(stderr, "Error léxico [%d:%d]: %s\n", line, col, msg);
    }
}
%}

%x COMMENT

DIGIT          [0-9]
LETTER         [A-Za-z]
ID_START       ({LETTER}|_)
ID_CONT        ({LETTER}|{DIGIT}|_)

INT            {DIGIT}+
REAL           {DIGIT}+\.{DIGIT}+([eE][+-]?{DIGIT}+)?

WS             [ \t]+
NL             (\r\n|\n)

%%

/* --- Espacios y comentarios (se consumen) --- */
{WS}            { advance_position(yytext, yyleng); /* ignorar */ }
{NL}            { advance_position(yytext, yyleng); /* ignorar */ }

"//"[^\r\n]*   { advance_position(yytext, yyleng); /* ignorar */ }

"/*"            {
                    comment_start_line = yylineno;
                    comment_start_col  = yycolumn;
                    advance_position(yytext, yyleng);
                    BEGIN(COMMENT);
                }

<COMMENT>"*/"   {
                    advance_position(yytext, yyleng);
                    BEGIN(INITIAL);
                }

<COMMENT><<EOF>> {
                    lex_error_at(comment_start_line, comment_start_col, "comentario de bloque sin cerrar", NULL);
                    BEGIN(INITIAL);
                    return TOKEN_EOF;
                }

<COMMENT>.|{NL}|{WS} {
                    advance_position(yytext, yyleng);
                    /* seguir consumiendo comentario */
                }

/* --- Palabras reservadas y tipos (antes que ID) --- */
"fn"            { advance_position(yytext,yyleng); return TOKEN_KW_FN; }
"let"           { advance_position(yytext,yyleng); return TOKEN_KW_LET; }
"mut"           { advance_position(yytext,yyleng); return TOKEN_KW_MUT; }
"if"            { advance_position(yytext,yyleng); return TOKEN_KW_IF; }
"else"          { advance_position(yytext,yyleng); return TOKEN_KW_ELSE; }
"return"        { advance_position(yytext,yyleng); return TOKEN_KW_RETURN; }
"true"          { advance_position(yytext,yyleng); return TOKEN_KW_TRUE; }
"false"         { advance_position(yytext,yyleng); return TOKEN_KW_FALSE; }
"i32"           { advance_position(yytext,yyleng); return TOKEN_KW_I32; }
"f64"           { advance_position(yytext,yyleng); return TOKEN_KW_F64; }
"bool"          { advance_position(yytext,yyleng); return TOKEN_KW_BOOL; }

/* --- Operadores multi-caracter (prioridad) --- */
"=="            { advance_position(yytext,yyleng); return TOKEN_EQUAL_EQUAL; }
"!="            { advance_position(yytext,yyleng); return TOKEN_BANG_EQUAL; }
"<="            { advance_position(yytext,yyleng); return TOKEN_LESS_EQUAL; }
">="            { advance_position(yytext,yyleng); return TOKEN_GREATER_EQUAL; }
"&&"            { advance_position(yytext,yyleng); return TOKEN_AND_AND; }
"||"            { advance_position(yytext,yyleng); return TOKEN_OR_OR; }

/* --- Números --- */
{REAL}          {
                    /* El valor semántico (si existe) es el lexema. */
                    yylval.lexeme = xstrdup(yytext);
                    advance_position(yytext, yyleng);
                    return TOKEN_NUMBER;
                }

/* Mal formado: 12e, 12e+, 12E- (sin dígitos de exponente) => error léxico */
{INT}[eE][+-]?  {
                    int start_line = yylineno;
                    int start_col  = yycolumn;
                    advance_position(yytext, yyleng);
                    lex_error_at(start_line, start_col, "literal numérico con exponente incompleto", yytext);
                    return TOKEN_UNKNOWN;
                }

{INT}           {
                    yylval.lexeme = xstrdup(yytext);
                    advance_position(yytext, yyleng);
                    return TOKEN_NUMBER;
                }

/* --- Identificadores --- */
{ID_START}{ID_CONT}* {
                    int start_line = yylineno;
                    int start_col  = yycolumn;

                    /* Validación normativa: prohibido '__' consecutivo. */
                    if (strstr(yytext, "__") != NULL) {
                        advance_position(yytext, yyleng);
                        lex_error_at(start_line, start_col, "identificador inválido (contiene '__')", yytext);
                        return TOKEN_UNKNOWN;
                    }

                    yylval.lexeme = xstrdup(yytext);
                    advance_position(yytext, yyleng);
                    return TOKEN_IDENTIFIER;
                }

/* --- Operadores de 1 caracter --- */
"+"             { advance_position(yytext,yyleng); return TOKEN_PLUS; }
"-"             { advance_position(yytext,yyleng); return TOKEN_MINUS; }
"*"             { advance_position(yytext,yyleng); return TOKEN_STAR; }
"/"             { advance_position(yytext,yyleng); return TOKEN_SLASH; }
"%"             { advance_position(yytext,yyleng); return TOKEN_PERCENT; }
"="             { advance_position(yytext,yyleng); return TOKEN_EQUAL; }
"!"             { advance_position(yytext,yyleng); return TOKEN_BANG; }
"<"             { advance_position(yytext,yyleng); return TOKEN_LESS; }
">"             { advance_position(yytext,yyleng); return TOKEN_GREATER; }

/* --- Delimitadores --- */
";"             { advance_position(yytext,yyleng); return TOKEN_SEMICOLON; }
","             { advance_position(yytext,yyleng); return TOKEN_COMMA; }
":"             { advance_position(yytext,yyleng); return TOKEN_COLON; }
"("             { advance_position(yytext,yyleng); return TOKEN_LPAREN; }
")"             { advance_position(yytext,yyleng); return TOKEN_RPAREN; }
"{"             { advance_position(yytext,yyleng); return TOKEN_LBRACE; }
"}"             { advance_position(yytext,yyleng); return TOKEN_RBRACE; }

/* Punto aislado => error léxico (según especificación) */
"."             {
                    int start_line = yylineno;
                    int start_col  = yycolumn;
                    advance_position(yytext, yyleng);
                    lex_error_at(start_line, start_col, "punto aislado (solo permitido en reales)", yytext);
                    return TOKEN_UNKNOWN;
                }

<<EOF>>         { return TOKEN_EOF; }

/* Carácter no reconocido */
.               {
                    int start_line = yylineno;
                    int start_col  = yycolumn;
                    advance_position(yytext, yyleng);
                    lex_error_at(start_line, start_col, "carácter no reconocido", yytext);
                    return TOKEN_UNKNOWN;
                }

%%
