/**
 * @file parser.c
 * @brief Implementacion del parser LR ascendente
 *
 * Ejecuta el automata LR construido a partir de las tablas compartidas y, a
 * partir de las reducciones, genera el arbol de sintaxis abstracta y alimenta
 * la tabla de simbolos correspondiente.
 */

#define _POSIX_C_SOURCE 200809L

#include "../../include/parser.h"
#include "../../include/lr_tables.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef PARSER_DEBUG_LR
/* Cuando PARSER_DEBUG_LR esta definido se imprimen tablas y reducciones. */
#endif

#ifndef strdup
extern char *strdup(const char *s);
#endif

/* ============================================================================
 * PARSER LR  -  UTILIDADES COMPARTIDAS
 * ============================================================================ */

static const LRProduction* lr_get_cached_productions(size_t *count) {
    static const LRProduction *productions = NULL;
    static size_t production_count = 0;
    if (!productions) {
        productions = lr_get_productions(&production_count);
    }
    if (count) {
        *count = production_count;
    }
    return productions;
}

typedef struct {
    TokenType type;
    char *lexeme;
    size_t line;
    size_t column;
} LRToken;

typedef struct {
    LRToken *data;
    size_t count;
    size_t capacity;
} LRTokenBuffer;

static void lr_token_buffer_init(LRTokenBuffer *buf) {
    buf->data = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

static void lr_token_buffer_free(LRTokenBuffer *buf) {
    if (!buf) return;
    for (size_t i = 0; i < buf->count; ++i) {
        free(buf->data[i].lexeme);
    }
    free(buf->data);
    buf->data = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

static bool lr_token_buffer_push(LRTokenBuffer *buf, const token_t *tok) {
    if (buf->count == buf->capacity) {
        size_t new_cap = buf->capacity ? buf->capacity * 2 : 32;
        LRToken *new_data = (LRToken*)realloc(buf->data, new_cap * sizeof(LRToken));
        if (!new_data) {
            return false;
        }
        buf->data = new_data;
        buf->capacity = new_cap;
    }
    LRToken *slot = &buf->data[buf->count];
    slot->type = tok->type;
    slot->line = tok->line;
    slot->column = tok->column;
    const char *src = tok->lexeme ? tok->lexeme : "?";
    slot->lexeme = strdup(src);
    if (!slot->lexeme) {
        return false;
    }
    buf->count++;
    return true;
}

static LRReductionNode* lr_node_create(int symbol, const char *lexeme, size_t line, size_t column) {
    LRReductionNode *node = (LRReductionNode*)calloc(1, sizeof(LRReductionNode));
    if (!node) return NULL;
    node->symbol = symbol;
    if (lexeme) {
        node->lexeme = strdup(lexeme);
    }
    node->line = line;
    node->column = column;
    return node;
}

static bool lr_node_add_child(LRReductionNode *parent, LRReductionNode *child) {
    if (!parent || !child) return false;
    if (parent->child_count == parent->capacity) {
        size_t new_cap = parent->capacity ? parent->capacity * 2 : 4;
        LRReductionNode **new_children = (LRReductionNode**)realloc(parent->children, new_cap * sizeof(LRReductionNode*));
        if (!new_children) {
            return false;
        }
        parent->children = new_children;
        parent->capacity = new_cap;
    }
    parent->children[parent->child_count++] = child;
    return true;
}

static void lr_node_free(LRReductionNode *node) {
    if (!node) return;
    for (size_t i = 0; i < node->child_count; ++i) {
        lr_node_free(node->children[i]);
    }
    free(node->children);
    free(node->lexeme);
    free(node);
}

typedef struct {
    int *data;
    size_t count;
    size_t capacity;
} LRIntVector;

static void lr_int_vector_init(LRIntVector *vec) {
    vec->data = NULL;
    vec->count = 0;
    vec->capacity = 0;
}

static void lr_int_vector_free(LRIntVector *vec) {
    free(vec->data);
    vec->data = NULL;
    vec->count = 0;
    vec->capacity = 0;
}

static bool lr_int_vector_push(LRIntVector *vec, int value) {
    if (vec->count == vec->capacity) {
        size_t new_cap = vec->capacity ? vec->capacity * 2 : 32;
        int *new_data = (int*)realloc(vec->data, new_cap * sizeof(int));
        if (!new_data) {
            return false;
        }
        vec->data = new_data;
        vec->capacity = new_cap;
    }
    vec->data[vec->count++] = value;
    return true;
}

typedef struct {
    int *data;
    size_t count;
    size_t capacity;
} LRIntStack;

static void lr_int_stack_init(LRIntStack *stack) {
    stack->data = NULL;
    stack->count = 0;
    stack->capacity = 0;
}

static void lr_int_stack_free(LRIntStack *stack) {
    free(stack->data);
    stack->data = NULL;
    stack->count = 0;
    stack->capacity = 0;
}

static bool lr_int_stack_push(LRIntStack *stack, int value) {
    if (stack->count == stack->capacity) {
        size_t new_cap = stack->capacity ? stack->capacity * 2 : 32;
        int *new_data = (int*)realloc(stack->data, new_cap * sizeof(int));
        if (!new_data) {
            return false;
        }
        stack->data = new_data;
        stack->capacity = new_cap;
    }
    stack->data[stack->count++] = value;
    return true;
}

static int lr_int_stack_top(const LRIntStack *stack) {
    if (stack->count == 0) return -1;
    return stack->data[stack->count - 1];
}

static void lr_int_stack_pop(LRIntStack *stack, size_t n) {
    if (stack->count >= n) {
        stack->count -= n;
    } else {
        stack->count = 0;
    }
}

typedef struct {
    LRReductionNode **data;
    size_t count;
    size_t capacity;
} LRNodeStack;

static void lr_node_stack_init(LRNodeStack *stack) {
    stack->data = NULL;
    stack->count = 0;
    stack->capacity = 0;
}

static void lr_node_stack_free(LRNodeStack *stack) {
    free(stack->data);
    stack->data = NULL;
    stack->count = 0;
    stack->capacity = 0;
}

static bool lr_node_stack_push(LRNodeStack *stack, LRReductionNode *node) {
    if (stack->count == stack->capacity) {
        size_t new_cap = stack->capacity ? stack->capacity * 2 : 32;
        LRReductionNode **new_data = (LRReductionNode**)realloc(stack->data, new_cap * sizeof(LRReductionNode*));
        if (!new_data) {
            return false;
        }
        stack->data = new_data;
        stack->capacity = new_cap;
    }
    stack->data[stack->count++] = node;
    return true;
}

static LRReductionNode* lr_node_stack_pop(LRNodeStack *stack) {
    if (stack->count == 0) return NULL;
    return stack->data[--stack->count];
}

typedef struct {
    const LRParseTable *table;
    LRTokenBuffer tokens;
    LRReductionNode *root;
    LRIntVector reductions;
    bool success;
    char error_msg[256];
    size_t error_line;
    size_t error_col;
    TokenType error_token;
} LRParseArtifacts;

static void lr_parse_artifacts_init(LRParseArtifacts *art) {
    art->table = NULL;
    lr_token_buffer_init(&art->tokens);
    art->root = NULL;
    lr_int_vector_init(&art->reductions);
    art->success = false;
    art->error_msg[0] = '\0';
    art->error_line = 0;
    art->error_col = 0;
    art->error_token = TOKEN_UNKNOWN;
}

static void lr_parse_artifacts_free(LRParseArtifacts *art) {
    lr_node_free(art->root);
    lr_token_buffer_free(&art->tokens);
    lr_int_vector_free(&art->reductions);
    art->root = NULL;
    art->table = NULL;
}

static bool lr_collect_tokens_for_lr(const Parser *parser, LRTokenBuffer *buffer) {
    if (!parser || !parser->source_text) {
        return false;
    }
    Lexer temp;
    lexer_init(&temp, parser->source_text);
    lexer_set_symbol_table(&temp, NULL);
    for (;;) {
        token_t *tok = lexer_next_token(&temp);
        if (!tok) {
            return false;
        }
        bool pushed = lr_token_buffer_push(buffer, tok);
        TokenType ttype = tok->type;
        free_token(tok);
        if (!pushed) {
            return false;
        }
        if (ttype == TOKEN_EOF) {
            break;
        }
    }
    return true;
}

static void lr_expected_tokens(const LRParseTable *table, size_t state, char *buffer, size_t size) {
    buffer[0] = '\0';
    bool first = true;
    for (int term = 0; term < LR_TERM_COUNT; ++term) {
        const LRAction *action = &table->action[state * LR_TERM_COUNT + term];
        if (action->type != LR_ACTION_ERROR) {
            if (!first) {
                size_t len = strlen(buffer);
                if (len + 2 < size) {
                    strncat(buffer, ", ", size - len - 1);
                }
            }
            size_t len2 = strlen(buffer);
            const char *name = lr_terminal_name(term);
            if (len2 + strlen(name) < size) {
                strncat(buffer, name, size - len2 - 1);
            }
            first = false;
        }
    }
    if (first) {
        snprintf(buffer, size, "<ninguno>");
    }
}

static bool lr_run_lr_parser(Parser *parser, LRParseArtifacts *art) {
    lr_parse_artifacts_init(art);
    const LRParseTable *table = lr_get_parse_table();
    if (!table) {
        snprintf(art->error_msg, sizeof(art->error_msg), "No se pudo construir la tabla LR");
        return false;
    }
    art->table = table;
    if (!lr_collect_tokens_for_lr(parser, &art->tokens)) {
        snprintf(art->error_msg, sizeof(art->error_msg), "No se pudo tokenizar la entrada para el parser LR");
        return false;
    }

    LRIntStack state_stack;
    LRNodeStack node_stack;
    lr_int_stack_init(&state_stack);
    lr_node_stack_init(&node_stack);
    lr_int_stack_push(&state_stack, 0);

    const LRProduction *productions = lr_get_cached_productions(NULL);
    size_t ip = 0;
    bool running = true;
    while (running) {
        if (ip >= art->tokens.count) {
            snprintf(art->error_msg, sizeof(art->error_msg), "Se alcanzo el final de tokens sin aceptar");
            break;
        }
        LRToken *lookahead = &art->tokens.data[ip];
        int term = lr_terminal_from_token(lookahead->type);
        if (term < 0) {
            snprintf(art->error_msg, sizeof(art->error_msg), "Token '%s' no soportado por la tabla LR", lookahead->lexeme ? lookahead->lexeme : "?");
            art->error_line = lookahead->line;
            art->error_col = lookahead->column;
            art->error_token = lookahead->type;
            break;
        }
        int state = lr_int_stack_top(&state_stack);
        const LRAction action = table->action[state * LR_TERM_COUNT + term];
        switch (action.type) {
            case LR_ACTION_SHIFT: {
                LRReductionNode *node = lr_node_create(
                    LR_SYMBOL_FROM_TERMINAL(term),
                    lookahead->lexeme,
                    lookahead->line,
                    lookahead->column);
                if (!node || !lr_node_stack_push(&node_stack, node)) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Sin memoria durante SHIFT");
                    running = false;
                    break;
                }
                if (!lr_int_stack_push(&state_stack, action.value)) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Sin memoria en la pila de estados LR");
                    running = false;
                    break;
                }
                ip++;
                break;
            }
            case LR_ACTION_REDUCE: {
                const LRProduction *prod = &productions[action.value];
                LRReductionNode *node = lr_node_create(prod->lhs, NULL, 0, 0);
                if (!node) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Sin memoria durante REDUCE");
                    running = false;
                    break;
                }
                // Cada produccion ya define como se reduce ,
                // por lo que solo debemos tomar los nodos RHS en orden y colgarlos del nuevo LHS.
                LRReductionNode *children[LR_MAX_RHS] = {0};
                for (int i = prod->rhs_len - 1; i >= 0; --i) {
                    children[i] = lr_node_stack_pop(&node_stack);
                    if (!children[i]) {
                        snprintf(art->error_msg, sizeof(art->error_msg), "Pila de nodos inconsistente en reduccion");
                        running = false;
                        break;
                    }
                }
                if (!running) {
                    lr_node_free(node);
                    break;
                }
                if (prod->rhs_len > 0) {
                    if (state_stack.count < (size_t)prod->rhs_len) {
                        snprintf(art->error_msg, sizeof(art->error_msg), "Pila de estados insuficiente en reduccion");
                        lr_node_free(node);
                        running = false;
                        break;
                    }
                    lr_int_stack_pop(&state_stack, (size_t)prod->rhs_len);
                }
                bool attach_ok = true;
                for (int i = 0; i < prod->rhs_len; ++i) {
                    if (!lr_node_add_child(node, children[i])) {
                        attach_ok = false;
                        break;
                    }
                }
                if (!attach_ok) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Sin memoria al adjuntar hijos durante reduccion");
                    lr_node_free(node);
                    running = false;
                    break;
                }
                if ((node->line == 0 && node->column == 0) && prod->rhs_len > 0) {
                    for (int i = 0; i < prod->rhs_len; ++i) {
                        if (children[i] && (children[i]->line || children[i]->column)) {
                            node->line = children[i]->line;
                            node->column = children[i]->column;
                            break;
                        }
                    }
                }
                int goto_state = lr_int_stack_top(&state_stack);
                if (goto_state < 0) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Pila de estados vacia en reduccion");
                    lr_node_free(node);
                    running = false;
                    break;
                }
                int next_state = table->gotos[goto_state * LR_NONTERM_COUNT + LR_SYMBOL_TO_NONTERM(prod->lhs)];
                if (next_state < 0) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Goto indefinido tras reducir %s", prod->description);
                    lr_node_free(node);
                    running = false;
                    break;
                }
                if (!lr_node_stack_push(&node_stack, node) || !lr_int_stack_push(&state_stack, next_state)) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Sin memoria tras reduccion");
                    running = false;
                    break;
                }
                if (!lr_int_vector_push(&art->reductions, action.value)) {
                    snprintf(art->error_msg, sizeof(art->error_msg), "Sin memoria al registrar reducciones");
                    running = false;
                }
                break;
            }
            case LR_ACTION_ACCEPT: {
                art->success = true;
                running = false;
                break;
            }
            case LR_ACTION_ERROR:
            default: {
                char expected[256];
                lr_expected_tokens(table, (size_t)state, expected, sizeof(expected));
                snprintf(art->error_msg, sizeof(art->error_msg), "No hay accion LR para el token '%s' en el estado %d. Esperado: %s", lookahead->lexeme ? lookahead->lexeme : "?", state, expected);
                art->error_line = lookahead->line;
                art->error_col = lookahead->column;
                art->error_token = lookahead->type;
                running = false;
                break;
            }
        }
    }

    if (art->success) {
        LRReductionNode *root = lr_node_stack_pop(&node_stack);
        art->root = root;
    } else {
        while (node_stack.count > 0) {
            lr_node_free(lr_node_stack_pop(&node_stack));
        }
    }
    lr_node_stack_free(&node_stack);
    lr_int_stack_free(&state_stack);
    return art->success;
}

#ifdef PARSER_DEBUG_LR
static void lr_print_parse_table(const LRParseTable *table) {
    if (!table) return;
    printf("=== TABLA LR: ACCION ===\n");
    for (size_t state = 0; state < table->state_count; ++state) {
        printf("Estado %zu: ", state);
        bool printed = false;
        for (int term = 0; term < LR_TERM_COUNT; ++term) {
            const LRAction *entry = &table->action[state * LR_TERM_COUNT + term];
            if (entry->type == LR_ACTION_ERROR) continue;
            printed = true;
            switch (entry->type) {
                case LR_ACTION_SHIFT:
                    printf("%s=s%d  ", lr_terminal_name(term), entry->value);
                    break;
                case LR_ACTION_REDUCE:
                    printf("%s=r%d  ", lr_terminal_name(term), entry->value);
                    break;
                case LR_ACTION_ACCEPT:
                    printf("%s=acc  ", lr_terminal_name(term));
                    break;
                default:
                    break;
            }
        }
        if (!printed) {
            printf("(sin acciones)");
        }
        printf("\n");
    }
    printf("\n=== TABLA LR: GOTO ===\n");
    for (size_t state = 0; state < table->state_count; ++state) {
        printf("Estado %zu: ", state);
        bool printed = false;
        for (int nt = 0; nt < (int)LR_NONTERM_COUNT; ++nt) {
            int dest = table->gotos[state * LR_NONTERM_COUNT + nt];
            if (dest >= 0) {
                printed = true;
                printf("%s=%d  ", lr_nonterminal_name(nt), dest);
            }
        }
        if (!printed) {
            printf("(sin transiciones)");
        }
        printf("\n");
    }
}

// Imprime el arbol de reducciones generado por el parser LR para verificar
// visualmente cada caso de la gramatica predefinida.
static void lr_print_reduction_tree(const LRReductionNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; ++i) {
        printf("  ");
    }
    const char *name = lr_symbol_name(node->symbol);
    if (LR_SYMBOL_IS_TERMINAL(node->symbol)) {
        printf("%s : %s\n", name, node->lexeme ? node->lexeme : "");
    } else {
        printf("%s\n", name);
    }
    for (size_t i = 0; i < node->child_count; ++i) {
        lr_print_reduction_tree(node->children[i], indent + 1);
    }
}

static void lr_print_reduction_sequence(const LRIntVector *reductions) {
    if (!reductions || reductions->count == 0) {
        printf("(sin reducciones registradas)\n");
        return;
    }
    const LRProduction *productions = lr_get_cached_productions(NULL);
    for (size_t i = 0; i < reductions->count; ++i) {
        int prod_index = reductions->data[i];
        if (prod_index >= 0 && prod_index < (int)LR_PRODUCTION_COUNT) {
            printf("%zu) %s\n", i + 1, productions[prod_index].description);
        }
    }
}
#endif /* PARSER_DEBUG_LR */

/* ============================================================================
 * IMPLEMENTACION DEL AST
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

ASTNode* ast_create_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, size_t line, size_t col) {
    ASTNode *node = ast_create_node(AST_IF_STMT, line, col);
    if (!node) return NULL;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
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

        case AST_IF_STMT:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            ast_free(node->data.if_stmt.else_branch);
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
        "LetStmt", "ExprStmt", "ReturnStmt", "IfStmt",
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
 * IMPRESION DEL AST
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

        case AST_IF_STMT:
            printf("\n");
            for (int i = 0; i < indent + 1; ++i) printf("  ");
            printf("Condicion:\n");
            ast_print(node->data.if_stmt.condition, indent + 2);
            for (int i = 0; i < indent + 1; ++i) printf("  ");
            printf("Then:\n");
            ast_print(node->data.if_stmt.then_branch, indent + 2);
            if (node->data.if_stmt.else_branch) {
                for (int i = 0; i < indent + 1; ++i) printf("  ");
                printf("Else:\n");
                ast_print(node->data.if_stmt.else_branch, indent + 2);
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
 * CONSTRUCCION DEL AST A PARTIR DEL ARBOL LR
 * ============================================================================ */

static ASTNode* lr_build_ast_from_tree(const LRReductionNode *root, Parser *parser);
static ASTNode* lr_build_programa(const LRReductionNode *node, Parser *parser);
static bool lr_collect_lista_items(const LRReductionNode *node, Parser *parser, ASTNode *program);
static ASTNode* lr_build_item(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_funcion(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_lista_param_opt(const LRReductionNode *node, Parser *parser);
static bool lr_fill_lista_param(const LRReductionNode *node, Parser *parser, ASTNode *list);
static bool lr_fill_lista_param_tail(const LRReductionNode *node, Parser *parser, ASTNode *list);
static ASTNode* lr_build_parametro(const LRReductionNode *node, Parser *parser);
static char* lr_build_tipo(const LRReductionNode *node);
static ASTNode* lr_build_bloque(const LRReductionNode *node, Parser *parser);
static bool lr_fill_lista_sentencias(const LRReductionNode *node, Parser *parser, ASTNode *block);
static ASTNode* lr_build_sentencia(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_let_sentencia(const LRReductionNode *node, Parser *parser);
static bool lr_eval_mut_opt(const LRReductionNode *node);
static char* lr_build_anotacion_tipo_opt(const LRReductionNode *node);
static ASTNode* lr_build_inicializacion_opt(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_expr_sentencia(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_return_sentencia(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_if_sentencia(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_if_else_body(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_expresion(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_asignacion(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_apply_asignacion_tail(ASTNode *left, const LRReductionNode *tail, Parser *parser);
static ASTNode* lr_build_logico_or(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_logico_and(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_igualdad(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_comparacion(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_aditivo(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_multiplicativo(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_unario(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_postfijo(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_apply_postfijo_tail(ASTNode *base, const LRReductionNode *tail, Parser *parser);
static ASTNode* lr_build_llamada(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_lista_argumentos_opt(const LRReductionNode *node, Parser *parser);
static bool lr_fill_argumentos(const LRReductionNode *node, Parser *parser, ASTNode *list);
static bool lr_fill_argumentos_tail(const LRReductionNode *node, Parser *parser, ASTNode *list);
static ASTNode* lr_build_primario(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_literal(const LRReductionNode *node, Parser *parser);
static ASTNode* lr_build_booleano(const LRReductionNode *node, Parser *parser);
static BinaryOp lr_binary_op_from_terminal(int terminal_symbol);
static ASTNode* lr_make_binary(Parser *parser, BinaryOp op, ASTNode *left, ASTNode *right, size_t line, size_t col);
static ASTNode* lr_make_unary(Parser *parser, UnaryOp op, ASTNode *operand, size_t line, size_t col);

static ASTNode* lr_builder_fail(Parser *parser, const char *message, size_t line, size_t col) {
    if (parser) {
        parser->has_error = true;
        parser->error_count += 1;
        snprintf(parser->error_msg, sizeof(parser->error_msg), "%s", message);
        parser->error_line = line;
        parser->error_col = col;
    }
    return NULL;
}

static bool lr_builder_error(Parser *parser, const char *message, size_t line, size_t col) {
    lr_builder_fail(parser, message, line, col);
    return false;
}

static const LRReductionNode* lr_child(const LRReductionNode *node, size_t index) {
    if (!node || index >= node->child_count) {
        return NULL;
    }
    return node->children[index];
}

static bool lr_node_is(const LRReductionNode *node, int symbol) {
    return node && node->symbol == symbol;
}

static bool lr_append_child(ASTNode *parent, ASTNode *child, Parser *parser) {
    if (!child) {
        return false;
    }
    if (!ast_add_child(parent, child)) {
        ast_free(child);
        lr_builder_error(parser, "Sin memoria al agregar hijo", parent ? parent->line : 0, parent ? parent->column : 0);
        return false;
    }
    return true;
}

static ASTNode* lr_make_binary(Parser *parser, BinaryOp op, ASTNode *left, ASTNode *right, size_t line, size_t col) {
    if (!left || !right) {
        if (left) ast_free(left);
        if (right) ast_free(right);
        return NULL;
    }
    ASTNode *node = ast_create_binary(op, left, right, line, col);
    if (!node) {
        ast_free(left);
        ast_free(right);
        lr_builder_fail(parser, "Sin memoria al crear expresion binaria", line, col);
        return NULL;
    }
    return node;
}

static ASTNode* lr_make_unary(Parser *parser, UnaryOp op, ASTNode *operand, size_t line, size_t col) {
    if (!operand) {
        return NULL;
    }
    ASTNode *node = ast_create_unary(op, operand, line, col);
    if (!node) {
        ast_free(operand);
        lr_builder_fail(parser, "Sin memoria al crear expresion unaria", line, col);
        return NULL;
    }
    return node;
}

static ASTNode* lr_build_ast_from_tree(const LRReductionNode *root, Parser *parser) {
    if (!root || root->child_count == 0) {
        return lr_builder_fail(parser, "Arbol de reducciones vacio", 0, 0);
    }
    const LRReductionNode *program_node = root;
    if (lr_node_is(root, LR_SYMBOL_FROM_NONTERM(LR_NT_START)) && root->child_count > 0) {
        program_node = lr_child(root, 0);
    }
    if (!lr_node_is(program_node, LR_SYMBOL_FROM_NONTERM(LR_NT_PROGRAMA))) {
        return lr_builder_fail(parser, "Nodo raiz no es Programa", program_node ? program_node->line : 0, program_node ? program_node->column : 0);
    }
    return lr_build_programa(program_node, parser);
}

static ASTNode* lr_build_programa(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Nodo Programa invalido", 0, 0);
    }
    ASTNode *program = ast_create_list(AST_PROGRAM, node->line ? node->line : 1, node->column);
    if (!program) {
        return lr_builder_fail(parser, "Sin memoria al crear nodo programa", node->line, node->column);
    }
    const LRReductionNode *items = lr_child(node, 0);
    if (!items) {
        ast_free(program);
        return lr_builder_fail(parser, "Programa sin ListaItems", node->line, node->column);
    }
    if (!lr_collect_lista_items(items, parser, program)) {
        ast_free(program);
        return NULL;
    }
    return program;
}

static bool lr_collect_lista_items(const LRReductionNode *node, Parser *parser, ASTNode *program) {
    if (!node) {
        return true;
    }

    if (lr_node_is(node, LR_SYMBOL_FROM_NONTERM(LR_NT_LISTA_ITEMS))) {
        if (node->child_count == 0) {
            return true;
        }
        if (node->child_count == 1) {
            return lr_collect_lista_items(lr_child(node, 0), parser, program);
        }
        ASTNode *item = lr_build_item(lr_child(node, 0), parser);
        if (!item) {
            return false;
        }
        if (!lr_append_child(program, item, parser)) {
            return false;
        }
        return lr_collect_lista_items(lr_child(node, 1), parser, program);
    }

    /* Algunos arboles reducidos pueden omitir el nodo Item cuando solo hay un elemento. */
    ASTNode *single = lr_build_item(node, parser);
    if (!single) {
        return false;
    }
    return lr_append_child(program, single, parser);
}

static ASTNode* lr_build_item(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Nodo Item invalido", 0, 0);
    }

    const LRReductionNode *payload = node;
    if (lr_node_is(node, LR_SYMBOL_FROM_NONTERM(LR_NT_ITEM))) {
        if (node->child_count == 0) {
            return lr_builder_fail(parser, "Nodo Item invalido", node->line, node->column);
        }
        payload = lr_child(node, 0);
    }

    if (lr_node_is(payload, LR_SYMBOL_FROM_NONTERM(LR_NT_FUNCION))) {
        return lr_build_funcion(payload, parser);
    }
    if (lr_node_is(payload, LR_SYMBOL_FROM_NONTERM(LR_NT_SENTENCIA))) {
        return lr_build_sentencia(payload, parser);
    }

    return lr_builder_fail(parser, "Item desconocido", payload ? payload->line : 0, payload ? payload->column : 0);
}

static ASTNode* lr_build_funcion(const LRReductionNode *node, Parser *parser) {
    const LRReductionNode *ident = lr_child(node, 1);
    const LRReductionNode *param_opt = lr_child(node, 3);
    const LRReductionNode *bloque = lr_child(node, 5);
    if (!ident || !bloque) {
        return lr_builder_fail(parser, "Funcion invalida", node->line, node->column);
    }
    ASTNode *params = lr_build_lista_param_opt(param_opt, parser);
    ASTNode *body = lr_build_bloque(bloque, parser);
    if (!params || !body) {
        if (params) ast_free(params);
        if (body) ast_free(body);
        return NULL;
    }
    const char *name = ident->lexeme ? ident->lexeme : "";
    if (parser->symbol_table) {
        SymbolEntry *entry = symbol_table_insert(parser->symbol_table, name, TOKEN_IDENTIFIER, node->line);
        if (entry) {
            symbol_entry_set_function(entry, true);
        }
    }
    ASTNode *func = ast_create_function(name, params, body, node->line, node->column);
    if (!func) {
        ast_free(params);
        ast_free(body);
        return lr_builder_fail(parser, "Sin memoria al crear funcion", node->line, node->column);
    }
    return func;
}

static ASTNode* lr_build_lista_param_opt(const LRReductionNode *node, Parser *parser) {
    size_t line = node ? node->line : 0;
    ASTNode *list = ast_create_list(AST_BLOCK, line, node ? node->column : 0);
    if (!list) {
        return lr_builder_fail(parser, "Sin memoria al crear lista de parametros", line, node ? node->column : 0);
    }
    if (!node || node->child_count == 0) {
        return list;
    }
    if (!lr_fill_lista_param(lr_child(node, 0), parser, list)) {
        ast_free(list);
        return NULL;
    }
    return list;
}

static bool lr_fill_lista_param(const LRReductionNode *node, Parser *parser, ASTNode *list) {
    if (!node || node->child_count == 0) {
        return true;
    }
    ASTNode *param = lr_build_parametro(lr_child(node, 0), parser);
    if (!param) {
        return false;
    }
    if (!lr_append_child(list, param, parser)) {
        return false;
    }
    return lr_fill_lista_param_tail(lr_child(node, 1), parser, list);
}

static bool lr_fill_lista_param_tail(const LRReductionNode *node, Parser *parser, ASTNode *list) {
    if (!node || node->child_count == 0) {
        return true;
    }
    ASTNode *param = lr_build_parametro(lr_child(node, 1), parser);
    if (!param) {
        return false;
    }
    if (!lr_append_child(list, param, parser)) {
        return false;
    }
    return lr_fill_lista_param_tail(lr_child(node, 2), parser, list);
}

static ASTNode* lr_build_parametro(const LRReductionNode *node, Parser *parser) {
    const LRReductionNode *ident = lr_child(node, 0);
    const LRReductionNode *tipo = lr_child(node, 2);
    if (!ident || !tipo) {
        return lr_builder_fail(parser, "Parametro invalido", node ? node->line : 0, node ? node->column : 0);
    }
    char *type_name = lr_build_tipo(tipo);
    if (!type_name) {
        return lr_builder_fail(parser, "Tipo de parametro invalido", tipo->line, tipo->column);
    }
    if (parser->symbol_table && ident->lexeme) {
        SymbolEntry *entry = symbol_table_insert(parser->symbol_table, ident->lexeme, TOKEN_IDENTIFIER, ident->line);
        if (entry) {
            symbol_entry_set_type(entry, type_name);
            symbol_entry_set_parameter(entry, true);
        }
    }
    ASTNode *param = ast_create_parameter(ident->lexeme ? ident->lexeme : "", type_name, node->line, node->column);
    if (!param) {
        free(type_name);
        return lr_builder_fail(parser, "Sin memoria al crear parametro", node->line, node->column);
    }
    free(type_name);
    return param;
}

static char* lr_build_tipo(const LRReductionNode *node) {
    const LRReductionNode *child = lr_child(node, 0);
    if (!child || !child->lexeme) {
        return NULL;
    }
    return strdup(child->lexeme);
}

static ASTNode* lr_build_bloque(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Bloque invalido", 0, 0);
    }
    ASTNode *block = ast_create_list(AST_BLOCK, node->line ? node->line : 0, node->column);
    if (!block) {
        return lr_builder_fail(parser, "Sin memoria al crear bloque", node->line, node->column);
    }
    if (!lr_fill_lista_sentencias(lr_child(node, 1), parser, block)) {
        ast_free(block);
        return NULL;
    }
    return block;
}

static bool lr_fill_lista_sentencias(const LRReductionNode *node, Parser *parser, ASTNode *block) {
    if (!node || node->child_count == 0) {
        return true;
    }
    ASTNode *stmt = lr_build_sentencia(lr_child(node, 0), parser);
    if (!stmt) {
        return false;
    }
    if (!lr_append_child(block, stmt, parser)) {
        return false;
    }
    return lr_fill_lista_sentencias(lr_child(node, 1), parser, block);
}

static ASTNode* lr_build_sentencia(const LRReductionNode *node, Parser *parser) {
    if (!node || node->child_count == 0) {
        return lr_builder_fail(parser, "Sentencia invalida", node ? node->line : 0, node ? node->column : 0);
    }
    const LRReductionNode *child = lr_child(node, 0);
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_LET_SENTENCIA))) {
        return lr_build_let_sentencia(child, parser);
    }
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_EXPR_SENTENCIA))) {
        return lr_build_expr_sentencia(child, parser);
    }
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_BLOQUE))) {
        return lr_build_bloque(child, parser);
    }
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_RETURN_SENTENCIA))) {
        return lr_build_return_sentencia(child, parser);
    }
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_IF_SENTENCIA))) {
        return lr_build_if_sentencia(child, parser);
    }
    return lr_builder_fail(parser, "Sentencia desconocida", child ? child->line : 0, child ? child->column : 0);
}

static ASTNode* lr_build_let_sentencia(const LRReductionNode *node, Parser *parser) {
    const LRReductionNode *ident = lr_child(node, 2);
    if (!ident || !ident->lexeme) {
        return lr_builder_fail(parser, "Declaracion let invalida", node ? node->line : 0, node ? node->column : 0);
    }
    bool is_mut = lr_eval_mut_opt(lr_child(node, 1));
    char *type_name = lr_build_anotacion_tipo_opt(lr_child(node, 3));
    ASTNode *init = lr_build_inicializacion_opt(lr_child(node, 4), parser);
    if (lr_child(node, 4) && lr_child(node, 4)->child_count > 0 && !init) {
        free(type_name);
        return NULL;
    }
    ASTNode *let_node = ast_create_let(ident->lexeme, is_mut, type_name, init, node->line, node->column);
    if (!let_node) {
        free(type_name);
        ast_free(init);
        return lr_builder_fail(parser, "Sin memoria al crear let", node->line, node->column);
    }
    if (parser->symbol_table) {
        SymbolEntry *entry = symbol_table_insert(parser->symbol_table, ident->lexeme, TOKEN_IDENTIFIER, ident->line);
        if (entry) {
            if (type_name) {
                symbol_entry_set_type(entry, type_name);
            }
            symbol_entry_set_mutable(entry, is_mut);
        }
    }
    free(type_name);
    return let_node;
}

static bool lr_eval_mut_opt(const LRReductionNode *node) {
    return node && node->child_count > 0;
}

static char* lr_build_anotacion_tipo_opt(const LRReductionNode *node) {
    if (!node || node->child_count == 0) {
        return NULL;
    }
    return lr_build_tipo(lr_child(node, 1));
}

static ASTNode* lr_build_inicializacion_opt(const LRReductionNode *node, Parser *parser) {
    if (!node || node->child_count == 0) {
        return NULL;
    }
    return lr_build_expresion(lr_child(node, 1), parser);
}

static ASTNode* lr_build_expr_sentencia(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Sentencia de expresion invalida", 0, 0);
    }
    ASTNode *expr = lr_build_expresion(lr_child(node, 0), parser);
    if (!expr) {
        return NULL;
    }
    ASTNode *wrapper = ast_create_list(AST_EXPR_STMT, node->line, node->column);
    if (!wrapper) {
        ast_free(expr);
        return lr_builder_fail(parser, "Sin memoria en sentencia de expresion", node->line, node->column);
    }
    if (!lr_append_child(wrapper, expr, parser)) {
        ast_free(wrapper);
        return NULL;
    }
    return wrapper;
}

static ASTNode* lr_build_return_sentencia(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Sentencia return invalida", 0, 0);
    }
    ASTNode *value = NULL;
    const LRReductionNode *opt = lr_child(node, 1);
    if (opt && opt->child_count > 0) {
        value = lr_build_expresion(lr_child(opt, 0), parser);
        if (!value) {
            return NULL;
        }
    }
    ASTNode *ret = ast_create_return(value, node->line, node->column);
    if (!ret) {
        ast_free(value);
        return lr_builder_fail(parser, "Sin memoria al crear return", node->line, node->column);
    }
    return ret;
}

static ASTNode* lr_build_if_sentencia(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Sentencia if invalida", 0, 0);
    }
    ASTNode *condition = lr_build_expresion(lr_child(node, 2), parser);
    if (!condition) {
        return NULL;
    }
    ASTNode *then_branch = lr_build_bloque(lr_child(node, 4), parser);
    if (!then_branch) {
        ast_free(condition);
        return NULL;
    }
    ASTNode *else_branch = NULL;
    const LRReductionNode *else_opt = lr_child(node, 5);
    if (else_opt && else_opt->child_count > 0) {
        else_branch = lr_build_if_else_body(lr_child(else_opt, 1), parser);
        if (!else_branch) {
            ast_free(condition);
            ast_free(then_branch);
            return NULL;
        }
    }
    ASTNode *if_node = ast_create_if(condition, then_branch, else_branch, node->line, node->column);
    if (!if_node) {
        ast_free(condition);
        ast_free(then_branch);
        ast_free(else_branch);
        return lr_builder_fail(parser, "Sin memoria al crear if", node->line, node->column);
    }
    return if_node;
}

static ASTNode* lr_build_if_else_body(const LRReductionNode *node, Parser *parser) {
    if (!node || node->child_count == 0) {
        return NULL;
    }
    const LRReductionNode *child = lr_child(node, 0);
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_IF_SENTENCIA))) {
        return lr_build_if_sentencia(child, parser);
    }
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_BLOQUE))) {
        return lr_build_bloque(child, parser);
    }
    return lr_builder_fail(parser, "Cuerpo else invalido", child ? child->line : 0, child ? child->column : 0);
}

static ASTNode* lr_build_expresion(const LRReductionNode *node, Parser *parser) {
    return lr_build_asignacion(lr_child(node, 0), parser);
}

static ASTNode* lr_build_asignacion(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Nodo asignacion invalido", 0, 0);
    }
    ASTNode *left = lr_build_logico_or(lr_child(node, 0), parser);
    if (!left) {
        return NULL;
    }
    return lr_apply_asignacion_tail(left, lr_child(node, 1), parser);
}

static ASTNode* lr_apply_asignacion_tail(ASTNode *left, const LRReductionNode *tail, Parser *parser) {
    if (!tail || tail->child_count == 0) {
        return left;
    }
    ASTNode *right = lr_build_asignacion(lr_child(tail, 1), parser);
    if (!right) {
        ast_free(left);
        return NULL;
    }
    ASTNode *assign = lr_make_binary(parser, OP_ASSIGN, left, right,
                                     left ? left->line : (tail ? tail->line : 0),
                                     left ? left->column : (tail ? tail->column : 0));
    return assign;
}

static ASTNode* lr_build_logico_or(const LRReductionNode *node, Parser *parser) {
    ASTNode *expr = lr_build_logico_and(lr_child(node, 0), parser);
    if (!expr) {
        return NULL;
    }
    const LRReductionNode *tail = lr_child(node, 1);
    while (tail && tail->child_count > 0) {
        ASTNode *right = lr_build_logico_and(lr_child(tail, 1), parser);
        if (!right) {
            ast_free(expr);
            return NULL;
        }
        ASTNode *combined = lr_make_binary(parser, OP_OR, expr, right, tail->line, tail->column);
        if (!combined) {
            return NULL;
        }
        expr = combined;
        tail = lr_child(tail, 2);
    }
    return expr;
}

static ASTNode* lr_build_logico_and(const LRReductionNode *node, Parser *parser) {
    ASTNode *expr = lr_build_igualdad(lr_child(node, 0), parser);
    if (!expr) {
        return NULL;
    }
    const LRReductionNode *tail = lr_child(node, 1);
    while (tail && tail->child_count > 0) {
        ASTNode *right = lr_build_igualdad(lr_child(tail, 1), parser);
        if (!right) {
            ast_free(expr);
            return NULL;
        }
        ASTNode *combined = lr_make_binary(parser, OP_AND, expr, right, tail->line, tail->column);
        if (!combined) {
            return NULL;
        }
        expr = combined;
        tail = lr_child(tail, 2);
    }
    return expr;
}

static BinaryOp lr_binary_op_from_terminal(int terminal_symbol) {
    switch (terminal_symbol) {
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_EQUAL_EQUAL): return OP_EQ;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_BANG_EQUAL): return OP_NEQ;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_LESS): return OP_LT;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_LESS_EQUAL): return OP_LE;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_GREATER): return OP_GT;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_GREATER_EQUAL): return OP_GE;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_PLUS): return OP_ADD;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_MINUS): return OP_SUB;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_STAR): return OP_MUL;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_SLASH): return OP_DIV;
        case LR_SYMBOL_FROM_TERMINAL(LR_TERM_PERCENT): return OP_MOD;
        default:
            return OP_ADD;
    }
}

static ASTNode* lr_build_igualdad(const LRReductionNode *node, Parser *parser) {
    ASTNode *expr = lr_build_comparacion(lr_child(node, 0), parser);
    if (!expr) {
        return NULL;
    }
    const LRReductionNode *tail = lr_child(node, 1);
    while (tail && tail->child_count > 0) {
        const LRReductionNode *op_node = lr_child(tail, 0);
        ASTNode *right = lr_build_comparacion(lr_child(tail, 1), parser);
        if (!right) {
            ast_free(expr);
            return NULL;
        }
        BinaryOp op = lr_binary_op_from_terminal(op_node ? op_node->symbol : -1);
        ASTNode *combined = lr_make_binary(parser, op, expr, right, op_node ? op_node->line : node->line, op_node ? op_node->column : node->column);
        if (!combined) {
            return NULL;
        }
        expr = combined;
        tail = lr_child(tail, 2);
    }
    return expr;
}

static ASTNode* lr_build_comparacion(const LRReductionNode *node, Parser *parser) {
    ASTNode *expr = lr_build_aditivo(lr_child(node, 0), parser);
    if (!expr) {
        return NULL;
    }
    const LRReductionNode *tail = lr_child(node, 1);
    while (tail && tail->child_count > 0) {
        const LRReductionNode *op_node = lr_child(tail, 0);
        ASTNode *right = lr_build_aditivo(lr_child(tail, 1), parser);
        if (!right) {
            ast_free(expr);
            return NULL;
        }
        BinaryOp op = lr_binary_op_from_terminal(op_node ? op_node->symbol : -1);
        ASTNode *combined = lr_make_binary(parser, op, expr, right, op_node ? op_node->line : node->line, op_node ? op_node->column : node->column);
        if (!combined) {
            return NULL;
        }
        expr = combined;
        tail = lr_child(tail, 2);
    }
    return expr;
}

static ASTNode* lr_build_aditivo(const LRReductionNode *node, Parser *parser) {
    ASTNode *expr = lr_build_multiplicativo(lr_child(node, 0), parser);
    if (!expr) {
        return NULL;
    }
    const LRReductionNode *tail = lr_child(node, 1);
    while (tail && tail->child_count > 0) {
        const LRReductionNode *op_node = lr_child(tail, 0);
        ASTNode *right = lr_build_multiplicativo(lr_child(tail, 1), parser);
        if (!right) {
            ast_free(expr);
            return NULL;
        }
        BinaryOp op = lr_binary_op_from_terminal(op_node ? op_node->symbol : -1);
        ASTNode *combined = lr_make_binary(parser, op, expr, right, op_node ? op_node->line : node->line, op_node ? op_node->column : node->column);
        if (!combined) {
            return NULL;
        }
        expr = combined;
        tail = lr_child(tail, 2);
    }
    return expr;
}

static ASTNode* lr_build_multiplicativo(const LRReductionNode *node, Parser *parser) {
    ASTNode *expr = lr_build_unario(lr_child(node, 0), parser);
    if (!expr) {
        return NULL;
    }
    const LRReductionNode *tail = lr_child(node, 1);
    while (tail && tail->child_count > 0) {
        const LRReductionNode *op_node = lr_child(tail, 0);
        ASTNode *right = lr_build_unario(lr_child(tail, 1), parser);
        if (!right) {
            ast_free(expr);
            return NULL;
        }
        BinaryOp op = lr_binary_op_from_terminal(op_node ? op_node->symbol : -1);
        ASTNode *combined = lr_make_binary(parser, op, expr, right, op_node ? op_node->line : node->line, op_node ? op_node->column : node->column);
        if (!combined) {
            return NULL;
        }
        expr = combined;
        tail = lr_child(tail, 2);
    }
    return expr;
}

static ASTNode* lr_build_unario(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Nodo unario invalido", 0, 0);
    }
    const LRReductionNode *first = lr_child(node, 0);
    if (lr_node_is(first, LR_SYMBOL_FROM_NONTERM(LR_NT_OPERADOR_UNARIO))) {
        const LRReductionNode *op_tok = lr_child(first, 0);
        UnaryOp op = OP_PLUS;
        if (op_tok) {
            switch (op_tok->symbol) {
                case LR_SYMBOL_FROM_TERMINAL(LR_TERM_BANG): op = OP_NOT; break;
                case LR_SYMBOL_FROM_TERMINAL(LR_TERM_MINUS): op = OP_NEG; break;
                case LR_SYMBOL_FROM_TERMINAL(LR_TERM_PLUS): op = OP_PLUS; break;
                default: break;
            }
        }
        ASTNode *operand = lr_build_unario(lr_child(node, 1), parser);
        if (!operand) {
            return NULL;
        }
        return lr_make_unary(parser, op, operand, op_tok ? op_tok->line : node->line, op_tok ? op_tok->column : node->column);
    }
    return lr_build_postfijo(first, parser);
}

static ASTNode* lr_apply_postfijo_tail(ASTNode *base, const LRReductionNode *tail, Parser *parser) {
    ASTNode *current = base;
    const LRReductionNode *iter = tail;
    while (iter && iter->child_count > 0) {
        const LRReductionNode *call_node = lr_child(iter, 0);
        ASTNode *args = lr_build_llamada(call_node, parser);
        if (!args) {
            ast_free(current);
            return NULL;
        }
        ASTNode *call = ast_create_call(current, args, call_node ? call_node->line : 0, call_node ? call_node->column : 0);
        if (!call) {
            ast_free(current);
            ast_free(args);
            return lr_builder_fail(parser, "Sin memoria al crear llamada", call_node ? call_node->line : 0, call_node ? call_node->column : 0);
        }
        current = call;
        iter = lr_child(iter, 1);
    }
    return current;
}

static ASTNode* lr_build_postfijo(const LRReductionNode *node, Parser *parser) {
    ASTNode *primary = lr_build_primario(lr_child(node, 0), parser);
    if (!primary) {
        return NULL;
    }
    return lr_apply_postfijo_tail(primary, lr_child(node, 1), parser);
}

static ASTNode* lr_build_llamada(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Nodo de llamada invalido", 0, 0);
    }
    return lr_build_lista_argumentos_opt(lr_child(node, 1), parser);
}

static ASTNode* lr_build_lista_argumentos_opt(const LRReductionNode *node, Parser *parser) {
    ASTNode *list = ast_create_list(AST_BLOCK, node ? node->line : 0, node ? node->column : 0);
    if (!list) {
        return lr_builder_fail(parser, "Sin memoria en lista de argumentos", node ? node->line : 0, node ? node->column : 0);
    }
    if (!node || node->child_count == 0) {
        return list;
    }
    if (!lr_fill_argumentos(lr_child(node, 0), parser, list)) {
        ast_free(list);
        return NULL;
    }
    return list;
}

static bool lr_fill_argumentos(const LRReductionNode *node, Parser *parser, ASTNode *list) {
    if (!node || node->child_count == 0) {
        return true;
    }
    ASTNode *expr = lr_build_expresion(lr_child(node, 0), parser);
    if (!expr) {
        return false;
    }
    if (!lr_append_child(list, expr, parser)) {
        return false;
    }
    return lr_fill_argumentos_tail(lr_child(node, 1), parser, list);
}

static bool lr_fill_argumentos_tail(const LRReductionNode *node, Parser *parser, ASTNode *list) {
    if (!node || node->child_count == 0) {
        return true;
    }
    ASTNode *expr = lr_build_expresion(lr_child(node, 1), parser);
    if (!expr) {
        return false;
    }
    if (!lr_append_child(list, expr, parser)) {
        return false;
    }
    return lr_fill_argumentos_tail(lr_child(node, 2), parser, list);
}

static ASTNode* lr_build_primario(const LRReductionNode *node, Parser *parser) {
    if (!node) {
        return lr_builder_fail(parser, "Primario invalido", 0, 0);
    }
    const LRReductionNode *child = lr_child(node, 0);
    if (!child) {
        return lr_builder_fail(parser, "Primario sin hijos", node->line, node->column);
    }
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_LITERAL))) {
        return lr_build_literal(child, parser);
    }
    if (child->symbol == LR_SYMBOL_FROM_TERMINAL(LR_TERM_IDENTIFIER)) {
        return ast_create_literal(AST_IDENTIFIER, child->lexeme ? child->lexeme : "", child->line, child->column);
    }
    if (child->symbol == LR_SYMBOL_FROM_TERMINAL(LR_TERM_LPAREN)) {
        return lr_build_expresion(lr_child(node, 1), parser);
    }
    return lr_builder_fail(parser, "Primario desconocido", child->line, child->column);
}

static ASTNode* lr_build_literal(const LRReductionNode *node, Parser *parser) {
    const LRReductionNode *child = lr_child(node, 0);
    if (!child) {
        return lr_builder_fail(parser, "Literal invalido", node ? node->line : 0, node ? node->column : 0);
    }
    if (child->symbol == LR_SYMBOL_FROM_TERMINAL(LR_TERM_NUMBER)) {
        return ast_create_literal(AST_NUMBER, child->lexeme ? child->lexeme : "", child->line, child->column);
    }
    if (lr_node_is(child, LR_SYMBOL_FROM_NONTERM(LR_NT_BOOLEANO))) {
        return lr_build_booleano(child, parser);
    }
    return lr_builder_fail(parser, "Literal desconocido", child->line, child->column);
}

static ASTNode* lr_build_booleano(const LRReductionNode *node, Parser *parser) {
    const LRReductionNode *kw = lr_child(node, 0);
    if (!kw) {
        return lr_builder_fail(parser, "Booleano invalido", node ? node->line : 0, node ? node->column : 0);
    }
    const char *value = (kw->symbol == LR_SYMBOL_FROM_TERMINAL(LR_TERM_KW_TRUE)) ? "true" : "false";
    return ast_create_literal(AST_BOOL, value, kw->line, kw->column);
}

bool parser_init(Parser *parser, Lexer *lexer, SymbolTable *symbol_table) {
    if (!parser) {
        return false;
    }
    parser->lexer = lexer;
    parser->symbol_table = symbol_table;
    parser->source_text = lexer ? lexer->source : NULL;
    parser->has_error = false;
    parser->error_msg[0] = '\0';
    parser->error_line = 0;
    parser->error_col = 0;
    parser->error_count = 0;
    parser->lines_compiled = 0;
    return true;
}

void parser_free(Parser *parser) {
    if (!parser) {
        return;
    }
    parser->lexer = NULL;
    parser->symbol_table = NULL;
    parser->source_text = NULL;
}

ASTNode* parser_parse(Parser *parser) {
    if (!parser) return NULL;

    LRParseArtifacts lr_artifacts;
    if (!lr_run_lr_parser(parser, &lr_artifacts)) {
        parser->has_error = true;
        parser->error_count = parser->error_count > 0 ? parser->error_count : 1;
        const char *msg = lr_artifacts.error_msg[0] ? lr_artifacts.error_msg : "Error durante el analisis LR";
        snprintf(parser->error_msg, sizeof(parser->error_msg), "%s", msg);
        parser->error_line = lr_artifacts.error_line;
        parser->error_col = lr_artifacts.error_col;
        lr_parse_artifacts_free(&lr_artifacts);
        return NULL;
    }

#ifdef PARSER_DEBUG_LR
    printf(" TABLA LR (ACCION / GOTO)\n");
    lr_print_parse_table(lr_artifacts.table);

    printf(" ARBOL DE REDUCCION (LR)\n");
    lr_print_reduction_tree(lr_artifacts.root, 0);

    printf(" SECUENCIA DE REDUCCIONES\n");
    lr_print_reduction_sequence(&lr_artifacts.reductions);
#endif

    int compiled_lines = 0;
    if (lr_artifacts.tokens.count > 0) {
        compiled_lines = (int)lr_artifacts.tokens.data[lr_artifacts.tokens.count - 1].line;
    }

    ASTNode *ast = lr_build_ast_from_tree(lr_artifacts.root, parser);
    lr_parse_artifacts_free(&lr_artifacts);
    if (!ast) {
        return NULL;
    }

    parser->lines_compiled = compiled_lines;
    parser->has_error = false;
    parser->error_count = 0;
    parser->error_msg[0] = '\0';
    parser->error_line = 0;
    parser->error_col = 0;
    return ast;
}

void parser_print_errors(const Parser *parser) {
    if (parser->has_error && parser->error_count > 0) {
        fprintf(stderr, " ERRORES SINTACTICOS\n");
        fprintf(stderr, "  Error en linea %zu, columna %zu:\n", parser->error_line, parser->error_col);
        fprintf(stderr, "  %s\n", parser->error_msg);
        fprintf(stderr, "\n  Total de errores sintacticos: %d\n", parser->error_count);
    }
}

void parser_print_stats(const Parser *parser) {

    printf("ESTADISTICAS DEL ANALISIS\n");
    printf("  Lineas compiladas: %d\n", parser->lines_compiled);
    printf("  Errores sintacticos: %d\n", parser->error_count);
}

int parser_get_error_count(const Parser *parser) {
    return parser->error_count;
}

int parser_get_lines_compiled(const Parser *parser) {
    return parser->lines_compiled;
}
