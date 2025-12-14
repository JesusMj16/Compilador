/**
 * @file parser.h
 * @brief Parser LR ascendente para el compilador
 *
 * El parser ejecuta un análisis LR automático (tablas acción/goto) sobre la
 * gramática del lenguaje y, a partir de las reducciones, construye el AST y
 * actualiza la tabla de símbolos.
 */

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "symbol_table.h"
#include <stddef.h>
#include <stdbool.h>

typedef enum ASTNodeType {
    // Nodos estructurales
    AST_PROGRAM,            // Programa completo
    AST_FUNCTION,           // Función
    AST_PARAMETER,          // Parámetro de función
    AST_BLOCK,              // Bloque { ... }
    
    // Sentencias
    AST_LET_STMT,           // Sentencia let
    AST_EXPR_STMT,          // Expresión como sentencia
    AST_RETURN_STMT,        // Sentencia return
    AST_IF_STMT,            // Sentencia if/else
    
    // Expresiones binarias
    AST_BINARY_EXPR,        // Expresión binaria (op left right)
    AST_UNARY_EXPR,         // Expresión unaria (op operand)
    AST_CALL_EXPR,          // Llamada a función
    AST_ASSIGN_EXPR,        // Asignación
    
    // Literales y primarios
    AST_IDENTIFIER,         // Identificador
    AST_NUMBER,             // Número
    AST_BOOL,               // Booleano (true/false)
    
    // Tipos
    AST_TYPE                // Anotación de tipo
} ASTNodeType;

typedef enum BinaryOp {
    // Aritméticos
    OP_ADD,      // +
    OP_SUB,      // -
    OP_MUL,      // *
    OP_DIV,      // /
    OP_MOD,      // %
    
    // Comparación
    OP_EQ,       // ==
    OP_NEQ,      // !=
    OP_LT,       // <
    OP_LE,       // <=
    OP_GT,       // >
    OP_GE,       // >=
    
    // Lógicos
    OP_AND,      // &&
    OP_OR,       // ||
    
    // Asignación
    OP_ASSIGN    // =
} BinaryOp;

typedef enum UnaryOp {
    OP_NOT,      // !
    OP_NEG,      // - (negación)
    OP_PLUS      // + (positivo)
} UnaryOp;

typedef struct ASTNode {
    ASTNodeType type;
    size_t line;
    size_t column;
    
    // Datos específicos del nodo (union para ahorrar memoria)
    union {
        // Para listas (program, block, parameters, arguments)
        struct {
            struct ASTNode **children;
            size_t child_count;
            size_t capacity;
        } list;
        
        // Para función
        struct {
            char *name;
            struct ASTNode *parameters;  // Lista de parámetros
            struct ASTNode *body;        // Bloque
        } function;
        
        // Para parámetro
        struct {
            char *name;
            char *type;
        } parameter;
        
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
            struct ASTNode *arguments;  // Lista de argumentos
        } call;
        
        // Para literales (identificadores, números, booleanos)
        struct {
            char *value;
        } literal;
        
        // Para tipos
        struct {
            char *type_name;
        } type_info;
    } data;
} ASTNode;

typedef struct Parser {
    Lexer *lexer;                 // Lexer asociado 
    SymbolTable *symbol_table;    // Tabla de símbolos
    const char *source_text;      // Código fuente completo

    // Información de error
    bool has_error;
    char error_msg[512];
    size_t error_line;
    size_t error_col;
    int error_count;

    // Estadísticas
    int lines_compiled;
} Parser;

ASTNode* ast_create_node(ASTNodeType type, size_t line, size_t col);

ASTNode* ast_create_list(ASTNodeType type, size_t line, size_t col);

bool ast_add_child(ASTNode *parent, ASTNode *child);

ASTNode* ast_create_binary(BinaryOp op, ASTNode *left, ASTNode *right, size_t line, size_t col);

ASTNode* ast_create_unary(UnaryOp op, ASTNode *operand, size_t line, size_t col);

ASTNode* ast_create_literal(ASTNodeType type, const char *value, size_t line, size_t col);

ASTNode* ast_create_function(const char *name, ASTNode *params, ASTNode *body, size_t line, size_t col);

ASTNode* ast_create_parameter(const char *name, const char *type, size_t line, size_t col);

ASTNode* ast_create_let(const char *name, bool is_mut, const char *type, ASTNode *init, size_t line, size_t col);

ASTNode* ast_create_return(ASTNode *value, size_t line, size_t col);
ASTNode* ast_create_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, size_t line, size_t col);

ASTNode* ast_create_call(ASTNode *callee, ASTNode *arguments, size_t line, size_t col);

void ast_free(ASTNode *node);

void ast_print(const ASTNode *node, int indent);

const char* ast_node_type_name(ASTNodeType type);

const char* binary_op_name(BinaryOp op);

const char* unary_op_name(UnaryOp op);

bool parser_init(Parser *parser, Lexer *lexer, SymbolTable *symbol_table);

void parser_free(Parser *parser);

ASTNode* parser_parse(Parser *parser);

void parser_print_errors(const Parser *parser);

void parser_print_stats(const Parser *parser);

int parser_get_error_count(const Parser *parser);

int parser_get_lines_compiled(const Parser *parser);

#endif // PARSER_H
