#include "../../include/lr_tables.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define LR_TERMINAL_BIT(term) (1ULL << (term))

static const LRProduction g_lr_productions[] = {
    { LR_SYMBOL_FROM_NONTERM(LR_NT_START), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_PROGRAMA) }, "S' -> Programa" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_PROGRAMA), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ITEMS) }, "Programa -> ListaItems" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ITEMS), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_ITEM), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ITEMS) }, "ListaItems -> Item ListaItems" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ITEMS), 0, { 0 }, "ListaItems -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ITEM), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_FUNCION) }, "Item -> Funcion" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ITEM), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA) }, "Item -> Sentencia" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_FUNCION), 6, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_FN), LR_SYMBOL_FROM_TERMINAL(LR_TERM_IDENTIFIER), LR_SYMBOL_FROM_TERMINAL(LR_TERM_LPAREN), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM_OPT), LR_SYMBOL_FROM_TERMINAL(LR_TERM_RPAREN), LR_SYMBOL_FROM_NONTERM(LR_NT_BLOQUE) }, "Funcion -> fn IDENT '(' ListaParametrosOpt ')' Bloque" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM_OPT), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM) }, "ListaParametrosOpt -> ListaParametros" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM_OPT), 0, { 0 }, "ListaParametrosOpt -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_PARAMETRO), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM_TAIL) }, "ListaParametros -> Parametro ListaParametrosTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_COMMA), LR_SYMBOL_FROM_NONTERM(LR_NT_PARAMETRO), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM_TAIL) }, "ListaParametrosTail -> ',' Parametro ListaParametrosTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_PARAM_TAIL), 0, { 0 }, "ListaParametrosTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_PARAMETRO), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_IDENTIFIER), LR_SYMBOL_FROM_TERMINAL(LR_TERM_COLON), LR_SYMBOL_FROM_NONTERM(LR_NT_TIPO) }, "Parametro -> IDENT ':' Tipo" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_TIPO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_I32) }, "Tipo -> i32" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_TIPO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_F64) }, "Tipo -> f64" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_TIPO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_BOOL) }, "Tipo -> bool" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_TIPO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_IDENTIFIER) }, "Tipo -> IDENT" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_BLOQUE), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_LBRACE), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_SENTENCIAS), LR_SYMBOL_FROM_TERMINAL(LR_TERM_RBRACE) }, "Bloque -> '{' ListaSentencias '}'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_SENTENCIAS), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_SENTENCIAS) }, "ListaSentencias -> Sentencia ListaSentencias" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_SENTENCIAS), 0, { 0 }, "ListaSentencias -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_LET_SENTENCIA), LR_SYMBOL_FROM_TERMINAL(LR_TERM_SEMICOLON) }, "Sentencia -> LetSentencia ';'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPR_SENTENCIA), LR_SYMBOL_FROM_TERMINAL(LR_TERM_SEMICOLON) }, "Sentencia -> ExprSentencia ';'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_BLOQUE) }, "Sentencia -> Bloque" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_RETURN_SENTENCIA), LR_SYMBOL_FROM_TERMINAL(LR_TERM_SEMICOLON) }, "Sentencia -> ReturnSentencia ';'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_IF_SENTENCIA) }, "Sentencia -> IfSentencia" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_IF_SENTENCIA), 6, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_IF), LR_SYMBOL_FROM_TERMINAL(LR_TERM_LPAREN), LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION), LR_SYMBOL_FROM_TERMINAL(LR_TERM_RPAREN), LR_SYMBOL_FROM_NONTERM(LR_NT_BLOQUE), LR_SYMBOL_FROM_NONTERM(LR_NT_ELSE_OPT) }, "IfSentencia -> if '(' Expresion ')' Bloque ElseOpt" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ELSE_OPT), 2, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_ELSE), LR_SYMBOL_FROM_NONTERM(LR_NT_IF_ELSE_BODY) }, "ElseOpt -> else IfElseBody" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ELSE_OPT), 0, { 0 }, "ElseOpt -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_IF_ELSE_BODY), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_IF_SENTENCIA) }, "IfElseBody -> IfSentencia" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_IF_ELSE_BODY), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_BLOQUE) }, "IfElseBody -> Bloque" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LET_SENTENCIA), 5, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_LET), LR_SYMBOL_FROM_NONTERM(LR_NT_MUT_OPT), LR_SYMBOL_FROM_TERMINAL(LR_TERM_IDENTIFIER), LR_SYMBOL_FROM_NONTERM(LR_NT_ANOT_TIPO_OPT), LR_SYMBOL_FROM_NONTERM(LR_NT_INIT_OPT) }, "LetSentencia -> let MutOpt IDENT AnotacionTipoOpt InicializacionOpt" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_MUT_OPT), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_MUT) }, "MutOpt -> mut" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_MUT_OPT), 0, { 0 }, "MutOpt -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ANOT_TIPO_OPT), 2, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_COLON), LR_SYMBOL_FROM_NONTERM(LR_NT_TIPO) }, "AnotacionTipoOpt -> ':' Tipo" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ANOT_TIPO_OPT), 0, { 0 }, "AnotacionTipoOpt -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_INIT_OPT), 2, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_EQUAL), LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION) }, "InicializacionOpt -> '=' Expresion" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_INIT_OPT), 0, { 0 }, "InicializacionOpt -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPR_SENTENCIA), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION) }, "ExprSentencia -> Expresion" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_RETURN_SENTENCIA), 2, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_RETURN), LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION_OPT) }, "ReturnSentencia -> return ExpresionOpt" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION_OPT), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION) }, "ExpresionOpt -> Expresion" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION_OPT), 0, { 0 }, "ExpresionOpt -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_ASIGNACION) }, "Expresion -> Asignacion" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ASIGNACION), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_OR), LR_SYMBOL_FROM_NONTERM(LR_NT_ASIGNACION_TAIL) }, "Asignacion -> LogicoOR AsignacionTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ASIGNACION_TAIL), 2, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_EQUAL), LR_SYMBOL_FROM_NONTERM(LR_NT_ASIGNACION) }, "AsignacionTail -> '=' Asignacion" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ASIGNACION_TAIL), 0, { 0 }, "AsignacionTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_OR), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_AND), LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_OR_TAIL) }, "LogicoOR -> LogicoAND LogicoORTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_OR_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_OR_OR), LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_AND), LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_OR_TAIL) }, "LogicoORTail -> '||' LogicoAND LogicoORTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_OR_TAIL), 0, { 0 }, "LogicoORTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_AND), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD), LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_AND_TAIL) }, "LogicoAND -> Igualdad LogicoANDTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_AND_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_AND_AND), LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD), LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_AND_TAIL) }, "LogicoANDTail -> '&&' Igualdad LogicoANDTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LOGICO_AND_TAIL), 0, { 0 }, "LogicoANDTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION), LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD_TAIL) }, "Igualdad -> Comparacion IgualdadTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_EQUAL_EQUAL), LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION), LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD_TAIL) }, "IgualdadTail -> '==' Comparacion IgualdadTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_BANG_EQUAL), LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION), LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD_TAIL) }, "IgualdadTail -> '!=' Comparacion IgualdadTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_IGUALDAD_TAIL), 0, { 0 }, "IgualdadTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL) }, "Comparacion -> Aditivo ComparacionTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_LESS), LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL) }, "ComparacionTail -> '<' Aditivo ComparacionTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_GREATER), LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL) }, "ComparacionTail -> '>' Aditivo ComparacionTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_LESS_EQUAL), LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL) }, "ComparacionTail -> '<=' Aditivo ComparacionTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_GREATER_EQUAL), LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL) }, "ComparacionTail -> '>=' Aditivo ComparacionTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_COMPARACION_TAIL), 0, { 0 }, "ComparacionTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO_TAIL) }, "Aditivo -> Multiplicativo AditivoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_PLUS), LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO_TAIL) }, "AditivoTail -> '+' Multiplicativo AditivoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_MINUS), LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO), LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO_TAIL) }, "AditivoTail -> '-' Multiplicativo AditivoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_ADITIVO_TAIL), 0, { 0 }, "AditivoTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_UNARIO), LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL) }, "Multiplicativo -> Unario MultiplicativoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_STAR), LR_SYMBOL_FROM_NONTERM(LR_NT_UNARIO), LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL) }, "MultiplicativoTail -> '*' Unario MultiplicativoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_SLASH), LR_SYMBOL_FROM_NONTERM(LR_NT_UNARIO), LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL) }, "MultiplicativoTail -> '/' Unario MultiplicativoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_PERCENT), LR_SYMBOL_FROM_NONTERM(LR_NT_UNARIO), LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL) }, "MultiplicativoTail -> '%' Unario MultiplicativoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_MULTIPLICATIVO_TAIL), 0, { 0 }, "MultiplicativoTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_UNARIO), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_OPERADOR_UNARIO), LR_SYMBOL_FROM_NONTERM(LR_NT_UNARIO) }, "Unario -> OperadorUnario Unario" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_UNARIO), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_POSTFIJO) }, "Unario -> Postfijo" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_OPERADOR_UNARIO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_BANG) }, "OperadorUnario -> '!'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_OPERADOR_UNARIO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_MINUS) }, "OperadorUnario -> '-'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_OPERADOR_UNARIO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_PLUS) }, "OperadorUnario -> '+'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_POSTFIJO), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_PRIMARIO), LR_SYMBOL_FROM_NONTERM(LR_NT_POSTFIJO_TAIL) }, "Postfijo -> Primario PostfijoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_POSTFIJO_TAIL), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_LLAMADA), LR_SYMBOL_FROM_NONTERM(LR_NT_POSTFIJO_TAIL) }, "PostfijoTail -> Llamada PostfijoTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_POSTFIJO_TAIL), 0, { 0 }, "PostfijoTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LLAMADA), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_LPAREN), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG_OPT), LR_SYMBOL_FROM_TERMINAL(LR_TERM_RPAREN) }, "Llamada -> '(' ListaArgumentosOpt ')'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG_OPT), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG) }, "ListaArgumentosOpt -> ListaArgumentos" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG_OPT), 0, { 0 }, "ListaArgumentosOpt -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG), 2, { LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG_TAIL) }, "ListaArgumentos -> Expresion ListaArgumentosTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG_TAIL), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_COMMA), LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION), LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG_TAIL) }, "ListaArgumentosTail -> ',' Expresion ListaArgumentosTail" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ARG_TAIL), 0, { 0 }, "ListaArgumentosTail -> epsilon" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_PRIMARIO), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_LITERAL) }, "Primario -> Literal" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_PRIMARIO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_IDENTIFIER) }, "Primario -> IDENT" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_PRIMARIO), 3, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_LPAREN), LR_SYMBOL_FROM_NONTERM(LR_NT_EXPRESION), LR_SYMBOL_FROM_TERMINAL(LR_TERM_RPAREN) }, "Primario -> '(' Expresion ')'" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LITERAL), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_NUMBER) }, "Literal -> NUMBER" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_LITERAL), 1, { LR_SYMBOL_FROM_NONTERM(LR_NT_BOOLEANO) }, "Literal -> Booleano" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_BOOLEANO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_TRUE) }, "Booleano -> true" },
    { LR_SYMBOL_FROM_NONTERM(LR_NT_BOOLEANO), 1, { LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_FALSE) }, "Booleano -> false" }
};

const size_t LR_PRODUCTION_COUNT = sizeof(g_lr_productions) / sizeof(g_lr_productions[0]);

static const char *g_lr_terminal_names[LR_TERM_COUNT] = {
    [LR_TERM_KW_FN] = "fn",
    [LR_TERM_KW_LET] = "let",
    [LR_TERM_KW_MUT] = "mut",
    [LR_TERM_KW_IF] = "if",
    [LR_TERM_KW_ELSE] = "else",
    [LR_TERM_KW_RETURN] = "return",
    [LR_TERM_KW_TRUE] = "true",
    [LR_TERM_KW_FALSE] = "false",
    [LR_TERM_KW_I32] = "i32",
    [LR_TERM_KW_F64] = "f64",
    [LR_TERM_KW_BOOL] = "bool",
    [LR_TERM_IDENTIFIER] = "IDENT",
    [LR_TERM_NUMBER] = "NUMBER",
    [LR_TERM_PLUS] = "+",
    [LR_TERM_MINUS] = "-",
    [LR_TERM_STAR] = "*",
    [LR_TERM_SLASH] = "/",
    [LR_TERM_PERCENT] = "%",
    [LR_TERM_EQUAL] = "=",
    [LR_TERM_EQUAL_EQUAL] = "==",
    [LR_TERM_BANG] = "!",
    [LR_TERM_BANG_EQUAL] = "!=",
    [LR_TERM_LESS] = "<",
    [LR_TERM_LESS_EQUAL] = "<=",
    [LR_TERM_GREATER] = ">",
    [LR_TERM_GREATER_EQUAL] = ">=",
    [LR_TERM_AND_AND] = "&&",
    [LR_TERM_OR_OR] = "||",
    [LR_TERM_SEMICOLON] = ";",
    [LR_TERM_COMMA] = ",",
    [LR_TERM_COLON] = ":",
    [LR_TERM_LPAREN] = "(",
    [LR_TERM_RPAREN] = ")",
    [LR_TERM_LBRACE] = "{",
    [LR_TERM_RBRACE] = "}",
    [LR_TERM_EOF] = "EOF"
};

static const char *g_lr_nonterminal_names[LR_NONTERM_COUNT] = {
    [LR_NT_START] = "S'",
    [LR_NT_PROGRAMA] = "Programa",
    [LR_NT_LISTA_ITEMS] = "ListaItems",
    [LR_NT_ITEM] = "Item",
    [LR_NT_FUNCION] = "Funcion",
    [LR_NT_LISTA_PARAM_OPT] = "ListaParametrosOpt",
    [LR_NT_LISTA_PARAM] = "ListaParametros",
    [LR_NT_LISTA_PARAM_TAIL] = "ListaParametrosTail",
    [LR_NT_PARAMETRO] = "Parametro",
    [LR_NT_TIPO] = "Tipo",
    [LR_NT_BLOQUE] = "Bloque",
    [LR_NT_LISTA_SENTENCIAS] = "ListaSentencias",
    [LR_NT_SENTENCIA] = "Sentencia",
    [LR_NT_LET_SENTENCIA] = "LetSentencia",
    [LR_NT_MUT_OPT] = "MutOpt",
    [LR_NT_ANOT_TIPO_OPT] = "AnotacionTipoOpt",
    [LR_NT_INIT_OPT] = "InicializacionOpt",
    [LR_NT_EXPR_SENTENCIA] = "ExprSentencia",
    [LR_NT_RETURN_SENTENCIA] = "ReturnSentencia",
    [LR_NT_EXPRESION_OPT] = "ExpresionOpt",
    [LR_NT_IF_SENTENCIA] = "IfSentencia",
    [LR_NT_ELSE_OPT] = "ElseOpt",
    [LR_NT_IF_ELSE_BODY] = "IfElseBody",
    [LR_NT_EXPRESION] = "Expresion",
    [LR_NT_ASIGNACION] = "Asignacion",
    [LR_NT_ASIGNACION_TAIL] = "AsignacionTail",
    [LR_NT_LOGICO_OR] = "LogicoOR",
    [LR_NT_LOGICO_OR_TAIL] = "LogicoORTail",
    [LR_NT_LOGICO_AND] = "LogicoAND",
    [LR_NT_LOGICO_AND_TAIL] = "LogicoANDTail",
    [LR_NT_IGUALDAD] = "Igualdad",
    [LR_NT_IGUALDAD_TAIL] = "IgualdadTail",
    [LR_NT_COMPARACION] = "Comparacion",
    [LR_NT_COMPARACION_TAIL] = "ComparacionTail",
    [LR_NT_ADITIVO] = "Aditivo",
    [LR_NT_ADITIVO_TAIL] = "AditivoTail",
    [LR_NT_MULTIPLICATIVO] = "Multiplicativo",
    [LR_NT_MULTIPLICATIVO_TAIL] = "MultiplicativoTail",
    [LR_NT_UNARIO] = "Unario",
    [LR_NT_OPERADOR_UNARIO] = "OperadorUnario",
    [LR_NT_POSTFIJO] = "Postfijo",
    [LR_NT_POSTFIJO_TAIL] = "PostfijoTail",
    [LR_NT_LLAMADA] = "Llamada",
    [LR_NT_LISTA_ARG_OPT] = "ListaArgumentosOpt",
    [LR_NT_LISTA_ARG] = "ListaArgumentos",
    [LR_NT_LISTA_ARG_TAIL] = "ListaArgumentosTail",
    [LR_NT_PRIMARIO] = "Primario",
    [LR_NT_LITERAL] = "Literal",
    [LR_NT_BOOLEANO] = "Booleano"
};

const LRProduction* lr_get_productions(size_t *count) {
    if (count) {
        *count = LR_PRODUCTION_COUNT;
    }
    return g_lr_productions;
}

const char* lr_terminal_name(int terminal) {
    if (terminal < 0 || terminal >= LR_TERM_COUNT) {
        return "?";
    }
    return g_lr_terminal_names[terminal];
}

const char* lr_nonterminal_name(int nonterminal) {
    if (nonterminal < 0 || nonterminal >= (int)LR_NONTERM_COUNT) {
        return "?";
    }
    return g_lr_nonterminal_names[nonterminal];
}

const char* lr_symbol_name(int symbol) {
    if (LR_SYMBOL_IS_TERMINAL(symbol)) {
        return lr_terminal_name(symbol);
    }
    return lr_nonterminal_name(LR_SYMBOL_TO_NONTERM(symbol));
}

int lr_terminal_from_token(TokenType type) {
    switch (type) {
        case TOKEN_KW_FN: return LR_TERM_KW_FN;
        case TOKEN_KW_LET: return LR_TERM_KW_LET;
        case TOKEN_KW_MUT: return LR_TERM_KW_MUT;
        case TOKEN_KW_IF: return LR_TERM_KW_IF;
        case TOKEN_KW_ELSE: return LR_TERM_KW_ELSE;
        case TOKEN_KW_RETURN: return LR_TERM_KW_RETURN;
        case TOKEN_KW_TRUE: return LR_TERM_KW_TRUE;
        case TOKEN_KW_FALSE: return LR_TERM_KW_FALSE;
        case TOKEN_KW_I32: return LR_TERM_KW_I32;
        case TOKEN_KW_F64: return LR_TERM_KW_F64;
        case TOKEN_KW_BOOL: return LR_TERM_KW_BOOL;
        case TOKEN_IDENTIFIER: return LR_TERM_IDENTIFIER;
        case TOKEN_NUMBER: return LR_TERM_NUMBER;
        case TOKEN_PLUS: return LR_TERM_PLUS;
        case TOKEN_MINUS: return LR_TERM_MINUS;
        case TOKEN_STAR: return LR_TERM_STAR;
        case TOKEN_SLASH: return LR_TERM_SLASH;
        case TOKEN_PERCENT: return LR_TERM_PERCENT;
        case TOKEN_EQUAL: return LR_TERM_EQUAL;
        case TOKEN_EQUAL_EQUAL: return LR_TERM_EQUAL_EQUAL;
        case TOKEN_BANG: return LR_TERM_BANG;
        case TOKEN_BANG_EQUAL: return LR_TERM_BANG_EQUAL;
        case TOKEN_LESS: return LR_TERM_LESS;
        case TOKEN_LESS_EQUAL: return LR_TERM_LESS_EQUAL;
        case TOKEN_GREATER: return LR_TERM_GREATER;
        case TOKEN_GREATER_EQUAL: return LR_TERM_GREATER_EQUAL;
        case TOKEN_AND_AND: return LR_TERM_AND_AND;
        case TOKEN_OR_OR: return LR_TERM_OR_OR;
        case TOKEN_SEMICOLON: return LR_TERM_SEMICOLON;
        case TOKEN_COMMA: return LR_TERM_COMMA;
        case TOKEN_COLON: return LR_TERM_COLON;
        case TOKEN_LPAREN: return LR_TERM_LPAREN;
        case TOKEN_RPAREN: return LR_TERM_RPAREN;
        case TOKEN_LBRACE: return LR_TERM_LBRACE;
        case TOKEN_RBRACE: return LR_TERM_RBRACE;
        case TOKEN_EOF: return LR_TERM_EOF;
        default: return -1;
    }
}

typedef struct {
    uint64_t terminals;
    bool has_epsilon;
} LRFirstSet;

typedef struct {
    int production;
    int dot;
} LRItem;

typedef struct {
    LRItem *items;
    size_t count;
    size_t capacity;
} LRItemSet;

typedef struct {
    LRItemSet *states;
    int **transitions;
    size_t count;
    size_t capacity;
} LRAutomaton;

static void lr_itemset_init(LRItemSet *set) {
    set->items = NULL;
    set->count = 0;
    set->capacity = 0;
}

static void lr_itemset_free(LRItemSet *set) {
    if (!set) {
        return;
    }
    free(set->items);
    set->items = NULL;
    set->count = 0;
    set->capacity = 0;
}

static int lr_item_compare(const void *a, const void *b) {
    const LRItem *ia = (const LRItem*)a;
    const LRItem *ib = (const LRItem*)b;
    if (ia->production != ib->production) {
        return (ia->production < ib->production) ? -1 : 1;
    }
    if (ia->dot != ib->dot) {
        return (ia->dot < ib->dot) ? -1 : 1;
    }
    return 0;
}

static bool lr_itemset_contains(const LRItemSet *set, int production, int dot) {
    for (size_t i = 0; i < set->count; ++i) {
        if (set->items[i].production == production && set->items[i].dot == dot) {
            return true;
        }
    }
    return false;
}

static bool lr_itemset_add_item(LRItemSet *set, int production, int dot) {
    if (lr_itemset_contains(set, production, dot)) {
        return false;
    }
    if (set->count == set->capacity) {
        size_t new_cap = set->capacity ? set->capacity * 2 : 8;
        LRItem *new_items = (LRItem*)realloc(set->items, new_cap * sizeof(LRItem));
        if (!new_items) {
            return false;
        }
        set->items = new_items;
        set->capacity = new_cap;
    }
    set->items[set->count].production = production;
    set->items[set->count].dot = dot;
    set->count++;
    return true;
}

static void lr_itemset_sort(LRItemSet *set) {
    if (set->count > 1) {
        qsort(set->items, set->count, sizeof(LRItem), lr_item_compare);
    }
}

static bool lr_itemset_equal(const LRItemSet *a, const LRItemSet *b) {
    if (a->count != b->count) {
        return false;
    }
    for (size_t i = 0; i < a->count; ++i) {
        if (a->items[i].production != b->items[i].production ||
            a->items[i].dot != b->items[i].dot) {
            return false;
        }
    }
    return true;
}

static bool lr_itemset_clone(LRItemSet *dst, const LRItemSet *src) {
    lr_itemset_init(dst);
    if (src->count == 0) {
        return true;
    }
    dst->items = (LRItem*)malloc(src->count * sizeof(LRItem));
    if (!dst->items) {
        return false;
    }
    memcpy(dst->items, src->items, src->count * sizeof(LRItem));
    dst->count = src->count;
    dst->capacity = src->count;
    return true;
}

static bool lr_symbol_nullable(int symbol, const LRFirstSet *first_sets) {
    if (LR_SYMBOL_IS_TERMINAL(symbol)) {
        return false;
    }
    int nt = LR_SYMBOL_TO_NONTERM(symbol);
    return first_sets[nt].has_epsilon;
}

static uint64_t lr_first_terminals_of_symbol(int symbol, const LRFirstSet *first_sets) {
    if (LR_SYMBOL_IS_TERMINAL(symbol)) {
        return LR_TERMINAL_BIT(symbol);
    }
    int nt = LR_SYMBOL_TO_NONTERM(symbol);
    return first_sets[nt].terminals;
}

static void lr_compute_first_sets(LRFirstSet *first_sets) {
    for (size_t i = 0; i < LR_NONTERM_COUNT; ++i) {
        first_sets[i].terminals = 0;
        first_sets[i].has_epsilon = false;
    }

    bool changed;
    do {
        changed = false;
        for (size_t p = 0; p < LR_PRODUCTION_COUNT; ++p) {
            const LRProduction *prod = &g_lr_productions[p];
            int lhs_nt = LR_SYMBOL_TO_NONTERM(prod->lhs);
            bool nullable_prefix = true;
            if (prod->rhs_len == 0) {
                if (!first_sets[lhs_nt].has_epsilon) {
                    first_sets[lhs_nt].has_epsilon = true;
                    changed = true;
                }
                continue;
            }
            for (int i = 0; i < prod->rhs_len; ++i) {
                int symbol = prod->rhs[i];
                uint64_t before = first_sets[lhs_nt].terminals;
                first_sets[lhs_nt].terminals |= lr_first_terminals_of_symbol(symbol, first_sets);
                if (first_sets[lhs_nt].terminals != before) {
                    changed = true;
                }
                if (!lr_symbol_nullable(symbol, first_sets)) {
                    nullable_prefix = false;
                    break;
                }
            }
            if (nullable_prefix && !first_sets[lhs_nt].has_epsilon) {
                first_sets[lhs_nt].has_epsilon = true;
                changed = true;
            }
        }
    } while (changed);
}

static void lr_compute_follow_sets(const LRFirstSet *first_sets, uint64_t *follow_sets) {
    for (size_t i = 0; i < LR_NONTERM_COUNT; ++i) {
        follow_sets[i] = 0;
    }
    follow_sets[LR_NT_PROGRAMA] |= LR_TERMINAL_BIT(LR_TERM_EOF);

    bool changed;
    do {
        changed = false;
        for (size_t p = 0; p < LR_PRODUCTION_COUNT; ++p) {
            const LRProduction *prod = &g_lr_productions[p];
            int lhs_nt = LR_SYMBOL_TO_NONTERM(prod->lhs);
            uint64_t trailer = follow_sets[lhs_nt];
            for (int i = prod->rhs_len - 1; i >= 0; --i) {
                int symbol = prod->rhs[i];
                if (LR_SYMBOL_IS_NONTERM(symbol)) {
                    int nt = LR_SYMBOL_TO_NONTERM(symbol);
                    uint64_t before = follow_sets[nt];
                    follow_sets[nt] |= trailer;
                    if (follow_sets[nt] != before) {
                        changed = true;
                    }
                    uint64_t first_terminals = lr_first_terminals_of_symbol(symbol, first_sets);
                    if (lr_symbol_nullable(symbol, first_sets)) {
                        trailer |= first_terminals;
                    } else {
                        trailer = first_terminals;
                    }
                } else {
                    trailer = LR_TERMINAL_BIT(symbol);
                }
            }
        }
    } while (changed);
}

static void lr_automaton_init(LRAutomaton *aut) {
    aut->states = NULL;
    aut->transitions = NULL;
    aut->count = 0;
    aut->capacity = 0;
}

static void lr_automaton_free(LRAutomaton *aut) {
    if (!aut) {
        return;
    }
    for (size_t i = 0; i < aut->count; ++i) {
        lr_itemset_free(&aut->states[i]);
        free(aut->transitions[i]);
    }
    free(aut->states);
    free(aut->transitions);
    aut->states = NULL;
    aut->transitions = NULL;
    aut->count = 0;
    aut->capacity = 0;
}

static bool lr_automaton_ensure_capacity(LRAutomaton *aut, size_t min_capacity) {
    if (aut->capacity >= min_capacity) {
        return true;
    }
    size_t new_cap = aut->capacity ? aut->capacity * 2 : 8;
    while (new_cap < min_capacity) {
        new_cap *= 2;
    }
    LRItemSet *new_states = (LRItemSet*)realloc(aut->states, new_cap * sizeof(LRItemSet));
    if (!new_states) {
        return false;
    }
    int **new_transitions = (int**)realloc(aut->transitions, new_cap * sizeof(int*));
    if (!new_transitions) {
        free(new_states);
        return false;
    }
    for (size_t i = aut->capacity; i < new_cap; ++i) {
        lr_itemset_init(&new_states[i]);
        new_transitions[i] = NULL;
    }
    aut->states = new_states;
    aut->transitions = new_transitions;
    aut->capacity = new_cap;
    return true;
}

static bool lr_automaton_add_state(LRAutomaton *aut, const LRItemSet *set) {
    if (!lr_automaton_ensure_capacity(aut, aut->count + 1)) {
        return false;
    }
    if (!lr_itemset_clone(&aut->states[aut->count], set)) {
        return false;
    }
    aut->transitions[aut->count] = (int*)malloc(sizeof(int) * LR_SYMBOL_COUNT);
    if (!aut->transitions[aut->count]) {
        lr_itemset_free(&aut->states[aut->count]);
        return false;
    }
    for (size_t s = 0; s < LR_SYMBOL_COUNT; ++s) {
        aut->transitions[aut->count][s] = -1;
    }
    aut->count++;
    return true;
}

static int lr_automaton_find_state(const LRAutomaton *aut, const LRItemSet *set) {
    for (size_t i = 0; i < aut->count; ++i) {
        if (lr_itemset_equal(&aut->states[i], set)) {
            return (int)i;
        }
    }
    return -1;
}

static void lr_itemset_closure(LRItemSet *set) {
    bool changed;
    do {
        changed = false;
        for (size_t i = 0; i < set->count; ++i) {
            const LRItem *item = &set->items[i];
            const LRProduction *prod = &g_lr_productions[item->production];
            if (item->dot >= prod->rhs_len) {
                continue;
            }
            int symbol = prod->rhs[item->dot];
            if (LR_SYMBOL_IS_NONTERM(symbol)) {
                for (size_t p = 0; p < LR_PRODUCTION_COUNT; ++p) {
                    if (g_lr_productions[p].lhs == symbol) {
                        if (lr_itemset_add_item(set, (int)p, 0)) {
                            changed = true;
                        }
                    }
                }
            }
        }
    } while (changed);
}

static bool lr_itemset_goto(const LRItemSet *set, int symbol, LRItemSet *dest) {
    lr_itemset_init(dest);
    for (size_t i = 0; i < set->count; ++i) {
        const LRItem *item = &set->items[i];
        const LRProduction *prod = &g_lr_productions[item->production];
        if (item->dot < prod->rhs_len && prod->rhs[item->dot] == symbol) {
            lr_itemset_add_item(dest, item->production, item->dot + 1);
        }
    }
    if (dest->count == 0) {
        return false;
    }
    lr_itemset_closure(dest);
    lr_itemset_sort(dest);
    return true;
}

static bool lr_build_automaton(LRAutomaton *aut) {
    lr_automaton_init(aut);
    LRItemSet start;
    lr_itemset_init(&start);
    if (!lr_itemset_add_item(&start, 0, 0)) {
        lr_itemset_free(&start);
        return false;
    }
    lr_itemset_closure(&start);
    lr_itemset_sort(&start);
    if (!lr_automaton_add_state(aut, &start)) {
        lr_itemset_free(&start);
        return false;
    }
    lr_itemset_free(&start);

    for (size_t i = 0; i < aut->count; ++i) {
        for (int symbol = 0; symbol < LR_SYMBOL_COUNT; ++symbol) {
            LRItemSet goto_set;
            if (!lr_itemset_goto(&aut->states[i], symbol, &goto_set)) {
                continue;
            }
            int dest = lr_automaton_find_state(aut, &goto_set);
            if (dest < 0) {
                dest = (int)aut->count;
                if (!lr_automaton_add_state(aut, &goto_set)) {
                    lr_itemset_free(&goto_set);
                    lr_automaton_free(aut);
                    return false;
                }
            }
            aut->transitions[i][symbol] = dest;
            lr_itemset_free(&goto_set);
        }
    }
    return true;
}

static void lr_parse_table_free(LRParseTable *table) {
    if (!table) {
        return;
    }
    free(table->action);
    free(table->gotos);
    table->action = NULL;
    table->gotos = NULL;
    table->state_count = 0;
}

static bool lr_set_action(LRAction *row, size_t state, int term, LRActionType type, int value) {
    LRAction *entry = &row[state * LR_TERM_COUNT + term];
    if (entry->type != LR_ACTION_ERROR && entry->type != type) {
        fprintf(stderr, "[LR] Conflicto en estado %zu, terminal %s\n", state, lr_terminal_name(term));
        return false;
    }
    entry->type = type;
    entry->value = value;
    return true;
}

static bool lr_build_parse_table(LRParseTable *table) {
    LRFirstSet first_sets[LR_NONTERM_COUNT];
    uint64_t follow_sets[LR_NONTERM_COUNT];
    lr_compute_first_sets(first_sets);
    lr_compute_follow_sets(first_sets, follow_sets);

    LRAutomaton automaton;
    if (!lr_build_automaton(&automaton)) {
        return false;
    }

    table->state_count = automaton.count;
    size_t action_cells = automaton.count * LR_TERM_COUNT;
    size_t goto_cells = automaton.count * LR_NONTERM_COUNT;
    table->action = (LRAction*)malloc(action_cells * sizeof(LRAction));
    table->gotos = (int*)malloc(goto_cells * sizeof(int));
    if (!table->action || !table->gotos) {
        lr_parse_table_free(table);
        lr_automaton_free(&automaton);
        return false;
    }
    for (size_t i = 0; i < action_cells; ++i) {
        table->action[i].type = LR_ACTION_ERROR;
        table->action[i].value = -1;
    }
    for (size_t i = 0; i < goto_cells; ++i) {
        table->gotos[i] = -1;
    }

    bool ok = true;
    for (size_t state = 0; state < automaton.count; ++state) {
        for (int term = 0; term < LR_TERM_COUNT; ++term) {
            int dest = automaton.transitions[state][term];
            if (dest >= 0) {
                if (!lr_set_action(table->action, state, term, LR_ACTION_SHIFT, dest)) {
                    ok = false;
                }
            }
        }
        for (int nt = 0; nt < (int)LR_NONTERM_COUNT; ++nt) {
            int symbol = LR_SYMBOL_FROM_NONTERM(nt);
            int dest = automaton.transitions[state][symbol];
            if (dest >= 0) {
                table->gotos[state * LR_NONTERM_COUNT + nt] = dest; //tabla de goto nos dira a que estado ir
            }
        }

        const LRItemSet *set = &automaton.states[state];
        for (size_t i = 0; i < set->count; ++i) {
            const LRItem *item = &set->items[i];
            const LRProduction *prod = &g_lr_productions[item->production];
            if (item->dot == prod->rhs_len) {
                if (prod->lhs == LR_SYMBOL_FROM_NONTERM(LR_NT_START)) {
                    lr_set_action(table->action, state, LR_TERM_EOF, LR_ACTION_ACCEPT, 0);
                    continue;
                }
                int A = LR_SYMBOL_TO_NONTERM(prod->lhs);
                uint64_t follow = follow_sets[A];
                for (int term = 0; term < LR_TERM_COUNT; ++term) {
                    if (follow & LR_TERMINAL_BIT(term)) {
                        lr_set_action(table->action, state, term, LR_ACTION_REDUCE, (int)item->production);
                    }
                }
            }
        }
    }

    lr_automaton_free(&automaton);
    if (!ok) {
        lr_parse_table_free(table);
    }
    return ok;
}

static LRParseTable g_lr_table = {0};
static bool g_lr_table_ready = false;

const LRParseTable* lr_get_parse_table(void) {
    if (!g_lr_table_ready) {
        if (!lr_build_parse_table(&g_lr_table)) {
            fprintf(stderr, "Error: no se pudo construir la tabla LR.\n");
            return NULL;
        }
        g_lr_table_ready = true;
    }
    return &g_lr_table;
}
