/**
 * @file symbol_table.h
 * @brief Definición de la tabla de símbolos del compilador
 * 
 * La tabla de símbolos almacena información sobre identificadores encontrados
 * durante el análisis léxico y semántico. Mantiene información sobre:
 * - Lexema (nombre del identificador)
 * - Tipo de token
 * - Líneas donde aparece el identificador
 * - Tipo de dato (si se conoce)
 * - Ámbito (scope)
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "lexer.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct LineList {
    size_t line;               /**< Número de línea */
    struct LineList *next;     /**< Siguiente línea */
} LineList;

typedef struct SymbolEntry {
    char *lexeme;              /**< Lexema (nombre del identificador) */
    TokenType token_type;      /**< Tipo de token */
    char *data_type;           /**< Tipo de dato (i32, f64, bool, etc.) */
    bool is_mutable;           /**< Indica si es mutable (mut) */
    bool is_function;          /**< Indica si es una función */
    bool is_parameter;         /**< Indica si es un parámetro de función */
    LineList *lines;           /**< Lista de líneas donde aparece */
    int scope_level;           /**< Nivel de ámbito (0 = global) */
    struct SymbolEntry *next;  /**< Siguiente entrada  */
} SymbolEntry;

typedef struct SymbolTable {
    SymbolEntry *head;         /**< Primera entrada de la tabla */
    SymbolEntry *tail;         /**< Última entrada de la tabla */
    size_t count;              /**< Número de entradas */
    int current_scope;         /**< Nivel de ámbito actual */
} SymbolTable;

void symbol_table_init(SymbolTable *table);

void symbol_table_free(SymbolTable *table);

SymbolEntry* symbol_table_insert(SymbolTable *table, const char *lexeme, TokenType token_type, size_t line);

SymbolEntry* symbol_table_lookup(const SymbolTable *table, const char *lexeme);

SymbolEntry* symbol_table_lookup_scope(const SymbolTable *table, const char *lexeme);

void symbol_entry_set_type(SymbolEntry *entry, const char *data_type);

void symbol_entry_set_mutable(SymbolEntry *entry, bool is_mutable);

void symbol_entry_set_function(SymbolEntry *entry, bool is_function);

void symbol_entry_set_parameter(SymbolEntry *entry, bool is_parameter);

void symbol_table_enter_scope(SymbolTable *table);

void symbol_table_exit_scope(SymbolTable *table);

void symbol_table_remove_scope(SymbolTable *table);

void symbol_table_print(const SymbolTable *table);

int symbol_table_write_to_file(const SymbolTable *table, const char *filename);

void symbol_table_build_from_tokens(SymbolTable *table, token_t *tokens);

bool symbol_entry_add_line(SymbolEntry *entry, size_t line);

size_t symbol_entry_count_occurrences(const SymbolEntry *entry);

#endif // SYMBOL_TABLE_H
