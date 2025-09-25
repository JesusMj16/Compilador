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
    TOKEN_IDENTIFIER,  /**< Identificador */
    TOKEN_NUMBER,      /**< Número */
    TOKEN_STRING,      /**< Cadena de texto */
    TOKEN_OPERATOR,    /**< Operador */
    TOKEN_DELIMITER,   /**< Delimitador */
    TOKEN_KEYWORD,     /**< Palabra clave */
    TOKEN_UNKNOWN,      /**< Token desconocido */
    TOKEN_EOF         /**< Fin de archivo */
} TokenType;

/*
* @brief Definición de la estructura Token
*/
typedef struct token_t{
    TokenType type;       /**< Tipo de token */
    char *lexeme;         /**< Lexema del token */
    size_t line;          /**< Línea donde se encontró el token */
    size_t column;        /**< Columna donde se encontró el token */
    struct token_t *next; /**< Puntero al siguiente token (para lista enlazada) */
} token_t;

/*
* @brief Estructura del lexer
*/
typedef struct Lexer {
    const char *source;   /**< Código fuente a analizar */
    const char *p;        /**< Puntero actual en el código fuente */
    size_t line;          /**< Línea actual */
    size_t col;           /**< Columna actual */
} Lexer;

token_t *create_token(TokenType type, const char *lexeme,size_t line, size_t column);
void free_token(token_t *token);
void free_token_list(token_t *head);
void lexer_init(Lexer *lxr, const char *source);
token_t* lexer_next_token(Lexer *lxr);
char *read_file(const char *filename);
token_t *get_next_token(const char *source);
token_t *tokenize_all(const char *source);
const char* token_type_name(TokenType t);
int write_tokens_to_file(const char *source_file, const char *output_file);

#endif // LEXER_H