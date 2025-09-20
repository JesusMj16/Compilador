/*
 * @file lexer.c
 * @brief Implementación de estructuras y funciones del lexer
 * 
 * Contiene la implementación de las funciones para crear y liberar tokens,
 * así como la definición de la estructura token_t.
 */

#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*
*@brief Crea un nuevo token
*
* @param type Tipo de token
* @return Puntero al token creado
*/
token_t *create_token(TokenType type, const char *lexeme,size_t line, size_t column) {
    token_t *new_token = (token_t *) malloc(sizeof(token_t));
    if (new_token == NULL) {
        printf("Error: No se pudo agregar un nuevo elemento por falta de memoria.\n");
        return NULL;
    }
    
    new_token-> type = type;
    new_token-> line = line;
    new_token-> column = column;
    new_token-> next = NULL;

    if (lexeme != NULL) {
        new_token->lexeme = strdup(lexeme);
        if (new_token->lexeme == NULL) {
            printf("Error: No se pudo reservar memoria para el token.\n");
        }
        
    }
    
    return new_token;
}


/*
 * @brief Libera un token
 *
 * @param token Puntero al token a liberar
 * @return void
 */
void free_token(token_t *token) {
    if( token != NULL) {
        free(token ->lexeme);
        free(token);
    }
}


