/*
* @file lexer.h
* @brief Definición del enum TokenType
*
* Contiene la definición del enum TokenType que representa los diferentes tipos de tokens
* que el lexer puede identificar en el código fuente.
*/

#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>
#include <stdint.h>

/*
* @brief Definición del enum TokenType
*/
typedef enum TokenType {
    TOKEN_IDENTIFIER,  /**> Identificador */
    TOKEN_NUMBER,      /**> Número */
    TOKEN_STRING,     /**> Cadena de texto */
    TOKEN_OPERATOR,   /**> Operador */
    TOKEN_DELIMITER,  /**> Delimitador */
    TOKEN_KEYWORD,    /**> Palabra clave */
    TOKEN_EOF,       /**> Fin de archivo */
    TOKEN_UNKNOWN    /**> Token desconocido */
} TokenType;


typedef struct {
    TokenType type; /**> Tipo de token */
    char *lexeme;   /**> Lexema del token */
    size_t line;    /**> Línea donde se encontró el token */
    size_t column;  /**> Columna donde se encontró el token */
    struct token_t *next; /**> Puntero al siguiente token (para lista enlazada) */
} token_t;

#endif // LEXER_H