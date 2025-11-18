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
#include "../include/parser.h"
#include "../include/symbol_table.h"

/**
 * @brief Imprime la ayuda de uso del compilador.
 */
static void print_usage(const char *program_name) {
    printf("Uso: %s [opciones] <archivo>\n", program_name);
    printf("Opciones:\n");
    printf("  -l             Solo análisis léxico\n");
    printf("  -p             Análisis sintáctico (parser)\n");
    printf("  -t             Generar archivo de tokens\n");
    printf("  -s             Mostrar estadísticas del parser\n");
    printf("  -h, --help     Mostrar esta ayuda\n");
    printf("\nEjemplos:\n");
    printf("  %s programa.lang              # Análisis completo\n", program_name);
    printf("  %s -l programa.lang           # Solo análisis léxico\n", program_name);
    printf("  %s -p programa.lang           # Análisis sintáctico\n", program_name);
    printf("  %s -t programa.lang           # Generar archivo de tokens\n", program_name);
}

/**
 * @brief Ejecuta el análisis léxico y muestra los tokens en terminal.
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
    SymbolTable table;
    symbol_table_init(&table);
    lexer_set_symbol_table(&lexer, &table);
    
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
    
    printf("\nTotal de tokens: %d\n", token_count);
    
    symbol_table_free(&table);
    free(source);
    return 0;
}

/**
 * @brief Genera un archivo de tokens para el parser.
 * 
 * @param filename El nombre del archivo a analizar.
 * @return 0 si es exitoso, 1 si hay error.
 */
static int generate_tokens_file(const char *filename) {
    printf("=== GENERACIÓN DE ARCHIVO DE TOKENS ===\n");
    printf("Archivo fuente: %s\n", filename);
    
    // Generar nombre de archivo de salida en la carpeta docs/Analizador-sintactico/archivos_parser
    char default_output[512];
    
    // Crear nombre basado en el archivo fuente
    const char *base = strrchr(filename, '/');
    base = base ? base + 1 : filename;
    
    // Encontrar el punto de la extensión
    const char *dot = strrchr(base, '.');
    if (dot) {
        size_t len = dot - base;
        snprintf(default_output, sizeof(default_output), "docs/Analizador-sintactico/archivos_parser/%.*s_tokens.txt", (int)len, base);
    } else {
        snprintf(default_output, sizeof(default_output), "docs/Analizador-sintactico/archivos_parser/%s_tokens.txt", base);
    }
    
    printf("Archivo de salida: %s\n\n", default_output);
    
    int result = write_tokens_to_file(filename, default_output);
    
    if (result == 0) {
        printf("\n✓ Archivo de tokens generado exitosamente\n");
        printf("  - Formato: tipo_token lexema linea columna [indice_palabra_clave]\n");
        printf("  - Listo para ser usado por el parser\n");
    }
    
    return result;
}

/**
 * @brief Ejecuta el análisis sintáctico y muestra el AST.
 * 
 * @param filename El nombre del archivo a analizar.
 * @param show_stats Si se deben mostrar las estadísticas.
 * @return 0 si es exitoso, 1 si hay error.
 */
static int run_syntactic_analysis(const char *filename, int show_stats) {
    printf("=== ANÁLISIS SINTÁCTICO ===\n");
    printf("Archivo: %s\n\n", filename);
    
    char *source = read_file(filename);
    if (source == NULL) {
        fprintf(stderr, "Error: No se pudo leer el archivo '%s'\n", filename);
        return 1;
    }
    
    Lexer lexer;
    lexer_init(&lexer, source);
    
    SymbolTable symbol_table;
    symbol_table_init(&symbol_table);
    lexer_set_symbol_table(&lexer, &symbol_table);
    
    Parser parser;
    if (!parser_init(&parser, &lexer, &symbol_table)) {
        fprintf(stderr, "Error: No se pudo inicializar el parser\n");
        free(source);
        symbol_table_free(&symbol_table);
        return 1;
    }
    
    printf("⏳ Construyendo árbol de sintaxis abstracta...\n\n");
    
    ASTNode *ast = parser_parse(&parser);
    
    int errors = parser_get_error_count(&parser);
    int lines = parser_get_lines_compiled(&parser);
    
    if (errors > 0) {
        parser_print_errors(&parser);
    }
    
    if (ast) {
        printf("✓ Análisis sintáctico exitoso\n");
        
        if (show_stats) {
            parser_print_stats(&parser);
        }
        
        printf("\n═══════════════════════════════════════\n");
        printf(" ÁRBOL DE SINTAXIS ABSTRACTA (AST)\n");
        printf("═══════════════════════════════════════\n\n");
        ast_print(ast, 0);
        
        printf("\n═══════════════════════════════════════\n");
        symbol_table_print(&symbol_table);
        
        // Guardar tabla de símbolos en archivo
        char symbols_file[512];
        snprintf(symbols_file, sizeof(symbols_file), "%s.symbols.txt", filename);
        symbol_table_write_to_file(&symbol_table, symbols_file);
        
        printf("\n╔════════════════════════════════════════════╗\n");
        printf("║    RESUMEN DEL ANÁLISIS                    ║\n");
        printf("╚════════════════════════════════════════════╝\n");
        printf("  Total de errores: %d\n", errors);
        printf("  Total de líneas compiladas: %d\n", lines);
        printf("\n");
        
        ast_free(ast);
    } else {
        fprintf(stderr, "✗ Error: No se pudo construir el AST\n");
        parser_print_errors(&parser);
        
        printf("\n╔════════════════════════════════════════════╗\n");
        printf("║    RESUMEN DEL ANÁLISIS                    ║\n");
        printf("╚════════════════════════════════════════════╝\n");
        printf("  Total de errores: %d\n", errors);
        printf("  Total de líneas compiladas: %d\n", lines);
        printf("\n");
        
        parser_free(&parser);
        symbol_table_free(&symbol_table);
        free(source);
        return 1;
    }
    
    parser_free(&parser);
    symbol_table_free(&symbol_table);
    free(source);
    return errors > 0 ? 1 : 0;
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
    
    // Variables de opciones
    int lexical_only = 0;
    int parser_only = 0;
    int generate_tokens = 0;
    int show_stats = 0;
    const char *filename = NULL;
    
    // Procesar argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            lexical_only = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            parser_only = 1;
        } else if (strcmp(argv[i], "-t") == 0) {
            generate_tokens = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            show_stats = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
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
    
    // Ejecutar según la opción
    if (generate_tokens) {
        return generate_tokens_file(filename);
    } else if (lexical_only) {
        return run_lexical_analysis(filename);
    } else if (parser_only) {
        return run_syntactic_analysis(filename, show_stats);
    } else {
        // Por defecto: análisis completo
        printf("╔════════════════════════════════════════════╗\n");
        printf("║   COMPILADOR - ANÁLISIS COMPLETO           ║\n");
        printf("╚════════════════════════════════════════════╝\n\n");
        
        printf(" Fase 1: Análisis Léxico\n");
        printf("────────────────────────────\n");
        int lex_result = run_lexical_analysis(filename);
        if (lex_result != 0) {
            fprintf(stderr, "\n Error en análisis léxico\n");
            return lex_result;
        }
        
        printf("\n Fase 2: Análisis Sintáctico\n");
        printf("────────────────────────────\n");
        int parse_result = run_syntactic_analysis(filename, 1);
        
        printf("\n╔════════════════════════════════════════════╗\n");
        if (parse_result == 0) {
            printf("║    ✓ COMPILACIÓN EXITOSA                   ║\n");
        } else {
            printf("║    ✗ COMPILACIÓN CON ERRORES               ║\n");
        }
        printf("╚════════════════════════════════════════════╝\n");
        return parse_result;
    }
}
