/**
 * @file parser.c
 * @brief Implementación del Parser Descendente Recursivo
 * 
 * Parser simple que implementa la gramática reducida:
 * - Funciones con parámetros
 * - Sentencias: let, return, expresiones
 * - Expresiones con precedencia completa
 * - NO incluye: if, while, for, match, arreglos
 */

#define _POSIX_C_SOURCE 200809L

#include "../../include/parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef strdup
extern char *strdup(const char *s);
#endif

/* ============================================================================
 * FUNCIONES AUXILIARES DEL PARSER
 * ============================================================================ */

/**
 * @brief Avanza al siguiente token
 */
static void parser_advance(Parser *parser) {
    if (parser->previous_token) {
        free_token(parser->previous_token);
    }
    parser->previous_token = parser->current_token;
    parser->current_token = lexer_next_token(parser->lexer);
    
    if (parser->current_token && parser->current_token->line > (size_t)parser->lines_compiled) {
        parser->lines_compiled = (int)parser->current_token->line;
    }
}

/**
 * @brief Verifica si el token actual es del tipo esperado
 */
static bool parser_check(Parser *parser, TokenType type) {
    return parser->current_token && parser->current_token->type == type;
}

/**
 * @brief Consume un token si es del tipo esperado
 */
static bool parser_match(Parser *parser, TokenType type) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    return false;
}

/**
 * @brief Espera un token específico o reporta error
 */
static bool parser_expect(Parser *parser, TokenType type, const char *message) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    
    parser->has_error = true;
    parser->error_count++;
    snprintf(parser->error_msg, sizeof(parser->error_msg), "%s", message);
    if (parser->current_token) {
        parser->error_line = parser->current_token->line;
        parser->error_col = parser->current_token->column;
    }
    
    return false;
}

/**
 * @brief Reporta un error
 */
static void parser_error(Parser *parser, const char *message) {
    parser->has_error = true;
    parser->error_count++;
    snprintf(parser->error_msg, sizeof(parser->error_msg), "%s", message);
    if (parser->current_token) {
        parser->error_line = parser->current_token->line;
        parser->error_col = parser->current_token->column;
    }
}

/* ============================================================================
 * IMPLEMENTACIÓN DEL AST
 * ============================================================================ */

ASTNode* ast_create_node(ASTNodeType type, size_t line, size_t col) {
    ASTNode *node = (ASTNode*)calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    node->line = line;
    node->column = col;
    
    return node;
}

ASTNode* ast_create_list(ASTNodeType type, size_t line, size_t col) {
    ASTNode *node = ast_create_node(type, line, col);
    if (!node) return NULL;
    
    node->data.list.children = (ASTNode**)malloc(sizeof(ASTNode*) * 8);
    node->data.list.child_count = 0;
    node->data.list.capacity = 8;
    
    return node;
}

bool ast_add_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return false;
    
    if (parent->data.list.child_count >= parent->data.list.capacity) {
        size_t new_cap = parent->data.list.capacity * 2;
        ASTNode **new_children = (ASTNode**)realloc(
            parent->data.list.children,
            sizeof(ASTNode*) * new_cap
        );
        if (!new_children) return false;
        
        parent->data.list.children = new_children;
        parent->data.list.capacity = new_cap;
    }
    
    parent->data.list.children[parent->data.list.child_count++] = child;
    return true;
}

ASTNode* ast_create_binary(BinaryOp op, ASTNode *left, ASTNode *right, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_BINARY_EXPR, line, col);
    if (!node) return NULL;
    
    node->data.binary.op = op;
    node->data.binary.left = left;
    node->data.binary.right = right;
    
    return node;
}

ASTNode* ast_create_unary(UnaryOp op, ASTNode *operand, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_UNARY_EXPR, line, col);
    if (!node) return NULL;
    
    node->data.unary.op = op;
    node->data.unary.operand = operand;
    
    return node;
}

ASTNode* ast_create_literal(ASTNodeType type, const char *value, size_t line, size_t col) {
    ASTNode *node = ast_create_node(type, line, col);
    if (!node) return NULL;
    
    if (value) {
        node->data.literal.value = strdup(value);
    }
    
    return node;
}

ASTNode* ast_create_function(const char *name, ASTNode *params, ASTNode *body, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_FUNCTION, line, col);
    if (!node) return NULL;
    
    if (name) {
        node->data.function.name = strdup(name);
    }
    node->data.function.parameters = params;
    node->data.function.body = body;
    
    return node;
}

ASTNode* ast_create_parameter(const char *name, const char *type, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_PARAMETER, line, col);
    if (!node) return NULL;
    
    if (name) {
        node->data.parameter.name = strdup(name);
    }
    if (type) {
        node->data.parameter.type = strdup(type);
    }
    
    return node;
}

ASTNode* ast_create_let(const char *name, bool is_mut, const char *type, ASTNode *init, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_LET_STMT, line, col);
    if (!node) return NULL;
    
    if (name) {
        node->data.let_stmt.name = strdup(name);
    }
    node->data.let_stmt.is_mutable = is_mut;
    if (type) {
        node->data.let_stmt.type = strdup(type);
    }
    node->data.let_stmt.initializer = init;
    
    return node;
}

ASTNode* ast_create_return(ASTNode *value, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_RETURN_STMT, line, col);
    if (!node) return NULL;
    
    node->data.return_stmt.value = value;
    
    return node;
}

ASTNode* ast_create_call(ASTNode *callee, ASTNode *arguments, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_CALL_EXPR, line, col);
    if (!node) return NULL;
    
    node->data.call.callee = callee;
    node->data.call.arguments = arguments;
    
    return node;
}

void ast_free(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            for (size_t i = 0; i < node->data.list.child_count; i++) {
                ast_free(node->data.list.children[i]);
            }
            free(node->data.list.children);
            break;
            
        case AST_FUNCTION:
            free(node->data.function.name);
            ast_free(node->data.function.parameters);
            ast_free(node->data.function.body);
            break;
            
        case AST_PARAMETER:
            free(node->data.parameter.name);
            free(node->data.parameter.type);
            break;
            
        case AST_LET_STMT:
            free(node->data.let_stmt.name);
            free(node->data.let_stmt.type);
            ast_free(node->data.let_stmt.initializer);
            break;
            
        case AST_RETURN_STMT:
            ast_free(node->data.return_stmt.value);
            break;
            
        case AST_BINARY_EXPR:
            ast_free(node->data.binary.left);
            ast_free(node->data.binary.right);
            break;
            
        case AST_UNARY_EXPR:
            ast_free(node->data.unary.operand);
            break;
            
        case AST_CALL_EXPR:
            ast_free(node->data.call.callee);
            ast_free(node->data.call.arguments);
            break;
            
        case AST_IDENTIFIER:
        case AST_NUMBER:
        case AST_BOOL:
            free(node->data.literal.value);
            break;
            
        default:
            break;
    }
    
    free(node);
}

/* ============================================================================
 * FUNCIONES DE NOMBRES
 * ============================================================================ */

const char* ast_node_type_name(ASTNodeType type) {
    static const char *names[] = {
        "Program", "Function", "Parameter", "Block",
        "LetStmt", "ExprStmt", "ReturnStmt",
        "BinaryExpr", "UnaryExpr", "CallExpr", "AssignExpr",
        "Identifier", "Number", "Bool", "Type"
    };
    
    if (type >= 0 && type < (sizeof(names) / sizeof(names[0]))) {
        return names[type];
    }
    return "Unknown";
}

const char* binary_op_name(BinaryOp op) {
    static const char *names[] = {
        "+", "-", "*", "/", "%",
        "==", "!=", "<", "<=", ">", ">=",
        "&&", "||", "="
    };
    
    if (op >= 0 && op < (sizeof(names) / sizeof(names[0]))) {
        return names[op];
    }
    return "?";
}

const char* unary_op_name(UnaryOp op) {
    switch (op) {
        case OP_NOT: return "!";
        case OP_NEG: return "-";
        case OP_PLUS: return "+";
        default: return "?";
    }
}

/* ============================================================================
 * IMPRESIÓN DEL AST
 * ============================================================================ */

void ast_print(const ASTNode *node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    printf("%s", ast_node_type_name(node->type));
    
    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            printf(" (%zu children)\n", node->data.list.child_count);
            for (size_t i = 0; i < node->data.list.child_count; i++) {
                ast_print(node->data.list.children[i], indent + 1);
            }
            break;
            
        case AST_FUNCTION:
            printf(": %s\n", node->data.function.name ? node->data.function.name : "?");
            if (node->data.function.parameters) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Parameters:\n");
                ast_print(node->data.function.parameters, indent + 2);
            }
            if (node->data.function.body) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Body:\n");
                ast_print(node->data.function.body, indent + 2);
            }
            break;
            
        case AST_PARAMETER:
            printf(": %s: %s\n",
                   node->data.parameter.name ? node->data.parameter.name : "?",
                   node->data.parameter.type ? node->data.parameter.type : "?");
            break;
            
        case AST_LET_STMT:
            printf(": %s%s%s%s\n",
                   node->data.let_stmt.is_mutable ? "mut " : "",
                   node->data.let_stmt.name ? node->data.let_stmt.name : "?",
                   node->data.let_stmt.type ? ": " : "",
                   node->data.let_stmt.type ? node->data.let_stmt.type : "");
            if (node->data.let_stmt.initializer) {
                ast_print(node->data.let_stmt.initializer, indent + 1);
            }
            break;
            
        case AST_RETURN_STMT:
            printf("\n");
            if (node->data.return_stmt.value) {
                ast_print(node->data.return_stmt.value, indent + 1);
            }
            break;
            
        case AST_BINARY_EXPR:
            printf(": %s\n", binary_op_name(node->data.binary.op));
            ast_print(node->data.binary.left, indent + 1);
            ast_print(node->data.binary.right, indent + 1);
            break;
            
        case AST_UNARY_EXPR:
            printf(": %s\n", unary_op_name(node->data.unary.op));
            ast_print(node->data.unary.operand, indent + 1);
            break;
            
        case AST_CALL_EXPR:
            printf("\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Callee:\n");
            ast_print(node->data.call.callee, indent + 2);
            if (node->data.call.arguments && node->data.call.arguments->data.list.child_count > 0) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Arguments:\n");
                ast_print(node->data.call.arguments, indent + 2);
            }
            break;
            
        case AST_IDENTIFIER:
        case AST_NUMBER:
        case AST_BOOL:
            printf(": %s\n", node->data.literal.value ? node->data.literal.value : "?");
            break;
            
        case AST_EXPR_STMT:
            printf("\n");
            if (node->data.list.child_count > 0) {
                ast_print(node->data.list.children[0], indent + 1);
            }
            break;
            
        default:
            printf("\n");
            break;
    }
}

/* ============================================================================
 * PARSER DESCENDENTE RECURSIVO - DECLARACIONES ADELANTADAS
 * ============================================================================ */

static ASTNode* parse_expresion(Parser *parser);
static ASTNode* parse_sentencia(Parser *parser);
static ASTNode* parse_bloque(Parser *parser);

/* ============================================================================
 * PARSER - PRIMARIOS Y EXPRESIONES
 * ============================================================================ */

/**
 * Primario ::= Literal | IDENT | '(' Expresion ')'
 * Literal  ::= NUMBER | Booleano
 * Booleano ::= 'true' | 'false'
 */
static ASTNode* parse_primario(Parser *parser) {
    token_t *tok = parser->current_token;
    
    if (parser_check(parser, TOKEN_NUMBER)) {
        ASTNode *node = ast_create_literal(AST_NUMBER, tok->lexeme, tok->line, tok->column);
        parser_advance(parser);
        return node;
    }
    
    if (parser_check(parser, TOKEN_KW_TRUE) || parser_check(parser, TOKEN_KW_FALSE)) {
        ASTNode *node = ast_create_literal(AST_BOOL, tok->lexeme, tok->line, tok->column);
        parser_advance(parser);
        return node;
    }
    
    if (parser_check(parser, TOKEN_IDENTIFIER)) {
        ASTNode *node = ast_create_literal(AST_IDENTIFIER, tok->lexeme, tok->line, tok->column);
        parser_advance(parser);
        return node;
    }
    
    if (parser_match(parser, TOKEN_LPAREN)) {
        ASTNode *expr = parse_expresion(parser);
        if (!expr) return NULL;
        
        if (!parser_expect(parser, TOKEN_RPAREN, "Se esperaba ')' después de la expresión")) {
            ast_free(expr);
            return NULL;
        }
        return expr;
    }
    
    parser_error(parser, "Se esperaba una expresión");
    return NULL;
}

/**
 * Postfijo ::= Primario { Llamada }
 * Llamada  ::= '(' ListaArgumentosOpt ')'
 */
static ASTNode* parse_postfijo(Parser *parser) {
    ASTNode *expr = parse_primario(parser);
    if (!expr) return NULL;
    
    while (parser_check(parser, TOKEN_LPAREN)) {
        size_t line = parser->current_token->line;
        size_t col = parser->current_token->column;
        parser_advance(parser); // consume '('
        
        ASTNode *args = ast_create_list(AST_BLOCK, line, col);
        
        if (!parser_check(parser, TOKEN_RPAREN)) {
            // Hay argumentos
            do {
                ASTNode *arg = parse_expresion(parser);
                if (!arg) {
                    ast_free(args);
                    ast_free(expr);
                    return NULL;
                }
                ast_add_child(args, arg);
            } while (parser_match(parser, TOKEN_COMMA));
        }
        
        if (!parser_expect(parser, TOKEN_RPAREN, "Se esperaba ')' después de los argumentos")) {
            ast_free(args);
            ast_free(expr);
            return NULL;
        }
        
        expr = ast_create_call(expr, args, line, col);
    }
    
    return expr;
}

/**
 * Unario ::= ( '!' | '-' | '+' ) Unario | Postfijo
 */
static ASTNode* parse_unario(Parser *parser) {
    if (parser_check(parser, TOKEN_BANG) || 
        parser_check(parser, TOKEN_MINUS) || 
        parser_check(parser, TOKEN_PLUS)) {
        
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *operand = parse_unario(parser);
        if (!operand) return NULL;
        
        UnaryOp op;
        if (op_type == TOKEN_BANG) op = OP_NOT;
        else if (op_type == TOKEN_MINUS) op = OP_NEG;
        else op = OP_PLUS;
        
        return ast_create_unary(op, operand, tok->line, tok->column);
    }
    
    return parse_postfijo(parser);
}

/**
 * Multiplicativo ::= Unario { ( '*' | '/' | '%' ) Unario }
 */
static ASTNode* parse_multiplicativo(Parser *parser) {
    ASTNode *left = parse_unario(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_STAR) || 
           parser_check(parser, TOKEN_SLASH) ||
           parser_check(parser, TOKEN_PERCENT)) {
        
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_unario(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        BinaryOp op;
        if (op_type == TOKEN_STAR) op = OP_MUL;
        else if (op_type == TOKEN_SLASH) op = OP_DIV;
        else op = OP_MOD;
        
        left = ast_create_binary(op, left, right, tok->line, tok->column);
    }
    
    return left;
}

/**
 * Aditivo ::= Multiplicativo { ( '+' | '-' ) Multiplicativo }
 */
static ASTNode* parse_aditivo(Parser *parser) {
    ASTNode *left = parse_multiplicativo(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_PLUS) || parser_check(parser, TOKEN_MINUS)) {
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_multiplicativo(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        BinaryOp op = (op_type == TOKEN_PLUS) ? OP_ADD : OP_SUB;
        left = ast_create_binary(op, left, right, tok->line, tok->column);
    }
    
    return left;
}

/**
 * Comparacion ::= Aditivo { ( '<' | '>' | '<=' | '>=' ) Aditivo }
 */
static ASTNode* parse_comparacion(Parser *parser) {
    ASTNode *left = parse_aditivo(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_LESS) || 
           parser_check(parser, TOKEN_LESS_EQUAL) ||
           parser_check(parser, TOKEN_GREATER) || 
           parser_check(parser, TOKEN_GREATER_EQUAL)) {
        
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_aditivo(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        BinaryOp op;
        if (op_type == TOKEN_LESS) op = OP_LT;
        else if (op_type == TOKEN_LESS_EQUAL) op = OP_LE;
        else if (op_type == TOKEN_GREATER) op = OP_GT;
        else op = OP_GE;
        
        left = ast_create_binary(op, left, right, tok->line, tok->column);
    }
    
    return left;
}

/**
 * Igualdad ::= Comparacion { ( '==' | '!=' ) Comparacion }
 */
static ASTNode* parse_igualdad(Parser *parser) {
    ASTNode *left = parse_comparacion(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_EQUAL_EQUAL) || parser_check(parser, TOKEN_BANG_EQUAL)) {
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_comparacion(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        BinaryOp op = (op_type == TOKEN_EQUAL_EQUAL) ? OP_EQ : OP_NEQ;
        left = ast_create_binary(op, left, right, tok->line, tok->column);
    }
    
    return left;
}

/**
 * LogicoAND ::= Igualdad { '&&' Igualdad }
 */
static ASTNode* parse_logico_and(Parser *parser) {
    ASTNode *left = parse_igualdad(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_AND_AND)) {
        token_t *tok = parser->current_token;
        parser_advance(parser);
        
        ASTNode *right = parse_igualdad(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        left = ast_create_binary(OP_AND, left, right, tok->line, tok->column);
    }
    
    return left;
}

/**
 * LogicoOR ::= LogicoAND { '||' LogicoAND }
 */
static ASTNode* parse_logico_or(Parser *parser) {
    ASTNode *left = parse_logico_and(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_OR_OR)) {
        token_t *tok = parser->current_token;
        parser_advance(parser);
        
        ASTNode *right = parse_logico_and(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        left = ast_create_binary(OP_OR, left, right, tok->line, tok->column);
    }
    
    return left;
}

/**
 * Asignacion ::= LogicoOR [ '=' Asignacion ]
 */
static ASTNode* parse_asignacion(Parser *parser) {
    ASTNode *left = parse_logico_or(parser);
    if (!left) return NULL;
    
    if (parser_check(parser, TOKEN_EQUAL)) {
        token_t *tok = parser->current_token;
        parser_advance(parser);
        
        ASTNode *right = parse_asignacion(parser); // Asociatividad derecha
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        left = ast_create_binary(OP_ASSIGN, left, right, tok->line, tok->column);
    }
    
    return left;
}

/**
 * Expresion ::= Asignacion
 */
static ASTNode* parse_expresion(Parser *parser) {
    return parse_asignacion(parser);
}

/* ============================================================================
 * PARSER - SENTENCIAS
 * ============================================================================ */

/**
 * LetSentencia ::= 'let' [ 'mut' ] IDENT [ ':' Tipo ] [ '=' Expresion ]
 * Tipo ::= 'i32' | 'f64' | 'bool' | IDENT
 */
static ASTNode* parse_let_sentencia(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'let'
    
    bool is_mutable = parser_match(parser, TOKEN_KW_MUT);
    
    if (!parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, "Se esperaba un identificador después de 'let'");
        return NULL;
    }
    
    char *name = strdup(parser->current_token->lexeme);
    parser_advance(parser);
    
    char *type = NULL;
    if (parser_match(parser, TOKEN_COLON)) {
        if (parser_check(parser, TOKEN_KW_I32) || 
            parser_check(parser, TOKEN_KW_F64) ||
            parser_check(parser, TOKEN_KW_BOOL) || 
            parser_check(parser, TOKEN_IDENTIFIER)) {
            type = strdup(parser->current_token->lexeme);
            parser_advance(parser);
        } else {
            parser_error(parser, "Se esperaba un tipo después de ':'");
            free(name);
            return NULL;
        }
    }
    
    ASTNode *init = NULL;
    if (parser_match(parser, TOKEN_EQUAL)) {
        init = parse_expresion(parser);
        if (!init) {
            free(name);
            free(type);
            return NULL;
        }
    }
    
    // Agregar a la tabla de símbolos
    if (parser->symbol_table) {
        SymbolEntry *entry = symbol_table_insert(parser->symbol_table, name, TOKEN_IDENTIFIER, line);
        if (entry) {
            symbol_entry_set_mutable(entry, is_mutable);
            if (type) {
                symbol_entry_set_type(entry, type);
            }
        }
    }
    
    ASTNode *node = ast_create_let(name, is_mutable, type, init, line, col);
    free(name);
    free(type);
    
    return node;
}

/**
 * ReturnSentencia ::= 'return' [ Expresion ]
 */
static ASTNode* parse_return_sentencia(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'return'
    
    ASTNode *value = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        value = parse_expresion(parser);
        if (!value) return NULL;
    }
    
    return ast_create_return(value, line, col);
}

/**
 * Bloque ::= '{' ListaSentencias '}'
 * ListaSentencias ::= Sentencia ListaSentencias | epsilon
 */
static ASTNode* parse_bloque(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    if (!parser_expect(parser, TOKEN_LBRACE, "Se esperaba '{'")) {
        return NULL;
    }
    
    // Entrar a nuevo ámbito
    if (parser->symbol_table) {
        symbol_table_enter_scope(parser->symbol_table);
    }
    
    ASTNode *block = ast_create_list(AST_BLOCK, line, col);
    
    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
        ASTNode *stmt = parse_sentencia(parser);
        if (!stmt) {
            ast_free(block);
            if (parser->symbol_table) {
                symbol_table_exit_scope(parser->symbol_table);
            }
            return NULL;
        }
        ast_add_child(block, stmt);
    }
    
    if (!parser_expect(parser, TOKEN_RBRACE, "Se esperaba '}'")) {
        ast_free(block);
        if (parser->symbol_table) {
            symbol_table_exit_scope(parser->symbol_table);
        }
        return NULL;
    }
    
    // Salir del ámbito
    if (parser->symbol_table) {
        symbol_table_exit_scope(parser->symbol_table);
    }
    
    return block;
}

/**
 * Sentencia ::= LetSentencia ';' | ExprSentencia ';' | Bloque | ReturnSentencia ';'
 */
static ASTNode* parse_sentencia(Parser *parser) {
    if (parser_check(parser, TOKEN_KW_LET)) {
        ASTNode *stmt = parse_let_sentencia(parser);
        if (!stmt) return NULL;
        
        if (!parser_expect(parser, TOKEN_SEMICOLON, "Se esperaba ';' después de la sentencia let")) {
            ast_free(stmt);
            return NULL;
        }
        return stmt;
    }
    
    if (parser_check(parser, TOKEN_KW_RETURN)) {
        ASTNode *stmt = parse_return_sentencia(parser);
        if (!stmt) return NULL;
        
        if (!parser_expect(parser, TOKEN_SEMICOLON, "Se esperaba ';' después de la sentencia return")) {
            ast_free(stmt);
            return NULL;
        }
        return stmt;
    }
    
    if (parser_check(parser, TOKEN_LBRACE)) {
        return parse_bloque(parser);
    }
    
    // ExprSentencia
    ASTNode *expr = parse_expresion(parser);
    if (!expr) return NULL;
    
    if (!parser_expect(parser, TOKEN_SEMICOLON, "Se esperaba ';' después de la expresión")) {
        ast_free(expr);
        return NULL;
    }
    
    ASTNode *stmt = ast_create_list(AST_EXPR_STMT, expr->line, expr->column);
    if (stmt) {
        ast_add_child(stmt, expr);
    }
    
    return stmt;
}

/**
 * Funcion ::= 'fn' IDENT '(' ListaParametrosOpt ')' Bloque
 * ListaParametrosOpt ::= ListaParametros | epsilon
 * ListaParametros ::= Parametro { ',' Parametro }
 * Parametro ::= IDENT ':' Tipo
 */
static ASTNode* parse_funcion(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'fn'
    
    if (!parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, "Se esperaba un nombre de función");
        return NULL;
    }
    
    char *name = strdup(parser->current_token->lexeme);
    
    // Agregar función a la tabla de símbolos
    if (parser->symbol_table) {
        SymbolEntry *entry = symbol_table_insert(parser->symbol_table, name, TOKEN_IDENTIFIER, line);
        if (entry) {
            symbol_entry_set_function(entry, true);
        }
    }
    
    parser_advance(parser);
    
    if (!parser_expect(parser, TOKEN_LPAREN, "Se esperaba '(' después del nombre de la función")) {
        free(name);
        return NULL;
    }
    
    ASTNode *params = ast_create_list(AST_BLOCK, line, col);
    
    // Parámetros
    if (!parser_check(parser, TOKEN_RPAREN)) {
        do {
            if (!parser_check(parser, TOKEN_IDENTIFIER)) {
                parser_error(parser, "Se esperaba un nombre de parámetro");
                free(name);
                ast_free(params);
                return NULL;
            }
            
            char *param_name = strdup(parser->current_token->lexeme);
            size_t param_line = parser->current_token->line;
            size_t param_col = parser->current_token->column;
            parser_advance(parser);
            
            if (!parser_expect(parser, TOKEN_COLON, "Se esperaba ':' después del nombre del parámetro")) {
                free(param_name);
                free(name);
                ast_free(params);
                return NULL;
            }
            
            if (!parser_check(parser, TOKEN_KW_I32) && 
                !parser_check(parser, TOKEN_KW_F64) &&
                !parser_check(parser, TOKEN_KW_BOOL) && 
                !parser_check(parser, TOKEN_IDENTIFIER)) {
                parser_error(parser, "Se esperaba un tipo para el parámetro");
                free(param_name);
                free(name);
                ast_free(params);
                return NULL;
            }
            
            char *param_type = strdup(parser->current_token->lexeme);
            parser_advance(parser);
            
            // Agregar parámetro a la tabla de símbolos
            if (parser->symbol_table) {
                SymbolEntry *entry = symbol_table_insert(parser->symbol_table, param_name, TOKEN_IDENTIFIER, param_line);
                if (entry) {
                    symbol_entry_set_type(entry, param_type);
                    symbol_entry_set_parameter(entry, true);
                }
            }
            
            ASTNode *param = ast_create_parameter(param_name, param_type, param_line, param_col);
            free(param_name);
            free(param_type);
            
            if (param) {
                ast_add_child(params, param);
            }
            
        } while (parser_match(parser, TOKEN_COMMA));
    }
    
    if (!parser_expect(parser, TOKEN_RPAREN, "Se esperaba ')' después de los parámetros")) {
        free(name);
        ast_free(params);
        return NULL;
    }
    
    ASTNode *body = parse_bloque(parser);
    if (!body) {
        free(name);
        ast_free(params);
        return NULL;
    }
    
    ASTNode *func = ast_create_function(name, params, body, line, col);
    free(name);
    
    return func;
}

/**
 * Programa ::= ListaItems EOF
 * ListaItems ::= Item ListaItems | epsilon
 * Item ::= Funcion | Sentencia
 */
static ASTNode* parse_programa(Parser *parser) {
    ASTNode *program = ast_create_list(AST_PROGRAM, 1, 1);
    
    while (!parser_check(parser, TOKEN_EOF)) {
        ASTNode *item = NULL;
        
        if (parser_check(parser, TOKEN_KW_FN)) {
            item = parse_funcion(parser);
        } else {
            item = parse_sentencia(parser);
        }
        
        if (!item) {
            if (!parser->panic_mode) {
                ast_free(program);
                return NULL;
            }
            // Modo pánico: sincronizar
            while (!parser_check(parser, TOKEN_EOF) && 
                   !parser_check(parser, TOKEN_KW_FN) &&
                   !parser_check(parser, TOKEN_SEMICOLON)) {
                parser_advance(parser);
            }
            if (parser_check(parser, TOKEN_SEMICOLON)) {
                parser_advance(parser);
            }
            continue;
        }
        
        ast_add_child(program, item);
    }
    
    return program;
}
bool parser_init(Parser *parser, Lexer *lexer, SymbolTable *symbol_table) {
    parser->lexer = lexer;
    parser->current_token = NULL;
    parser->previous_token = NULL;
    parser->symbol_table = symbol_table;
    parser->has_error = false;
    parser->error_msg[0] = '\0';
    parser->error_line = 0;
    parser->error_col = 0;
    parser->error_count = 0;
    parser->lines_compiled = 0;
    parser->panic_mode = false;
    
    // Cargar primer token
    parser->current_token = lexer_next_token(lexer);
    
    return true;
}

void parser_free(Parser *parser) {
    if (parser->current_token) {
        free_token(parser->current_token);
        parser->current_token = NULL;
    }
    if (parser->previous_token) {
        free_token(parser->previous_token);
        parser->previous_token = NULL;
    }
}

ASTNode* parser_parse(Parser *parser) {
    return parse_programa(parser);
}

void parser_print_errors(const Parser *parser) {
    if (parser->has_error && parser->error_count > 0) {
        fprintf(stderr, "\n╔════════════════════════════════════════════╗\n");
        fprintf(stderr, "║        ERRORES SINTÁCTICOS                 ║\n");
        fprintf(stderr, "╚════════════════════════════════════════════╝\n\n");
        fprintf(stderr, "  Error en línea %zu, columna %zu:\n", parser->error_line, parser->error_col);
        fprintf(stderr, "  %s\n", parser->error_msg);
        fprintf(stderr, "\n  Total de errores sintácticos: %d\n", parser->error_count);
    }
}

void parser_print_stats(const Parser *parser) {
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║      ESTADÍSTICAS DEL ANÁLISIS             ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");
    printf("  Líneas compiladas: %d\n", parser->lines_compiled);
    printf("  Errores sintácticos: %d\n", parser->error_count);
}

int parser_get_error_count(const Parser *parser) {
    return parser->error_count;
}

int parser_get_lines_compiled(const Parser *parser) {
    return parser->lines_compiled;
}

const char* non_terminal_name(int nt) {
    static const char *names[] = {
        "Programa", "ListaItems", "Item", "Funcion", "ListaParametrosOpt",
        "ListaParametros", "Parametro", "Tipo", "Bloque", "ListaSentencias",
        "Sentencia", "LetSentencia", "ExprSentencia", "ReturnSentencia",
        "Expresion", "Asignacion", "LogicoOR", "LogicoAND", "Igualdad",
        "Comparacion", "Aditivo", "Multiplicativo", "Unario", "Postfijo",
        "Llamada", "ListaArgumentosOpt", "ListaArgumentos", "Primario",
        "Literal", "Booleano"
    };
    
    if (nt >= 0 && nt < (int)(sizeof(names) / sizeof(names[0]))) {
        return names[nt];
    }
    return "Unknown";
}
