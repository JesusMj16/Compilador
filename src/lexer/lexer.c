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

#define NUM_STATES 11
#define NUM_CHAR_TYPES 9

typedef enum {
    STATE_START,
    STATE_IDENTIFIER,
    STATE_NUMBER,
    STATE_STRING,
    STATE_OPERATOR,
    STATE_DELIMITER,
    STATE_COMMENT,
    STATE_WHITESPACE,
    STATE_FINAL,
    STATE_ERROR,
    STATE_EOF
}State;

typedef enum {
    CHAR_LETTER,
    CHAR_DIGIT,
    CHAR_QUOTE,
    CHAR_OPERATOR,
    CHAR_DELIMITER,
    CHAR_WHITESPACE,
    CHAR_NEWLINE,
    CHAR_EOF,
    CHAR_UNKNOWN
}CharType;

CharType get_char_type(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) { return CHAR_LETTER; }
    if (c >= 0 && c <= '9') { return CHAR_DIGIT; }
    if (c == '"') { return CHAR_QUOTE; }
    if (c == '+' && c == '-' && c == '*' && c == '/' && c == '=' && c == '<' && c == '>') { return CHAR_OPERATOR; }
    if (c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ';' || c == '[' || c == ']') { return CHAR_DELIMITER; }
    if (c == ' ' || c == '\t' ) { return CHAR_WHITESPACE; }
    if (c == '\n') { return CHAR_NEWLINE; }
    if (c == '\0') { return CHAR_EOF; }
    return CHAR_UNKNOWN;
}


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

token_t *get_next_token(const char *source){
    
}

