/*
 * parser.y
 * Parser Bison (Yacc) para Mini-Leng (lenguaje reducido).
 * - Diseñado para integrarse con el lexer Flex en src/automatizado/lexer/lexer.l
 * - Manejo de errores sintácticos con recuperación básica.
 * - Acciones semánticas mínimas: revisión de tipos + mutabilidad de variables.
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* El lexer Flex expone estas variables globales. */
extern int yylineno;
extern int yycolumn;

int yylex(void);
static void yyerror(const char *s);

static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

typedef enum TypeTag {
    TYPE_UNKNOWN = 0,
    TYPE_I32,
    TYPE_F64,
    TYPE_BOOL,
    TYPE_ERROR
} TypeTag;

static int semantic_errors = 0;
static int inside_function = 0;
static int scope_depth = 0;

typedef struct Symbol {
    char *name;
    TypeTag type;
    int is_mutable;
    int depth;
    struct Symbol *next;
} Symbol;

static Symbol *symtab = NULL;

static void enter_scope(void) {
    scope_depth++;
}

static void exit_scope(void) {
    Symbol **pp = &symtab;
    while (*pp) {
        if ((*pp)->depth == scope_depth) {
            Symbol *dead = *pp;
            *pp = dead->next;
            free(dead->name);
            free(dead);
            continue;
        }
        pp = &(*pp)->next;
    }
    scope_depth--;
    if (scope_depth < 0) scope_depth = 0;
}

static Symbol *lookup_symbol(const char *name) {
    for (Symbol *s = symtab; s; s = s->next) {
        if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

static int declare_symbol(const char *name, TypeTag type, int is_mutable) {
    for (Symbol *s = symtab; s; s = s->next) {
        if (s->depth == scope_depth && strcmp(s->name, name) == 0) {
            return 0;
        }
    }

    Symbol *s = (Symbol *)calloc(1, sizeof(Symbol));
    if (!s) return 0;
    s->name = xstrdup(name);
    s->type = type;
    s->is_mutable = is_mutable;
    s->depth = scope_depth;
    s->next = symtab;
    symtab = s;
    return 1;
}

static void semantic_error(const char *msg) {
    semantic_errors++;
    fprintf(stderr, "Error semántico [%d:%d]: %s\n", yylineno, yycolumn, msg);
}

static TypeTag promote_numeric(TypeTag a, TypeTag b) {
    if (a == TYPE_ERROR || b == TYPE_ERROR) return TYPE_ERROR;
    if (a == TYPE_UNKNOWN || b == TYPE_UNKNOWN) return TYPE_UNKNOWN;
    if ((a != TYPE_I32 && a != TYPE_F64) || (b != TYPE_I32 && b != TYPE_F64)) return TYPE_ERROR;
    return (a == TYPE_F64 || b == TYPE_F64) ? TYPE_F64 : TYPE_I32;
}

static int is_compatible_assignment(TypeTag dst, TypeTag src) {
    if (dst == TYPE_ERROR || src == TYPE_ERROR) return 0;
    if (dst == TYPE_UNKNOWN || src == TYPE_UNKNOWN) return 1;
    if (dst == src) return 1;
    if (dst == TYPE_F64 && src == TYPE_I32) return 1; /* promoción */
    return 0;
}

static TypeTag type_of_number_lexeme(const char *lex) {
    if (!lex) return TYPE_ERROR;
    for (const char *p = lex; *p; ++p) {
        if (*p == '.' || *p == 'e' || *p == 'E') return TYPE_F64;
    }
    return TYPE_I32;
}
%}

%define parse.error verbose

%union {
    char *lexeme;
    int type;
    int flag;
}

/* Tokens deben coincidir con los que retorna src/automatizado/lexer/lexer.l */
%token <lexeme> TOKEN_IDENTIFIER TOKEN_NUMBER

%token TOKEN_KW_FN TOKEN_KW_LET TOKEN_KW_MUT TOKEN_KW_IF TOKEN_KW_ELSE TOKEN_KW_RETURN
%token TOKEN_KW_TRUE TOKEN_KW_FALSE
%token TOKEN_KW_I32 TOKEN_KW_F64 TOKEN_KW_BOOL

%token TOKEN_PLUS TOKEN_MINUS TOKEN_STAR TOKEN_SLASH TOKEN_PERCENT
%token TOKEN_EQUAL TOKEN_EQUAL_EQUAL
%token TOKEN_BANG TOKEN_BANG_EQUAL
%token TOKEN_LESS TOKEN_LESS_EQUAL TOKEN_GREATER TOKEN_GREATER_EQUAL
%token TOKEN_AND_AND TOKEN_OR_OR

%token TOKEN_SEMICOLON TOKEN_COMMA TOKEN_COLON
%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACE TOKEN_RBRACE
%token TOKEN_EOF TOKEN_UNKNOWN

%type <flag> mut_opt
%type <type> type type_opt init_opt expr_opt
%type <type> expr assignment log_or log_and equality rel sum mult unary primary

%right TOKEN_EQUAL
%left TOKEN_OR_OR
%left TOKEN_AND_AND
%nonassoc TOKEN_EQUAL_EQUAL TOKEN_BANG_EQUAL
%nonassoc TOKEN_LESS TOKEN_LESS_EQUAL TOKEN_GREATER TOKEN_GREATER_EQUAL
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_STAR TOKEN_SLASH TOKEN_PERCENT
%right TOKEN_BANG
%right UPLUS UMINUS

%destructor { free($$); } <lexeme>

%%

program
    : function_list TOKEN_EOF
      {
          if (semantic_errors > 0) {
              YYABORT;
          }
      }
    ;

function_list
    : /* empty */
    | function_list function
    ;

function
    : TOKEN_KW_FN TOKEN_IDENTIFIER TOKEN_LPAREN
      {
          inside_function = 1;
          enter_scope(); /* scope de parámetros */
      }
      params_opt TOKEN_RPAREN block
      {
          exit_scope();
          inside_function = 0;
      }
    ;

params_opt
    : /* empty */
    | params
    ;

params
    : param
    | params TOKEN_COMMA param
    ;

param
    : TOKEN_IDENTIFIER TOKEN_COLON type
      {
          if (!declare_symbol($1, (TypeTag)$3, 0)) {
              semantic_error("parámetro redeclarado en el mismo ámbito");
          }
      }
    ;

block
    : TOKEN_LBRACE { enter_scope(); } stmt_list TOKEN_RBRACE { exit_scope(); }
    | TOKEN_LBRACE { enter_scope(); } error TOKEN_RBRACE { exit_scope(); yyerrok; }
    ;

stmt_list
    : /* empty */
    | stmt_list stmt
    ;

stmt
    : let_stmt TOKEN_SEMICOLON
    | return_stmt TOKEN_SEMICOLON
    | if_stmt
    | expr TOKEN_SEMICOLON
    | error TOKEN_SEMICOLON { yyerrok; }
    ;

let_stmt
    : TOKEN_KW_LET mut_opt TOKEN_IDENTIFIER type_opt init_opt
      {
          TypeTag annotated = (TypeTag)$4;
          TypeTag init_t = (TypeTag)$5;
          TypeTag final_t = annotated;

          if (final_t == TYPE_UNKNOWN) {
              final_t = init_t;
          }

          if (final_t == TYPE_UNKNOWN) {
              semantic_error("declaración let sin tipo ni inicializador (no se puede inferir)");
              final_t = TYPE_ERROR;
          }

          if (annotated != TYPE_UNKNOWN && init_t != TYPE_UNKNOWN && !is_compatible_assignment(annotated, init_t)) {
              semantic_error("tipo del inicializador no es compatible con el tipo anotado");
          }

          if (!declare_symbol($3, final_t, $2)) {
              semantic_error("variable redeclarada en el mismo ámbito");
          }
      }
    ;

mut_opt
    : TOKEN_KW_MUT { $$ = 1; }
    | /* empty */  { $$ = 0; }
    ;

type_opt
    : TOKEN_COLON type { $$ = $2; }
    | /* empty */      { $$ = TYPE_UNKNOWN; }
    ;

init_opt
    : TOKEN_EQUAL expr { $$ = $2; }
    | /* empty */      { $$ = TYPE_UNKNOWN; }
    ;

return_stmt
    : TOKEN_KW_RETURN expr_opt
      {
          if (!inside_function) {
              semantic_error("'return' fuera de una función");
          }
          (void)$2;
      }
    ;

expr_opt
    : expr          { $$ = $1; }
    | /* empty */   { $$ = TYPE_UNKNOWN; }
    ;

if_stmt
    : TOKEN_KW_IF expr block else_opt
      {
          TypeTag cond_t = (TypeTag)$2;
          if (cond_t != TYPE_BOOL && cond_t != TYPE_UNKNOWN && cond_t != TYPE_ERROR) {
              semantic_error("condición de if debe ser bool");
          }
      }
    ;

else_opt
    : TOKEN_KW_ELSE block
    | /* empty */
    ;

expr
    : assignment { $$ = $1; }
    ;

assignment
    : TOKEN_IDENTIFIER TOKEN_EQUAL assignment
      {
          Symbol *s = lookup_symbol($1);
          TypeTag rhs = (TypeTag)$3;

          if (!s) {
              semantic_error("asignación a variable no declarada");
              $$ = TYPE_ERROR;
          } else {
              if (!s->is_mutable) {
                  semantic_error("asignación a variable no mutable (falta 'mut')");
              }
              if (!is_compatible_assignment(s->type, rhs)) {
                  semantic_error("tipo incompatible en asignación");
              }
              $$ = s->type == TYPE_UNKNOWN ? rhs : s->type;
          }
      }
    | log_or { $$ = $1; }
    ;

log_or
    : log_and { $$ = $1; }
    | log_or TOKEN_OR_OR log_and
      {
          TypeTag a = (TypeTag)$1;
          TypeTag b = (TypeTag)$3;
          if ((a != TYPE_BOOL && a != TYPE_UNKNOWN && a != TYPE_ERROR) || (b != TYPE_BOOL && b != TYPE_UNKNOWN && b != TYPE_ERROR)) {
              semantic_error("'||' requiere operandos bool");
              $$ = TYPE_ERROR;
          } else {
              $$ = (a == TYPE_ERROR || b == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
          }
      }
    ;

log_and
    : equality { $$ = $1; }
    | log_and TOKEN_AND_AND equality
      {
          TypeTag a = (TypeTag)$1;
          TypeTag b = (TypeTag)$3;
          if ((a != TYPE_BOOL && a != TYPE_UNKNOWN && a != TYPE_ERROR) || (b != TYPE_BOOL && b != TYPE_UNKNOWN && b != TYPE_ERROR)) {
              semantic_error("'&&' requiere operandos bool");
              $$ = TYPE_ERROR;
          } else {
              $$ = (a == TYPE_ERROR || b == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
          }
      }
    ;

equality
    : rel { $$ = $1; }
    | equality TOKEN_EQUAL_EQUAL rel
      {
          TypeTag a = (TypeTag)$1;
          TypeTag b = (TypeTag)$3;
          if (a != TYPE_UNKNOWN && b != TYPE_UNKNOWN && a != TYPE_ERROR && b != TYPE_ERROR) {
              if (!is_compatible_assignment(a, b) && !is_compatible_assignment(b, a)) {
                  semantic_error("'==' requiere operandos de tipos compatibles");
                  $$ = TYPE_ERROR;
              } else {
                  $$ = TYPE_BOOL;
              }
          } else {
              $$ = (a == TYPE_ERROR || b == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
          }
      }
    | equality TOKEN_BANG_EQUAL rel
      {
          TypeTag a = (TypeTag)$1;
          TypeTag b = (TypeTag)$3;
          if (a != TYPE_UNKNOWN && b != TYPE_UNKNOWN && a != TYPE_ERROR && b != TYPE_ERROR) {
              if (!is_compatible_assignment(a, b) && !is_compatible_assignment(b, a)) {
                  semantic_error("'!=' requiere operandos de tipos compatibles");
                  $$ = TYPE_ERROR;
              } else {
                  $$ = TYPE_BOOL;
              }
          } else {
              $$ = (a == TYPE_ERROR || b == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
          }
      }
    ;

rel
    : sum { $$ = $1; }
    | rel TOKEN_LESS sum
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'<' requiere operandos numéricos");
          $$ = (r == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
      }
    | rel TOKEN_LESS_EQUAL sum
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'<=' requiere operandos numéricos");
          $$ = (r == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
      }
    | rel TOKEN_GREATER sum
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'>' requiere operandos numéricos");
          $$ = (r == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
      }
    | rel TOKEN_GREATER_EQUAL sum
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'>=' requiere operandos numéricos");
          $$ = (r == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
      }
    ;

sum
    : mult { $$ = $1; }
    | sum TOKEN_PLUS mult
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'+' requiere operandos numéricos");
          $$ = r;
      }
    | sum TOKEN_MINUS mult
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'-' requiere operandos numéricos");
          $$ = r;
      }
    ;

mult
    : unary { $$ = $1; }
    | mult TOKEN_STAR unary
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'*' requiere operandos numéricos");
          $$ = r;
      }
    | mult TOKEN_SLASH unary
      {
          TypeTag r = promote_numeric((TypeTag)$1, (TypeTag)$3);
          if (r == TYPE_ERROR) semantic_error("'/' requiere operandos numéricos");
          $$ = r;
      }
    | mult TOKEN_PERCENT unary
      {
          TypeTag a = (TypeTag)$1;
          TypeTag b = (TypeTag)$3;
          if ((a != TYPE_I32 && a != TYPE_UNKNOWN && a != TYPE_ERROR) || (b != TYPE_I32 && b != TYPE_UNKNOWN && b != TYPE_ERROR)) {
              semantic_error("'%' requiere operandos i32");
              $$ = TYPE_ERROR;
          } else {
              $$ = (a == TYPE_ERROR || b == TYPE_ERROR) ? TYPE_ERROR : TYPE_I32;
          }
      }
    ;

unary
    : TOKEN_BANG unary
      {
          TypeTag t = (TypeTag)$2;
          if (t != TYPE_BOOL && t != TYPE_UNKNOWN && t != TYPE_ERROR) {
              semantic_error("'!' requiere operando bool");
              $$ = TYPE_ERROR;
          } else {
              $$ = (t == TYPE_ERROR) ? TYPE_ERROR : TYPE_BOOL;
          }
      }
    | TOKEN_PLUS unary %prec UPLUS
      {
          TypeTag t = (TypeTag)$2;
          if (t != TYPE_I32 && t != TYPE_F64 && t != TYPE_UNKNOWN && t != TYPE_ERROR) {
              semantic_error("'+' unario requiere operando numérico");
              $$ = TYPE_ERROR;
          } else {
              $$ = t;
          }
      }
    | TOKEN_MINUS unary %prec UMINUS
      {
          TypeTag t = (TypeTag)$2;
          if (t != TYPE_I32 && t != TYPE_F64 && t != TYPE_UNKNOWN && t != TYPE_ERROR) {
              semantic_error("'-' unario requiere operando numérico");
              $$ = TYPE_ERROR;
          } else {
              $$ = t;
          }
      }
    | primary { $$ = $1; }
    ;

primary
    : TOKEN_NUMBER
      {
          $$ = type_of_number_lexeme($1);
      }
    | TOKEN_KW_TRUE  { $$ = TYPE_BOOL; }
    | TOKEN_KW_FALSE { $$ = TYPE_BOOL; }
    | TOKEN_IDENTIFIER
      {
          Symbol *s = lookup_symbol($1);
          if (!s) {
              semantic_error("uso de identificador no declarado");
              $$ = TYPE_ERROR;
          } else {
              $$ = s->type;
          }
      }
    | TOKEN_IDENTIFIER TOKEN_LPAREN args_opt TOKEN_RPAREN
      {
          $$ = TYPE_UNKNOWN; /* tipos de función no especificados */
      }
    | TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
    ;

args_opt
    : /* empty */
    | args
    ;

args
    : expr
    | args TOKEN_COMMA expr
    ;

type
    : TOKEN_KW_I32  { $$ = TYPE_I32; }
    | TOKEN_KW_F64  { $$ = TYPE_F64; }
    | TOKEN_KW_BOOL { $$ = TYPE_BOOL; }
    ;

%%

static void yyerror(const char *s) {
    fprintf(stderr, "Error sintáctico [%d:%d]: %s\n", yylineno, yycolumn, s);
}
