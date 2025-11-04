/**
 * @file parser.c
 * @brief Implementación del Parser Ascendente LR
 * 
 * Este archivo implementa un parser LR con:
 * - PILA: Para mantener estados y nodos durante el análisis
 * - MATRIZ DE TRANSICIONES: Tablas ACTION y GOTO para decisiones
 * - ÁRBOL DE REDUCCIÓN: AST construido mediante reducciones
 */

#define _POSIX_C_SOURCE 200809L

#include "../../include/parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Declaración de strdup si no está disponible
#ifndef strdup
extern char *strdup(const char *s);
#endif

/* ============================================================================
 * DEFINICIÓN DE PRODUCCIONES
 * ============================================================================ */

static const Production productions[] = {
    // 0: Programa -> ListaItems EOF
    {NT_PROGRAMA, 2, "Programa -> ListaItems EOF"},
    
    // 1-2: ListaItems
    {NT_LISTA_ITEMS, 2, "ListaItems -> Item ListaItems"},
    {NT_LISTA_ITEMS, 0, "ListaItems -> epsilon"},
    
    // 3-4: Item
    {NT_ITEM, 1, "Item -> Funcion"},
    {NT_ITEM, 1, "Item -> Sentencia"},
    
    // 5: Funcion -> fn IDENT ( ) Bloque
    {NT_FUNCION, 5, "Funcion -> fn IDENT ( ) Bloque"},
    
    // 6: Bloque -> { ListaSentencias }
    {NT_BLOQUE, 3, "Bloque -> { ListaSentencias }"},
    
    // 7-8: ListaSentencias
    {NT_LISTA_SENTENCIAS, 2, "ListaSentencias -> Sentencia ListaSentencias"},
    {NT_LISTA_SENTENCIAS, 0, "ListaSentencias -> epsilon"},
    
    // 9-17: Sentencia
    {NT_SENTENCIA, 2, "Sentencia -> LetSentencia ;"},
    {NT_SENTENCIA, 2, "Sentencia -> ExprSentencia ;"},
    {NT_SENTENCIA, 1, "Sentencia -> IfSentencia"},
    {NT_SENTENCIA, 1, "Sentencia -> WhileSentencia"},
    {NT_SENTENCIA, 2, "Sentencia -> ReturnSentencia ;"},
    {NT_SENTENCIA, 2, "Sentencia -> break ;"},
    {NT_SENTENCIA, 2, "Sentencia -> continue ;"},
    {NT_SENTENCIA, 1, "Sentencia -> Bloque"},
    
    // 16-19: LetSentencia
    {NT_LET_SENTENCIA, 4, "LetSentencia -> let IDENT : Tipo"},
    {NT_LET_SENTENCIA, 5, "LetSentencia -> let IDENT : Tipo = Expresion"},
    {NT_LET_SENTENCIA, 3, "LetSentencia -> let IDENT = Expresion"},
    {NT_LET_SENTENCIA, 5, "LetSentencia -> let mut IDENT : Tipo = Expresion"},
    
    // 20: ExprSentencia -> Expresion
    {NT_EXPR_SENTENCIA, 1, "ExprSentencia -> Expresion"},
    
    // 21-22: IfSentencia
    {NT_IF_SENTENCIA, 3, "IfSentencia -> if Expresion Bloque"},
    {NT_IF_SENTENCIA, 5, "IfSentencia -> if Expresion Bloque else Bloque"},
    
    // 23: WhileSentencia -> while Expresion Bloque
    {NT_WHILE_SENTENCIA, 3, "WhileSentencia -> while Expresion Bloque"},
    
    // 24-25: ReturnSentencia
    {NT_RETURN_SENTENCIA, 1, "ReturnSentencia -> return"},
    {NT_RETURN_SENTENCIA, 2, "ReturnSentencia -> return Expresion"},
    
    // 26: Expresion -> Asignacion
    {NT_EXPRESION, 1, "Expresion -> Asignacion"},
    
    // 27-28: Asignacion
    {NT_ASIGNACION, 1, "Asignacion -> LogicoOR"},
    {NT_ASIGNACION, 3, "Asignacion -> LogicoOR = Asignacion"},
    
    // 29-30: LogicoOR
    {NT_LOGICO_OR, 1, "LogicoOR -> LogicoAND"},
    {NT_LOGICO_OR, 3, "LogicoOR -> LogicoOR || LogicoAND"},
    
    // 31-32: LogicoAND
    {NT_LOGICO_AND, 1, "LogicoAND -> Igualdad"},
    {NT_LOGICO_AND, 3, "LogicoAND -> LogicoAND && Igualdad"},
    
    // 33-35: Igualdad
    {NT_IGUALDAD, 1, "Igualdad -> Comparacion"},
    {NT_IGUALDAD, 3, "Igualdad -> Igualdad == Comparacion"},
    {NT_IGUALDAD, 3, "Igualdad -> Igualdad != Comparacion"},
    
    // 36-41: Comparacion
    {NT_COMPARACION, 1, "Comparacion -> Term"},
    {NT_COMPARACION, 3, "Comparacion -> Comparacion < Term"},
    {NT_COMPARACION, 3, "Comparacion -> Comparacion <= Term"},
    {NT_COMPARACION, 3, "Comparacion -> Comparacion > Term"},
    {NT_COMPARACION, 3, "Comparacion -> Comparacion >= Term"},
    
    // 41-43: Term
    {NT_TERM, 1, "Term -> Factor"},
    {NT_TERM, 3, "Term -> Term + Factor"},
    {NT_TERM, 3, "Term -> Term - Factor"},
    
    // 44-47: Factor
    {NT_FACTOR, 1, "Factor -> Unario"},
    {NT_FACTOR, 3, "Factor -> Factor * Unario"},
    {NT_FACTOR, 3, "Factor -> Factor / Unario"},
    {NT_FACTOR, 3, "Factor -> Factor % Unario"},
    
    // 48-51: Unario
    {NT_UNARIO, 1, "Unario -> Postfijo"},
    {NT_UNARIO, 2, "Unario -> ! Unario"},
    {NT_UNARIO, 2, "Unario -> - Unario"},
    {NT_UNARIO, 2, "Unario -> + Unario"},
    
    // 52-53: Postfijo
    {NT_POSTFIJO, 1, "Postfijo -> Primario"},
    {NT_POSTFIJO, 4, "Postfijo -> Primario ( )"},
    
    // 54-58: Primario
    {NT_PRIMARIO, 1, "Primario -> IDENT"},
    {NT_PRIMARIO, 1, "Primario -> Literal"},
    {NT_PRIMARIO, 3, "Primario -> ( Expresion )"},
    
    // 57-61: Literal
    {NT_LITERAL, 1, "Literal -> NUMBER"},
    {NT_LITERAL, 1, "Literal -> STRING"},
    {NT_LITERAL, 1, "Literal -> CHAR"},
    {NT_LITERAL, 1, "Literal -> true"},
    {NT_LITERAL, 1, "Literal -> false"},
    
    // 62-65: Tipo
    {NT_TIPO, 1, "Tipo -> i32"},
    {NT_TIPO, 1, "Tipo -> f64"},
    {NT_TIPO, 1, "Tipo -> bool"},
    {NT_TIPO, 1, "Tipo -> char"},
};

#define NUM_PRODUCTIONS (sizeof(productions) / sizeof(Production))

/* ============================================================================
 * IMPLEMENTACIÓN DE LA PILA
 * ============================================================================ */

#define STACK_INITIAL_CAPACITY 128

bool stack_init(ParserStack *stack) {
    stack->elements = (StackElement*)malloc(sizeof(StackElement) * STACK_INITIAL_CAPACITY);
    if (!stack->elements) {
        return false;
    }
    stack->size = 0;
    stack->capacity = STACK_INITIAL_CAPACITY;
    
    // Estado inicial 0
    stack->elements[0].state = 0;
    stack->elements[0].node = NULL;
    stack->elements[0].token_type = TOKEN_EOF;
    stack->size = 1;
    
    return true;
}

void stack_free(ParserStack *stack) {
    if (stack->elements) {
        // No liberamos los nodos aquí porque forman parte del AST
        free(stack->elements);
        stack->elements = NULL;
    }
    stack->size = 0;
    stack->capacity = 0;
}

bool stack_push(ParserStack *stack, int state, ASTNode *node, TokenType token_type) {
    if (stack->size >= stack->capacity) {
        size_t new_capacity = stack->capacity * 2;
        StackElement *new_elements = (StackElement*)realloc(
            stack->elements, 
            sizeof(StackElement) * new_capacity
        );
        if (!new_elements) {
            return false;
        }
        stack->elements = new_elements;
        stack->capacity = new_capacity;
    }
    
    stack->elements[stack->size].state = state;
    stack->elements[stack->size].node = node;
    stack->elements[stack->size].token_type = token_type;
    stack->size++;
    
    return true;
}

StackElement stack_pop(ParserStack *stack) {
    if (stack->size > 0) {
        stack->size--;
        return stack->elements[stack->size];
    }
    StackElement empty = {0, NULL, TOKEN_EOF};
    return empty;
}

StackElement stack_peek(const ParserStack *stack) {
    if (stack->size > 0) {
        return stack->elements[stack->size - 1];
    }
    StackElement empty = {0, NULL, TOKEN_EOF};
    return empty;
}

int stack_top_state(const ParserStack *stack) {
    if (stack->size > 0) {
        return stack->elements[stack->size - 1].state;
    }
    return 0;
}

size_t stack_size(const ParserStack *stack) {
    return stack->size;
}

void stack_print(const ParserStack *stack) {
    printf("Pila (tamaño=%zu): [", stack->size);
    for (size_t i = 0; i < stack->size; i++) {
        if (i > 0) printf(", ");
        printf("%d", stack->elements[i].state);
    }
    printf("]\n");
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

ASTNode* ast_create_function(const char *name, ASTNode *body, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_FUNCTION, line, col);
    if (!node) return NULL;
    
    if (name) {
        node->data.function.name = strdup(name);
    }
    node->data.function.body = body;
    
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

ASTNode* ast_create_if(ASTNode *cond, ASTNode *then_br, ASTNode *else_br, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_IF_STMT, line, col);
    if (!node) return NULL;
    
    node->data.if_stmt.condition = cond;
    node->data.if_stmt.then_branch = then_br;
    node->data.if_stmt.else_branch = else_br;
    
    return node;
}

ASTNode* ast_create_while(ASTNode *cond, ASTNode *body, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_WHILE_STMT, line, col);
    if (!node) return NULL;
    
    node->data.while_stmt.condition = cond;
    node->data.while_stmt.body = body;
    
    return node;
}

ASTNode* ast_create_return(ASTNode *value, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_RETURN_STMT, line, col);
    if (!node) return NULL;
    
    node->data.return_stmt.value = value;
    
    return node;
}

void ast_free(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
        case AST_STATEMENT_LIST:
            for (size_t i = 0; i < node->data.list.child_count; i++) {
                ast_free(node->data.list.children[i]);
            }
            free(node->data.list.children);
            break;
            
        case AST_FUNCTION:
            free(node->data.function.name);
            ast_free(node->data.function.body);
            break;
            
        case AST_LET_STMT:
            free(node->data.let_stmt.name);
            free(node->data.let_stmt.type);
            ast_free(node->data.let_stmt.initializer);
            break;
            
        case AST_IF_STMT:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            ast_free(node->data.if_stmt.else_branch);
            break;
            
        case AST_WHILE_STMT:
            ast_free(node->data.while_stmt.condition);
            ast_free(node->data.while_stmt.body);
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
            for (size_t i = 0; i < node->data.call.arg_count; i++) {
                ast_free(node->data.call.args[i]);
            }
            free(node->data.call.args);
            break;
            
        case AST_IDENTIFIER:
        case AST_NUMBER:
        case AST_STRING:
        case AST_CHAR:
        case AST_BOOL:
            free(node->data.literal.value);
            break;
            
        default:
            break;
    }
    
    free(node);
}

/* ============================================================================
 * FUNCIONES AUXILIARES DE NOMBRES
 * ============================================================================ */

const char* ast_node_type_name(ASTNodeType type) {
    static const char *names[] = {
        "Program", "Function", "Block", "StatementList",
        "LetStmt", "ExprStmt", "IfStmt", "WhileStmt", "ForStmt",
        "ReturnStmt", "BreakStmt", "ContinueStmt",
        "BinaryExpr", "UnaryExpr", "CallExpr", "AssignExpr",
        "Identifier", "Number", "String", "Char", "Bool", "Array"
    };
    
    if (type >= 0 && type < (sizeof(names) / sizeof(names[0]))) {
        return names[type];
    }
    return "Unknown";
}

const char* non_terminal_name(NonTerminal nt) {
    static const char *names[] = {
        "Programa", "ListaItems", "Item", "Funcion", "Bloque",
        "ListaSentencias", "Sentencia", "LetSentencia", "ExprSentencia",
        "IfSentencia", "WhileSentencia", "ReturnSentencia",
        "Expresion", "Asignacion", "LogicoOR", "LogicoAND",
        "Igualdad", "Comparacion", "Term", "Factor",
        "Unario", "Postfijo", "Primario", "Literal", "Tipo"
    };
    
    if (nt >= 0 && nt < NT_COUNT) {
        return names[nt];
    }
    return "Unknown";
}

const char* action_type_name(ActionType type) {
    switch (type) {
        case ACTION_SHIFT: return "SHIFT";
        case ACTION_REDUCE: return "REDUCE";
        case ACTION_ACCEPT: return "ACCEPT";
        case ACTION_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

static const char* binary_op_name(BinaryOp op) {
    static const char *names[] = {
        "+", "-", "*", "/", "%",
        "==", "!=", "<", "<=", ">", ">=",
        "&&", "||",
        "=", "+=", "-=", "*=", "/=", "%="
    };
    
    if (op >= 0 && op < (sizeof(names) / sizeof(names[0]))) {
        return names[op];
    }
    return "?";
}

static const char* unary_op_name(UnaryOp op) {
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
        case AST_STATEMENT_LIST:
            printf(" (%zu children)\n", node->data.list.child_count);
            for (size_t i = 0; i < node->data.list.child_count; i++) {
                ast_print(node->data.list.children[i], indent + 1);
            }
            break;
            
        case AST_FUNCTION:
            printf(": %s\n", node->data.function.name ? node->data.function.name : "anonymous");
            ast_print(node->data.function.body, indent + 1);
            break;
            
        case AST_LET_STMT:
            printf(": %s%s%s\n",
                   node->data.let_stmt.is_mutable ? "mut " : "",
                   node->data.let_stmt.name ? node->data.let_stmt.name : "?",
                   node->data.let_stmt.type ? node->data.let_stmt.type : "");
            if (node->data.let_stmt.initializer) {
                ast_print(node->data.let_stmt.initializer, indent + 1);
            }
            break;
            
        case AST_IF_STMT:
            printf("\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            ast_print(node->data.if_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Then:\n");
            ast_print(node->data.if_stmt.then_branch, indent + 2);
            if (node->data.if_stmt.else_branch) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Else:\n");
                ast_print(node->data.if_stmt.else_branch, indent + 2);
            }
            break;
            
        case AST_WHILE_STMT:
            printf("\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            ast_print(node->data.while_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Body:\n");
            ast_print(node->data.while_stmt.body, indent + 2);
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
            
        case AST_IDENTIFIER:
        case AST_NUMBER:
        case AST_STRING:
        case AST_CHAR:
        case AST_BOOL:
            printf(": %s\n", node->data.literal.value ? node->data.literal.value : "null");
            break;
            
        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
            printf("\n");
            break;
            
        default:
            printf(" (not implemented)\n");
            break;
    }
}

/* ============================================================================
 * MATRIZ DE TRANSICIONES SIMPLIFICADA
 * Para una implementación completa, estas tablas serían mucho más grandes
 * ============================================================================ */

/**
 * @brief Obtiene la acción para un estado y token dados
 * Esta es una versión simplificada. Una tabla LR completa sería muy grande.
 */
static Action get_action(int state __attribute__((unused)), TokenType token __attribute__((unused))) {
    Action action;
    
    // Por simplicidad, implementamos un parser recursivo descendente
    // en lugar de tablas LR completas
    action.type = ACTION_ERROR;
    action.value = 0;
    
    return action;
}

/**
 * @brief Obtiene el siguiente estado para un no terminal
 */
static int get_goto(int state __attribute__((unused)), NonTerminal nt __attribute__((unused))) {
    // Tabla GOTO simplificada
    return -1;
}

/* ============================================================================
 * PARSER RECURSIVO DESCENDENTE
 * (Más simple que implementar tablas LR completas)
 * ============================================================================ */

// Declaraciones adelantadas
static ASTNode* parse_expression(Parser *parser);
static ASTNode* parse_statement(Parser *parser);
static ASTNode* parse_block(Parser *parser);

static void parser_advance(Parser *parser) {
    if (parser->current_token) {
        free_token(parser->current_token);
    }
    parser->current_token = lexer_next_token(parser->lexer);
}

static bool parser_check(Parser *parser, TokenType type) {
    return parser->current_token && parser->current_token->type == type;
}

static bool parser_match(Parser *parser, TokenType type) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return true;
    }
    return false;
}

static void parser_error_msg(Parser *parser, const char *msg) {
    parser->has_error = true;
    snprintf(parser->error_msg, sizeof(parser->error_msg), "%s", msg);
    if (parser->current_token) {
        parser->error_line = parser->current_token->line;
        parser->error_col = parser->current_token->column;
    }
}

// Parsing de expresiones con precedencia

static ASTNode* parse_primary(Parser *parser) {
    token_t *tok = parser->current_token;
    
    if (parser_check(parser, TOKEN_NUMBER)) {
        ASTNode *node = ast_create_literal(AST_NUMBER, tok->lexeme, tok->line, tok->column);
        parser_advance(parser);
        return node;
    }
    
    if (parser_check(parser, TOKEN_STRING)) {
        ASTNode *node = ast_create_literal(AST_STRING, tok->lexeme, tok->line, tok->column);
        parser_advance(parser);
        return node;
    }
    
    if (parser_check(parser, TOKEN_CHAR)) {
        ASTNode *node = ast_create_literal(AST_CHAR, tok->lexeme, tok->line, tok->column);
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
        ASTNode *expr = parse_expression(parser);
        if (!parser_match(parser, TOKEN_RPAREN)) {
            parser_error_msg(parser, "Se esperaba ')'");
            ast_free(expr);
            return NULL;
        }
        return expr;
    }
    
    parser_error_msg(parser, "Expresión esperada");
    return NULL;
}

static ASTNode* parse_unary(Parser *parser) {
    if (parser_check(parser, TOKEN_BANG) || parser_check(parser, TOKEN_MINUS) || 
        parser_check(parser, TOKEN_PLUS)) {
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *operand = parse_unary(parser);
        if (!operand) return NULL;
        
        UnaryOp op;
        if (op_type == TOKEN_BANG) op = OP_NOT;
        else if (op_type == TOKEN_MINUS) op = OP_NEG;
        else op = OP_PLUS;
        
        return ast_create_unary(op, operand, tok->line, tok->column);
    }
    
    return parse_primary(parser);
}

static ASTNode* parse_factor(Parser *parser) {
    ASTNode *left = parse_unary(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_STAR) || parser_check(parser, TOKEN_SLASH) ||
           parser_check(parser, TOKEN_PERCENT)) {
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_unary(parser);
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

static ASTNode* parse_term(Parser *parser) {
    ASTNode *left = parse_factor(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_PLUS) || parser_check(parser, TOKEN_MINUS)) {
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_factor(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        BinaryOp op = (op_type == TOKEN_PLUS) ? OP_ADD : OP_SUB;
        left = ast_create_binary(op, left, right, tok->line, tok->column);
    }
    
    return left;
}

static ASTNode* parse_comparison(Parser *parser) {
    ASTNode *left = parse_term(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_LESS) || parser_check(parser, TOKEN_LESS_EQUAL) ||
           parser_check(parser, TOKEN_GREATER) || parser_check(parser, TOKEN_GREATER_EQUAL)) {
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_term(parser);
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

static ASTNode* parse_equality(Parser *parser) {
    ASTNode *left = parse_comparison(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_EQUAL_EQUAL) || parser_check(parser, TOKEN_BANG_EQUAL)) {
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_comparison(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        BinaryOp op = (op_type == TOKEN_EQUAL_EQUAL) ? OP_EQ : OP_NEQ;
        left = ast_create_binary(op, left, right, tok->line, tok->column);
    }
    
    return left;
}

static ASTNode* parse_logical_and(Parser *parser) {
    ASTNode *left = parse_equality(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_AND_AND)) {
        token_t *tok = parser->current_token;
        parser_advance(parser);
        
        ASTNode *right = parse_equality(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        left = ast_create_binary(OP_AND, left, right, tok->line, tok->column);
    }
    
    return left;
}

static ASTNode* parse_logical_or(Parser *parser) {
    ASTNode *left = parse_logical_and(parser);
    if (!left) return NULL;
    
    while (parser_check(parser, TOKEN_OR_OR)) {
        token_t *tok = parser->current_token;
        parser_advance(parser);
        
        ASTNode *right = parse_logical_and(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        left = ast_create_binary(OP_OR, left, right, tok->line, tok->column);
    }
    
    return left;
}

static ASTNode* parse_assignment(Parser *parser) {
    ASTNode *left = parse_logical_or(parser);
    if (!left) return NULL;
    
    if (parser_check(parser, TOKEN_EQUAL) || parser_check(parser, TOKEN_PLUS_EQUAL) ||
        parser_check(parser, TOKEN_MINUS_EQUAL) || parser_check(parser, TOKEN_STAR_EQUAL) ||
        parser_check(parser, TOKEN_SLASH_EQUAL) || parser_check(parser, TOKEN_PERCENT_EQUAL)) {
        
        token_t *tok = parser->current_token;
        TokenType op_type = tok->type;
        parser_advance(parser);
        
        ASTNode *right = parse_assignment(parser);
        if (!right) {
            ast_free(left);
            return NULL;
        }
        
        BinaryOp op;
        switch (op_type) {
            case TOKEN_EQUAL: op = OP_ASSIGN; break;
            case TOKEN_PLUS_EQUAL: op = OP_ADD_ASSIGN; break;
            case TOKEN_MINUS_EQUAL: op = OP_SUB_ASSIGN; break;
            case TOKEN_STAR_EQUAL: op = OP_MUL_ASSIGN; break;
            case TOKEN_SLASH_EQUAL: op = OP_DIV_ASSIGN; break;
            case TOKEN_PERCENT_EQUAL: op = OP_MOD_ASSIGN; break;
            default: op = OP_ASSIGN; break;
        }
        
        left = ast_create_binary(op, left, right, tok->line, tok->column);
    }
    
    return left;
}

static ASTNode* parse_expression(Parser *parser) {
    parser->shift_count++;
    return parse_assignment(parser);
}

static ASTNode* parse_let_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'let'
    
    bool is_mutable = false;
    if (parser_match(parser, TOKEN_KW_MUT)) {
        is_mutable = true;
    }
    
    if (!parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_error_msg(parser, "Se esperaba identificador después de 'let'");
        return NULL;
    }
    
    char *name = strdup(parser->current_token->lexeme);
    parser_advance(parser);
    
    char *type = NULL;
    if (parser_match(parser, TOKEN_COLON)) {
        if (parser_check(parser, TOKEN_KW_I32) || parser_check(parser, TOKEN_KW_F64) ||
            parser_check(parser, TOKEN_KW_BOOL) || parser_check(parser, TOKEN_KW_CHAR)) {
            type = strdup(parser->current_token->lexeme);
            parser_advance(parser);
        }
    }
    
    ASTNode *init = NULL;
    if (parser_match(parser, TOKEN_EQUAL)) {
        init = parse_expression(parser);
        if (!init) {
            free(name);
            free(type);
            return NULL;
        }
    }
    
    ASTNode *node = ast_create_let(name, is_mutable, type, init, line, col);
    free(name);
    free(type);
    
    parser->reduce_count++;
    return node;
}

static ASTNode* parse_if_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'if'
    
    ASTNode *cond = parse_expression(parser);
    if (!cond) return NULL;
    
    ASTNode *then_br = parse_block(parser);
    if (!then_br) {
        ast_free(cond);
        return NULL;
    }
    
    ASTNode *else_br = NULL;
    if (parser_match(parser, TOKEN_KW_ELSE)) {
        if (parser_check(parser, TOKEN_KW_IF)) {
            else_br = parse_if_statement(parser);
        } else {
            else_br = parse_block(parser);
        }
        
        if (!else_br) {
            ast_free(cond);
            ast_free(then_br);
            return NULL;
        }
    }
    
    parser->reduce_count++;
    return ast_create_if(cond, then_br, else_br, line, col);
}

static ASTNode* parse_while_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'while'
    
    ASTNode *cond = parse_expression(parser);
    if (!cond) return NULL;
    
    ASTNode *body = parse_block(parser);
    if (!body) {
        ast_free(cond);
        return NULL;
    }
    
    parser->reduce_count++;
    return ast_create_while(cond, body, line, col);
}

static ASTNode* parse_return_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'return'
    
    ASTNode *value = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        value = parse_expression(parser);
    }
    
    parser->reduce_count++;
    return ast_create_return(value, line, col);
}

static ASTNode* parse_block(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    if (!parser_match(parser, TOKEN_LBRACE)) {
        parser_error_msg(parser, "Se esperaba '{'");
        return NULL;
    }
    
    ASTNode *block = ast_create_list(AST_BLOCK, line, col);
    
    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
        ASTNode *stmt = parse_statement(parser);
        if (!stmt) {
            ast_free(block);
            return NULL;
        }
        ast_add_child(block, stmt);
    }
    
    if (!parser_match(parser, TOKEN_RBRACE)) {
        parser_error_msg(parser, "Se esperaba '}'");
        ast_free(block);
        return NULL;
    }
    
    parser->reduce_count++;
    return block;
}

static ASTNode* parse_statement(Parser *parser) {
    if (parser_check(parser, TOKEN_KW_LET)) {
        ASTNode *stmt = parse_let_statement(parser);
        if (stmt && !parser_match(parser, TOKEN_SEMICOLON)) {
            parser_error_msg(parser, "Se esperaba ';'");
            ast_free(stmt);
            return NULL;
        }
        return stmt;
    }
    
    if (parser_check(parser, TOKEN_KW_IF)) {
        return parse_if_statement(parser);
    }
    
    if (parser_check(parser, TOKEN_KW_WHILE)) {
        return parse_while_statement(parser);
    }
    
    if (parser_check(parser, TOKEN_KW_RETURN)) {
        ASTNode *stmt = parse_return_statement(parser);
        if (stmt && !parser_match(parser, TOKEN_SEMICOLON)) {
            parser_error_msg(parser, "Se esperaba ';'");
            ast_free(stmt);
            return NULL;
        }
        return stmt;
    }
    
    if (parser_check(parser, TOKEN_KW_BREAK)) {
        size_t line = parser->current_token->line;
        size_t col = parser->current_token->column;
        parser_advance(parser);
        if (!parser_match(parser, TOKEN_SEMICOLON)) {
            parser_error_msg(parser, "Se esperaba ';'");
            return NULL;
        }
        return ast_create_node(AST_BREAK_STMT, line, col);
    }
    
    if (parser_check(parser, TOKEN_KW_CONTINUE)) {
        size_t line = parser->current_token->line;
        size_t col = parser->current_token->column;
        parser_advance(parser);
        if (!parser_match(parser, TOKEN_SEMICOLON)) {
            parser_error_msg(parser, "Se esperaba ';'");
            return NULL;
        }
        return ast_create_node(AST_CONTINUE_STMT, line, col);
    }
    
    if (parser_check(parser, TOKEN_LBRACE)) {
        return parse_block(parser);
    }
    
    // Expresión como sentencia
    ASTNode *expr = parse_expression(parser);
    if (!expr) return NULL;
    
    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        parser_error_msg(parser, "Se esperaba ';'");
        ast_free(expr);
        return NULL;
    }
    
    ASTNode *stmt = ast_create_node(AST_EXPR_STMT, expr->line, expr->column);
    if (stmt) {
        // Almacenar la expresión
        stmt->data.list.children = (ASTNode**)malloc(sizeof(ASTNode*));
        stmt->data.list.children[0] = expr;
        stmt->data.list.child_count = 1;
        stmt->data.list.capacity = 1;
    }
    
    return stmt;
}

static ASTNode* parse_function(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t col = parser->current_token->column;
    
    parser_advance(parser); // consume 'fn'
    
    if (!parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_error_msg(parser, "Se esperaba nombre de función");
        return NULL;
    }
    
    char *name = strdup(parser->current_token->lexeme);
    parser_advance(parser);
    
    if (!parser_match(parser, TOKEN_LPAREN)) {
        parser_error_msg(parser, "Se esperaba '('");
        free(name);
        return NULL;
    }
    
    // Simplificado: sin parámetros
    if (!parser_match(parser, TOKEN_RPAREN)) {
        parser_error_msg(parser, "Se esperaba ')'");
        free(name);
        return NULL;
    }
    
    ASTNode *body = parse_block(parser);
    if (!body) {
        free(name);
        return NULL;
    }
    
    ASTNode *func = ast_create_function(name, body, line, col);
    free(name);
    
    parser->reduce_count++;
    return func;
}

static ASTNode* parse_program(Parser *parser) {
    ASTNode *program = ast_create_list(AST_PROGRAM, 1, 1);
    
    while (!parser_check(parser, TOKEN_EOF)) {
        ASTNode *item = NULL;
        
        if (parser_check(parser, TOKEN_KW_FN)) {
            item = parse_function(parser);
        } else {
            item = parse_statement(parser);
        }
        
        if (!item) {
            ast_free(program);
            return NULL;
        }
        
        ast_add_child(program, item);
    }
    
    return program;
}

/* ============================================================================
 * FUNCIONES PÚBLICAS DEL PARSER
 * ============================================================================ */

bool parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer = lexer;
    parser->current_token = NULL;
    parser->has_error = false;
    parser->error_msg[0] = '\0';
    parser->error_line = 0;
    parser->error_col = 0;
    parser->shift_count = 0;
    parser->reduce_count = 0;
    
    if (!stack_init(&parser->stack)) {
        return false;
    }
    
    // Cargar primer token
    parser->current_token = lexer_next_token(lexer);
    
    return true;
}

void parser_free(Parser *parser) {
    if (parser->current_token) {
        free_token(parser->current_token);
        parser->current_token = NULL;
    }
    stack_free(&parser->stack);
}

ASTNode* parser_parse(Parser *parser) {
    return parse_program(parser);
}

void parser_print_error(const Parser *parser) {
    if (parser->has_error) {
        fprintf(stderr, "\n  Error de parsing en línea %zu, columna %zu:\n",
                parser->error_line, parser->error_col);
        fprintf(stderr, "   %s\n", parser->error_msg);
    }
}

void parser_print_stats(const Parser *parser) {
    printf("\n  Estadísticas del Parser:\n");
    printf("   Desplazamientos (shift): %d\n", parser->shift_count);
    printf("   Reducciones (reduce):    %d\n", parser->reduce_count);
    printf("   Tamaño de pila:          %zu\n", stack_size(&parser->stack));
}
