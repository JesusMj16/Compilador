/**
 * @file parser.h
 * @brief Parser Ascendente LR para el compilador
 * 
 * Este archivo define un parser ascendente (bottom-up) tipo LR que utiliza:
 * 1. Una PILA para mantener el estado del análisis
 * 2. Un ÁRBOL DE SINTAXIS ABSTRACTA (AST) como resultado
 * 3. TABLAS DE TRANSICIÓN (ACTION y GOTO) para las decisiones de parsing
 * 
 * El parser lee tokens del lexer y construye un AST mediante operaciones
 * de shift (desplazamiento) y reduce (reducción).
 */

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * DEFINICIONES DE SÍMBOLOS Y PRODUCCIONES
 * ============================================================================ */

/**
 * @brief Símbolos no terminales de la gramática
 */
typedef enum NonTerminal {
    NT_PROGRAMA,              // Programa
    NT_LISTA_ITEMS,          // ListaItems
    NT_ITEM,                 // Item
    NT_FUNCION,              // Funcion
    NT_BLOQUE,               // Bloque
    NT_LISTA_SENTENCIAS,     // ListaSentencias
    NT_SENTENCIA,            // Sentencia
    NT_LET_SENTENCIA,        // LetSentencia
    NT_EXPR_SENTENCIA,       // ExprSentencia
    NT_IF_SENTENCIA,         // IfSentencia
    NT_WHILE_SENTENCIA,      // WhileSentencia
    NT_RETURN_SENTENCIA,     // ReturnSentencia
    NT_EXPRESION,            // Expresion
    NT_ASIGNACION,           // Asignacion
    NT_LOGICO_OR,            // LogicoOR
    NT_LOGICO_AND,           // LogicoAND
    NT_IGUALDAD,             // Igualdad
    NT_COMPARACION,          // Comparacion
    NT_TERM,                 // Term
    NT_FACTOR,               // Factor
    NT_UNARIO,               // Unario
    NT_POSTFIJO,             // Postfijo
    NT_PRIMARIO,             // Primario
    NT_LITERAL,              // Literal
    NT_TIPO,                 // Tipo
    NT_COUNT                 // Número de no terminales
} NonTerminal;

/**
 * @brief Tipo de acción en la tabla ACTION
 */
typedef enum ActionType {
    ACTION_SHIFT,    // Desplazar token a la pila
    ACTION_REDUCE,   // Reducir según una producción
    ACTION_ACCEPT,   // Aceptar entrada
    ACTION_ERROR     // Error sintáctico
} ActionType;

/**
 * @brief Entrada en la tabla ACTION
 */
typedef struct {
    ActionType type;
    int value;  // Estado para SHIFT, producción para REDUCE
} Action;

/**
 * @brief Estructura para representar una producción de la gramática
 */
typedef struct {
    NonTerminal left;      // Lado izquierdo (no terminal)
    int right_size;        // Cantidad de símbolos en el lado derecho
    const char *name;      // Nombre descriptivo de la producción
} Production;

/* ============================================================================
 * ÁRBOL DE SINTAXIS ABSTRACTA (AST)
 * ============================================================================ */

/**
 * @brief Tipos de nodos del AST
 */
typedef enum ASTNodeType {
    // Nodos estructurales
    AST_PROGRAM,
    AST_FUNCTION,
    AST_BLOCK,
    AST_STATEMENT_LIST,
    
    // Sentencias
    AST_LET_STMT,
    AST_EXPR_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_RETURN_STMT,
    AST_BREAK_STMT,
    AST_CONTINUE_STMT,
    
    // Expresiones
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_CALL_EXPR,
    AST_ASSIGN_EXPR,
    
    // Primarios
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_STRING,
    AST_CHAR,
    AST_BOOL,
    AST_ARRAY
} ASTNodeType;

/**
 * @brief Tipos de operadores binarios
 */
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_EQ, OP_NEQ, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_AND, OP_OR,
    OP_ASSIGN, OP_ADD_ASSIGN, OP_SUB_ASSIGN, OP_MUL_ASSIGN, OP_DIV_ASSIGN, OP_MOD_ASSIGN
} BinaryOp;

/**
 * @brief Tipos de operadores unarios
 */
typedef enum {
    OP_NOT, OP_NEG, OP_PLUS
} UnaryOp;

/**
 * @brief Nodo del árbol de sintaxis abstracta
 */
typedef struct ASTNode {
    ASTNodeType type;
    size_t line;
    size_t column;
    
    // Datos específicos del nodo
    union {
        // Para programa y listas
        struct {
            struct ASTNode **children;
            size_t child_count;
            size_t capacity;
        } list;
        
        // Para función
        struct {
            char *name;
            struct ASTNode *body;
        } function;
        
        // Para sentencia let
        struct {
            char *name;
            bool is_mutable;
            char *type;
            struct ASTNode *initializer;
        } let_stmt;
        
        // Para sentencia if
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_stmt;
        
        // Para sentencia while
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;
        
        // Para sentencia return
        struct {
            struct ASTNode *value;
        } return_stmt;
        
        // Para expresiones binarias
        struct {
            BinaryOp op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;
        
        // Para expresiones unarias
        struct {
            UnaryOp op;
            struct ASTNode *operand;
        } unary;
        
        // Para llamadas
        struct {
            struct ASTNode *callee;
            struct ASTNode **args;
            size_t arg_count;
        } call;
        
        // Para identificadores y literales
        struct {
            char *value;
        } literal;
    } data;
} ASTNode;

/* ============================================================================
 * PILA DEL PARSER
 * ============================================================================ */

/**
 * @brief Elemento de la pila del parser
 */
typedef struct StackElement {
    int state;              // Estado LR
    ASTNode *node;          // Nodo del AST asociado
    TokenType token_type;   // Tipo de token (si es terminal)
} StackElement;

/**
 * @brief Pila del parser
 */
typedef struct ParserStack {
    StackElement *elements;
    size_t size;
    size_t capacity;
} ParserStack;

/* ============================================================================
 * PARSER PRINCIPAL
 * ============================================================================ */

/**
 * @brief Estructura principal del parser LR
 */
typedef struct Parser {
    Lexer *lexer;              // Lexer para obtener tokens
    token_t *current_token;    // Token actual
    ParserStack stack;         // Pila del parser
    
    // Tablas de parsing
    const Action **action_table;    // Tabla ACTION[estado][token]
    const int **goto_table;         // Tabla GOTO[estado][no_terminal]
    
    // Información de error
    bool has_error;
    char error_msg[256];
    size_t error_line;
    size_t error_col;
    
    // Estadísticas
    int shift_count;
    int reduce_count;
} Parser;

/* ============================================================================
 * FUNCIONES DE LA PILA
 * ============================================================================ */

/**
 * @brief Inicializa la pila del parser
 */
bool stack_init(ParserStack *stack);

/**
 * @brief Libera la memoria de la pila
 */
void stack_free(ParserStack *stack);

/**
 * @brief Inserta un elemento en la pila
 */
bool stack_push(ParserStack *stack, int state, ASTNode *node, TokenType token_type);

/**
 * @brief Elimina y retorna el elemento del tope de la pila
 */
StackElement stack_pop(ParserStack *stack);

/**
 * @brief Obtiene el elemento del tope sin eliminarlo
 */
StackElement stack_peek(const ParserStack *stack);

/**
 * @brief Obtiene el estado del tope de la pila
 */
int stack_top_state(const ParserStack *stack);

/**
 * @brief Obtiene el tamaño de la pila
 */
size_t stack_size(const ParserStack *stack);

/**
 * @brief Imprime el contenido de la pila (debug)
 */
void stack_print(const ParserStack *stack);

/* ============================================================================
 * FUNCIONES DEL AST
 * ============================================================================ */

/**
 * @brief Crea un nodo del AST
 */
ASTNode* ast_create_node(ASTNodeType type, size_t line, size_t col);

/**
 * @brief Crea un nodo de lista (programa, bloque, etc.)
 */
ASTNode* ast_create_list(ASTNodeType type, size_t line, size_t col);

/**
 * @brief Agrega un hijo a un nodo de lista
 */
bool ast_add_child(ASTNode *parent, ASTNode *child);

/**
 * @brief Crea un nodo de expresión binaria
 */
ASTNode* ast_create_binary(BinaryOp op, ASTNode *left, ASTNode *right, size_t line, size_t col);

/**
 * @brief Crea un nodo de expresión unaria
 */
ASTNode* ast_create_unary(UnaryOp op, ASTNode *operand, size_t line, size_t col);

/**
 * @brief Crea un nodo literal
 */
ASTNode* ast_create_literal(ASTNodeType type, const char *value, size_t line, size_t col);

/**
 * @brief Crea un nodo de función
 */
ASTNode* ast_create_function(const char *name, ASTNode *body, size_t line, size_t col);

/**
 * @brief Crea un nodo de sentencia let
 */
ASTNode* ast_create_let(const char *name, bool is_mut, const char *type, ASTNode *init, size_t line, size_t col);

/**
 * @brief Crea un nodo de sentencia if
 */
ASTNode* ast_create_if(ASTNode *cond, ASTNode *then_br, ASTNode *else_br, size_t line, size_t col);

/**
 * @brief Crea un nodo de sentencia while
 */
ASTNode* ast_create_while(ASTNode *cond, ASTNode *body, size_t line, size_t col);

/**
 * @brief Crea un nodo de sentencia return
 */
ASTNode* ast_create_return(ASTNode *value, size_t line, size_t col);

/**
 * @brief Libera un nodo del AST y todos sus hijos
 */
void ast_free(ASTNode *node);

/**
 * @brief Imprime el AST de forma jerárquica
 */
void ast_print(const ASTNode *node, int indent);

/**
 * @brief Obtiene el nombre de un tipo de nodo
 */
const char* ast_node_type_name(ASTNodeType type);

/* ============================================================================
 * FUNCIONES DEL PARSER
 * ============================================================================ */

/**
 * @brief Inicializa el parser
 */
bool parser_init(Parser *parser, Lexer *lexer);

/**
 * @brief Libera la memoria del parser
 */
void parser_free(Parser *parser);

/**
 * @brief Ejecuta el análisis sintáctico
 * @return El nodo raíz del AST si tiene éxito, NULL en caso de error
 */
ASTNode* parser_parse(Parser *parser);

/**
 * @brief Imprime un error del parser
 */
void parser_print_error(const Parser *parser);

/**
 * @brief Imprime las estadísticas del parsing
 */
void parser_print_stats(const Parser *parser);

/**
 * @brief Obtiene el nombre de un no terminal
 */
const char* non_terminal_name(NonTerminal nt);

/**
 * @brief Obtiene el nombre de una acción
 */
const char* action_type_name(ActionType type);

#endif // PARSER_H
