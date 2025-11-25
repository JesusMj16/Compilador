#ifndef LR_TABLES_H
#define LR_TABLES_H

#include <stddef.h>
#include <stdbool.h>
#include "lexer.h"

#define LR_MAX_RHS 6

typedef enum {
    LR_TERM_KW_FN,
    LR_TERM_KW_LET,
    LR_TERM_KW_MUT,
    LR_TERM_KW_IF,
    LR_TERM_KW_ELSE,
    LR_TERM_KW_RETURN,
    LR_TERM_KW_TRUE,
    LR_TERM_KW_FALSE,
    LR_TERM_KW_I32,
    LR_TERM_KW_F64,
    LR_TERM_KW_BOOL,
    LR_TERM_IDENTIFIER,
    LR_TERM_NUMBER,
    LR_TERM_PLUS,
    LR_TERM_MINUS,
    LR_TERM_STAR,
    LR_TERM_SLASH,
    LR_TERM_PERCENT,
    LR_TERM_EQUAL,
    LR_TERM_EQUAL_EQUAL,
    LR_TERM_BANG,
    LR_TERM_BANG_EQUAL,
    LR_TERM_LESS,
    LR_TERM_LESS_EQUAL,
    LR_TERM_GREATER,
    LR_TERM_GREATER_EQUAL,
    LR_TERM_AND_AND,
    LR_TERM_OR_OR,
    LR_TERM_SEMICOLON,
    LR_TERM_COMMA,
    LR_TERM_COLON,
    LR_TERM_LPAREN,
    LR_TERM_RPAREN,
    LR_TERM_LBRACE,
    LR_TERM_RBRACE,
    LR_TERM_EOF,
    LR_TERM_COUNT
} LRTerminal;

typedef enum {
    LR_NT_START,
    LR_NT_PROGRAMA,
    LR_NT_LISTA_ITEMS,
    LR_NT_ITEM,
    LR_NT_FUNCION,
    LR_NT_LISTA_PARAM_OPT,
    LR_NT_LISTA_PARAM,
    LR_NT_LISTA_PARAM_TAIL,
    LR_NT_PARAMETRO,
    LR_NT_TIPO,
    LR_NT_BLOQUE,
    LR_NT_LISTA_SENTENCIAS,
    LR_NT_SENTENCIA,
    LR_NT_LET_SENTENCIA,
    LR_NT_MUT_OPT,
    LR_NT_ANOT_TIPO_OPT,
    LR_NT_INIT_OPT,
    LR_NT_EXPR_SENTENCIA,
    LR_NT_RETURN_SENTENCIA,
    LR_NT_EXPRESION_OPT,
    LR_NT_IF_SENTENCIA,
    LR_NT_ELSE_OPT,
    LR_NT_IF_ELSE_BODY,
    LR_NT_EXPRESION,
    LR_NT_ASIGNACION,
    LR_NT_ASIGNACION_TAIL,
    LR_NT_LOGICO_OR,
    LR_NT_LOGICO_OR_TAIL,
    LR_NT_LOGICO_AND,
    LR_NT_LOGICO_AND_TAIL,
    LR_NT_IGUALDAD,
    LR_NT_IGUALDAD_TAIL,
    LR_NT_COMPARACION,
    LR_NT_COMPARACION_TAIL,
    LR_NT_ADITIVO,
    LR_NT_ADITIVO_TAIL,
    LR_NT_MULTIPLICATIVO,
    LR_NT_MULTIPLICATIVO_TAIL,
    LR_NT_UNARIO,
    LR_NT_OPERADOR_UNARIO,
    LR_NT_POSTFIJO,
    LR_NT_POSTFIJO_TAIL,
    LR_NT_LLAMADA,
    LR_NT_LISTA_ARG_OPT,
    LR_NT_LISTA_ARG,
    LR_NT_LISTA_ARG_TAIL,
    LR_NT_PRIMARIO,
    LR_NT_LITERAL,
    LR_NT_BOOLEANO,
    LR_NONTERM_COUNT
} LRNonTerminal;

#define LR_SYMBOL_COUNT (LR_TERM_COUNT + LR_NONTERM_COUNT)
#define LR_SYMBOL_IS_TERMINAL(sym) ((sym) >= 0 && (sym) < LR_TERM_COUNT)
#define LR_SYMBOL_IS_NONTERM(sym) ((sym) >= LR_TERM_COUNT)
#define LR_SYMBOL_FROM_TERMINAL(term) (term)
#define LR_SYMBOL_FROM_NONTERM(nt) (LR_TERM_COUNT + (nt))
#define LR_SYMBOL_TO_NONTERM(sym) ((sym) - LR_TERM_COUNT)

typedef struct {
    int lhs;
    int rhs_len;
    int rhs[LR_MAX_RHS];
    const char *description;
} LRProduction;

typedef enum {
    LR_ACTION_ERROR = 0,
    LR_ACTION_SHIFT,
    LR_ACTION_REDUCE,
    LR_ACTION_ACCEPT
} LRActionType;

typedef struct {
    LRActionType type;
    int value;
} LRAction;

typedef struct {
    LRAction *action;     /* state x terminal */
    int *gotos;           /* state x nonterminal */
    size_t state_count;
} LRParseTable;

typedef struct LRReductionNode {
    int symbol;
    char *lexeme;
    size_t line;
    size_t column;
    struct LRReductionNode **children;
    size_t child_count;
    size_t capacity;
} LRReductionNode;

extern const size_t LR_PRODUCTION_COUNT;

const LRParseTable* lr_get_parse_table(void);
const LRProduction* lr_get_productions(size_t *count);
const char* lr_symbol_name(int symbol);
const char* lr_terminal_name(int terminal);
const char* lr_nonterminal_name(int nonterminal);
int lr_terminal_from_token(TokenType type);

#endif /* LR_TABLES_H */
