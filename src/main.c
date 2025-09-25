/**
 * @file main.c
 * @brief Función principal del compilador
 * 
 * Punto de entrada principal del compilador que maneja los argumentos de línea
 * de comandos y coordina las diferentes fases del proceso de compilación.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/lexer.h"

/**
 * @brief Imprime la ayuda de uso del compilador.
 */
static void print_usage(const char *program_name) {
    printf("Uso: %s [opciones] <archivo>\n", program_name);
    printf("Opciones:\n");
    printf("  -l, --lex      Solo ejecutar el análisis léxico\n");
    printf("  -p, --parse    Ejecutar análisis léxico y sintáctico\n");
    printf("  -h, --help     Mostrar esta ayuda\n");
    printf("  -v, --version  Mostrar información de versión\n");
    printf("\nEjemplos:\n");
    printf("  %s programa.lang                  # Compilación completa\n", program_name);
    printf("  %s -l programa.lang              # Solo análisis léxico\n", program_name);
    printf("  %s -p programa.lang              # Análisis léxico y sintáctico\n", program_name);
}

/**
 * @brief Imprime la información de versión del compilador.
 */
static void print_version() {
    printf("Compilador v1.0.0\n");
    printf("Desarrollado como proyecto académico\n");
    printf("Análisis Léxico: AFD con funciones especializadas\n");
}

/**
 * @brief Ejecuta solo el análisis léxico de un archivo.
 * 
 * @param filename El nombre del archivo a analizar.
 * @return 0 si es exitoso, 1 si hay error.
 */
static int run_lexical_analysis(const char *filename) {
    printf("=== ANÁLISIS LÉXICO ===\n");
    printf("Archivo: %s\n\n", filename);
    
    char *source = read_file(filename);
    if (source == NULL) {
        fprintf(stderr, "Error: No se pudo leer el archivo '%s'\n", filename);
        return 1;
    }
    
    Lexer lexer;
    lexer_init(&lexer, source);
    
    printf("%-6s %-8s %-12s %s\n", "Línea", "Columna", "Tipo", "Lexema");
    printf("%-6s %-8s %-12s %s\n", "-----", "-------", "----", "------");
    
    int token_count = 0;
    for (;;) {
        token_t *token = lexer_next_token(&lexer);
        if (!token) {
            fprintf(stderr, "Error al obtener el siguiente token.\n");
            break;
        }
        
        printf("%-6zu %-8zu %-12s %s\n", 
               token->line, 
               token->column, 
               token_type_name(token->type), 
               token->lexeme ? token->lexeme : "NULL");
        
        token_count++;
        
        if (token->type == TOKEN_EOF) {
            free_token(token);
            break;
        }
        free_token(token);
    }
    
    printf("\nResumen:\n");
    printf("- Total de tokens: %d\n", token_count);
    printf("- Análisis léxico completado exitosamente\n");
    
    free(source);
    return 0;
}

/**
 * @brief Ejecuta análisis léxico y sintáctico (placeholder).
 * 
 * @param filename El nombre del archivo a analizar.
 * @return 0 si es exitoso, 1 si hay error.
 */
static int run_lexical_and_syntax_analysis(const char *filename) {
    printf("=== ANÁLISIS LÉXICO Y SINTÁCTICO ===\n");
    printf("Archivo: %s\n\n", filename);
    
    // Primero ejecutamos el análisis léxico
    char *source = read_file(filename);
    if (source == NULL) {
        fprintf(stderr, "Error: No se pudo leer el archivo '%s'\n", filename);
        return 1;
    }
    
    // Obtenemos TO-DOs los tokens
    token_t *tokens = tokenize_all(source);
    if (!tokens) {
        fprintf(stderr, "Error: No se pudieron obtener tokens del archivo\n");
        free(source);
        return 1;
    }
    
    printf("Tokens generados:\n");
    printf("%-6s %-8s %-12s %s\n", "Línea", "Columna", "Tipo", "Lexema");
    printf("%-6s %-8s %-12s %s\n", "-----", "-------", "----", "------");
    
    int token_count = 0;
    token_t *current = tokens;
    while (current) {
        printf("%-6zu %-8zu %-12s %s\n", 
               current->line, 
               current->column, 
               token_type_name(current->type), 
               current->lexeme ? current->lexeme : "NULL");
        token_count++;
        current = current->next;
    }
    
    printf("\n=== ANÁLISIS SINTÁCTICO ===\n");
    printf("TO-DO: Implementar parser\n");
    printf("- Los tokens están listos para ser procesados por el parser\n");
    printf("- Total de tokens para el parser: %d\n", token_count);
    
    free_token_list(tokens);
    free(source);
    return 0;
}

/**
 * @brief Ejecuta la compilación completa (placeholder).
 * 
 * @param filename El nombre del archivo a compilar.
 * @return 0 si es exitoso, 1 si hay error.
 */
static int run_full_compilation(const char *filename) {
    printf("=== COMPILACIÓN COMPLETA ===\n");
    printf("Archivo: %s\n\n", filename);
    
    printf("1. Análisis Léxico...\n");
    char *source = read_file(filename);
    if (source == NULL) {
        fprintf(stderr, "Error: No se pudo leer el archivo '%s'\n", filename);
        return 1;
    }
    
    token_t *tokens = tokenize_all(source);
    if (!tokens) {
        fprintf(stderr, "Error en el análisis léxico\n");
        free(source);
        return 1;
    }
    printf("   ✓ Análisis léxico completado\n");
    
    printf("2. Análisis Sintáctico...\n");
    printf("   TO-DO: Implementar parser\n");
    
    printf("3. Análisis Semántico...\n");
    printf("   TO-DO: Implementar análisis semántico\n");
    
    printf("4. Generación de Código...\n");
    printf("   TO-DO: Implementar generador de código\n");
    
    printf("\nEstado actual: Solo análisis léxico implementado\n");
    
    free_token_list(tokens);
    free(source);
    return 0;
}

/**
 * @brief Función principal del compilador.
 * 
 * @param argc Número de argumentos de línea de comandos.
 * @param argv Array de argumentos de línea de comandos.
 * @return 0 si es exitoso, código de error en caso contrario.
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: Se requiere al menos un argumento\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Procesar argumentos
    int lex_only = 0;
    int parse_only = 0;
    const char *filename = NULL;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lex") == 0) {
            lex_only = 1;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--parse") == 0) {
            parse_only = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        } else {
            fprintf(stderr, "Error: Opción desconocida '%s'\n\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!filename) {
        fprintf(stderr, "Error: Se requiere especificar un archivo\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Ejecutar según las opciones
    if (lex_only) {
        return run_lexical_analysis(filename);
    } else if (parse_only) {
        return run_lexical_and_syntax_analysis(filename);
    } else {
        return run_full_compilation(filename);
    }
}
