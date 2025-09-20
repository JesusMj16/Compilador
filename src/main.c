/**
 * @file main.c
 * @brief Punto de entrada del compilador
 * 
 * Este archivo contiene la función principal del compilador que inicializa el lexer,
 * procesa el archivo fuente y maneja los tokens generados.
 */

#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_fuente>\n", argv[0]);
        return 1;
    }

    const char *source_file = argv[1];
    FILE *file = fopen(source_file, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Aquí se inicializaría el lexer y se procesaría el archivo
    // Por ejemplo:
    // lexer_t *lexer = lexer_create(file);
    // token_t *token;
    // do {
    //     token = lexer_next_token(lexer);
    //     // Procesar el token
    // } while (token->type != TOKEN_EOF);
    // lexer_destroy(lexer);

    fclose(file);
    return 0;
}