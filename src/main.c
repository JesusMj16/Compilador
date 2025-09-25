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
    printf("  -t             Generar archivo de tokens\n");
    printf("  -h, --help     Mostrar esta ayuda\n");
    printf("\nEjemplos:\n");
    printf("  %s programa.lang              # Análisis léxico en terminal\n", program_name);
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
    
    // Variables simples
    int generate_tokens = 0;
    const char *filename = NULL;
    
    // Procesar argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            generate_tokens = 1;
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
    } else {
        return run_lexical_analysis(filename);
    }
}
