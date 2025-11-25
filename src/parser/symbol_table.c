/**
 * @file symbol_table.c
 * @brief Implementación de la tabla de símbolos
 */

#define _POSIX_C_SOURCE 200809L

#include "../../include/symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef strdup
extern char *strdup(const char *s);
#endif

#define SIZE_T_BUFFER 32

/* ============================================================================
 * FUNCIONES AUXILIARES PRIVADAS
 * ============================================================================ */

/**
 * @brief Crea una nueva lista de líneas
 */
static LineList* create_line_list(size_t line) {
    LineList *list = (LineList*)malloc(sizeof(LineList));
    if (!list) return NULL;
    
    list->line = line;
    list->next = NULL;
    return list;
}

/**
 * @brief Libera una lista de líneas
 */
static void free_line_list(LineList *list) {
    while (list) {
        LineList *next = list->next;
        free(list);
        list = next;
    }
}

/**
 * @brief Crea una nueva entrada de símbolo
 */
static SymbolEntry* create_symbol_entry(const char *lexeme, TokenType token_type, size_t line) {
    SymbolEntry *entry = (SymbolEntry*)calloc(1, sizeof(SymbolEntry));
    if (!entry) return NULL;
    
    entry->lexeme = strdup(lexeme);
    if (!entry->lexeme) {
        free(entry);
        return NULL;
    }
    
    entry->token_type = token_type;
    entry->data_type = NULL;
    entry->is_mutable = false;
    entry->is_function = false;
    entry->is_parameter = false;
    entry->lines = create_line_list(line);
    entry->scope_level = 0;
    entry->next = NULL;
    
    return entry;
}

/**
 * @brief Libera una entrada de símbolo
 */
static void free_symbol_entry(SymbolEntry *entry) {
    if (!entry) return;
    
    free(entry->lexeme);
    free(entry->data_type);
    free_line_list(entry->lines);
    free(entry);
}

/* ============================================================================
 * FUNCIONES PÚBLICAS DE LA TABLA DE SÍMBOLOS
 * ============================================================================ */

void symbol_table_init(SymbolTable *table) {
    table->head = NULL;
    table->tail = NULL;
    table->count = 0;
    table->current_scope = 0;
}

void symbol_table_free(SymbolTable *table) {
    SymbolEntry *current = table->head;
    while (current) {
        SymbolEntry *next = current->next;
        free_symbol_entry(current);
        current = next;
    }
    table->head = NULL;
    table->tail = NULL;
    table->count = 0;
}

SymbolEntry* symbol_table_insert(SymbolTable *table, const char *lexeme, 
                                  TokenType token_type, size_t line) {
    if (!table || !lexeme) return NULL;
    
    // Buscar si ya existe
    SymbolEntry *existing = symbol_table_lookup(table, lexeme);
    if (existing) {
        // Ya existe, agregar línea
        symbol_entry_add_line(existing, line);
        return existing;
    }
    
    // Crear nueva entrada
    SymbolEntry *entry = create_symbol_entry(lexeme, token_type, line);
    if (!entry) return NULL;
    
    entry->scope_level = table->current_scope;
    
    // Agregar a la lista
    if (!table->head) {
        table->head = entry;
        table->tail = entry;
    } else {
        table->tail->next = entry;
        table->tail = entry;
    }
    
    table->count++;
    return entry;
}

SymbolEntry* symbol_table_lookup(const SymbolTable *table, const char *lexeme) {
    if (!table || !lexeme) return NULL;
    
    SymbolEntry *current = table->head;
    while (current) {
        if (strcmp(current->lexeme, lexeme) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

SymbolEntry* symbol_table_lookup_scope(const SymbolTable *table, const char *lexeme) {
    if (!table || !lexeme) return NULL;
    
    SymbolEntry *result = NULL;
    SymbolEntry *current = table->head;
    
    // Buscar desde el ámbito actual hacia arriba
    while (current) {
        if (strcmp(current->lexeme, lexeme) == 0) {
            if (!result || current->scope_level > result->scope_level) {
                result = current;
            }
        }
        current = current->next;
    }
    
    return result;
}

void symbol_entry_set_type(SymbolEntry *entry, const char *data_type) {
    if (!entry) return;
    
    free(entry->data_type);
    entry->data_type = data_type ? strdup(data_type) : NULL;
}

void symbol_entry_set_mutable(SymbolEntry *entry, bool is_mutable) {
    if (entry) {
        entry->is_mutable = is_mutable;
    }
}

void symbol_entry_set_function(SymbolEntry *entry, bool is_function) {
    if (entry) {
        entry->is_function = is_function;
    }
}

void symbol_entry_set_parameter(SymbolEntry *entry, bool is_parameter) {
    if (entry) {
        entry->is_parameter = is_parameter;
    }
}

void symbol_table_enter_scope(SymbolTable *table) {
    if (table) {
        table->current_scope++;
    }
}

void symbol_table_exit_scope(SymbolTable *table) {
    if (table && table->current_scope > 0) {
        table->current_scope--;
    }
}

void symbol_table_remove_scope(SymbolTable *table) {
    if (!table) return;
    
    SymbolEntry *current = table->head;
    SymbolEntry *prev = NULL;
    
    while (current) {
        if (current->scope_level == table->current_scope) {
            SymbolEntry *to_delete = current;
            current = current->next;
            if (prev) {
                prev->next = current;
            } else {
                table->head = current;
            }
            if (table->tail == to_delete) {
                table->tail = prev;
            }
            free_symbol_entry(to_delete);
            if (table->count > 0) {
                table->count--;
            }
        } else {
            prev = current;
            current = current->next;
        }
    }
}

bool symbol_entry_add_line(SymbolEntry *entry, size_t line) {
    if (!entry) return false;
    
    // Verificar si la línea ya está en la lista
    LineList *current = entry->lines;
    while (current) {
        if (current->line == line) {
            return true; // Ya existe
        }
        if (!current->next) break;
        current = current->next;
    }
    
    // Agregar nueva línea al final
    LineList *new_line = create_line_list(line);
    if (!new_line) return false;
    
    if (!entry->lines) {
        entry->lines = new_line;
    } else {
        current->next = new_line;
    }
    
    return true;
}

size_t symbol_entry_count_occurrences(const SymbolEntry *entry) {
    if (!entry) return 0;
    
    size_t count = 0;
    LineList *current = entry->lines;
    while (current) {
        count++;
        current = current->next;
    }
    
    return count;
}

static size_t size_t_to_cstr(char *buffer, size_t buf_size, size_t value) {
    if (!buffer || buf_size == 0) return 0;
    char tmp[SIZE_T_BUFFER];
    size_t idx = 0;
    do {
        if (idx < sizeof(tmp)) {
            tmp[idx++] = (char)('0' + (value % 10));
        }
        value /= 10;
    } while (value > 0 && idx < sizeof(tmp));

    size_t written = idx;
    if (written + 1 > buf_size) {
        written = buf_size - 1;
    }
    for (size_t i = 0; i < written; ++i) {
        buffer[i] = tmp[written - 1 - i];
    }
    buffer[written] = '\0';
    return written;
}

static void print_size_t(FILE *stream, const char *prefix, size_t value, int newline) {
    if (!stream) return;
    if (prefix) fputs(prefix, stream);
    char buf[SIZE_T_BUFFER];
    size_t_to_cstr(buf, sizeof(buf), value);
    fputs(buf, stream);
    if (newline) fputc('\n', stream);
}

static int snprintf_size_t(char *buffer, size_t buf_size, const char *prefix, size_t value) {
    if (!buffer || buf_size == 0) return 0;
    int written = 0;
    if (prefix) {
        int prefix_written = (int)snprintf(buffer, buf_size, "%s", prefix);
        if (prefix_written < 0) return prefix_written;
        if ((size_t)prefix_written >= buf_size) return prefix_written;
        buffer += prefix_written;
        buf_size -= (size_t)prefix_written;
        written += prefix_written;
    }
    char tmp[SIZE_T_BUFFER];
    size_t len = size_t_to_cstr(tmp, sizeof(tmp), value);
    if (len >= buf_size) {
        len = buf_size > 0 ? buf_size - 1 : 0;
    }
    memcpy(buffer, tmp, len);
    if (buf_size > 0) buffer[len] = '\0';
    return written + (int)len;
}

void symbol_table_print(const SymbolTable *table) {
    if (!table) return;
    printf("     TABLA DE SIMBOLOS\n");
    print_size_t(stdout, "Total de simbolos: ", table->count, 1);
    puts("");

    printf("%-20s %-12s %-10s %-8s %-30s\n", 
           "Lexema", "Tipo Token", "Tipo Dato", "Ambito", "Lineas");
    
    SymbolEntry *current = table->head;
    while (current) {
        // Construir string de líneas
        char lines_str[256] = "";
        LineList *line_node = current->lines;
        int first = 1;
        while (line_node) {
            char temp[32];
            if (first) {
                snprintf_size_t(temp, sizeof(temp), NULL, line_node->line);
                first = 0;
            } else {
                snprintf_size_t(temp, sizeof(temp), ", ", line_node->line);
            }
            strncat(lines_str, temp, sizeof(lines_str) - strlen(lines_str) - 1);
            line_node = line_node->next;
        }
        
        // Construir tipo dato con flags
        char type_str[64] = "";
        if (current->is_function) {
            strncpy(type_str, "fn", sizeof(type_str) - 1);
        } else if (current->data_type) {
            snprintf(type_str, sizeof(type_str), "%s%s", 
                    current->is_mutable ? "mut " : "",
                    current->data_type);
        } else if (current->is_mutable) {
            strncpy(type_str, "mut", sizeof(type_str) - 1);
        } else {
            strncpy(type_str, "-", sizeof(type_str) - 1);
        }
        
        printf("%-20s %-12s %-10s %-8d %-30s\n",
               current->lexeme,
               token_type_name(current->token_type),
               type_str,
               current->scope_level,
               lines_str);
        
        current = current->next;
    }
    
    printf("\n");
}

int symbol_table_write_to_file(const SymbolTable *table, const char *filename) {
    if (!table || !filename) return 1;
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo crear el archivo '%s'\n", filename);
        return 1;
    }
    
    // Escribir encabezado
    fprintf(file, "# Tabla de Simbolos\n");
    fprintf(file, "# Formato: lexema | tipo_token | tipo_dato | ambito | lineas\n");
    print_size_t(file, "# Total de simbolos: ", table->count, 1);
    fputc('\n', file);
    
    SymbolEntry *current = table->head;
    while (current) {
        // Construir string de líneas
        char lines_str[512] = "";
        LineList *line_node = current->lines;
        int first = 1;
        while (line_node) {
            char temp[32];
            if (first) {
                snprintf_size_t(temp, sizeof(temp), NULL, line_node->line);
                first = 0;
            } else {
                snprintf_size_t(temp, sizeof(temp), ",", line_node->line);
            }
            strncat(lines_str, temp, sizeof(lines_str) - strlen(lines_str) - 1);
            line_node = line_node->next;
        }
        
        // Escribir entrada
        fprintf(file, "%s | %d | %s | %d | %s\n",
                current->lexeme,
                current->token_type,
                current->data_type ? current->data_type : "-",
                current->scope_level,
                lines_str);
        
        current = current->next;
    }
    
    fclose(file);
    printf("Tabla de simbolos escrita en: %s\n", filename);
    return 0;
}

void symbol_table_build_from_tokens(SymbolTable *table, token_t *tokens) {
    if (!table || !tokens) return;
    
    token_t *current = tokens;
    while (current) {
        // Solo agregar identificadores
        if (current->type == TOKEN_IDENTIFIER) {
            symbol_table_insert(table, current->lexeme, current->type, current->line);
        }
        
        if (current->type == TOKEN_EOF) break;
        current = current->next;
    }
}
