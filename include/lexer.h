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
    TOKEN_IDENTIFIER,   /**< Identificador */
    TOKEN_NUMBER,       /**< Literal numérico */
    TOKEN_STRING,       /**< Literal de cadena */
    TOKEN_CHAR,         /**< Literal de carácter */
    /* Palabras reservadas */
    TOKEN_KW_FN,
    TOKEN_KW_LET,
    TOKEN_KW_MUT,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_MATCH,
    TOKEN_KW_WHILE,
    TOKEN_KW_LOOP,
    TOKEN_KW_FOR,
    TOKEN_KW_IN,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_KW_RETURN,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,
    TOKEN_KW_I32,
    TOKEN_KW_F64,
    TOKEN_KW_BOOL,
    TOKEN_KW_CHAR,
    /* Operadores */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_AND_AND,
    TOKEN_OR_OR,
    TOKEN_PLUS_EQUAL,
    TOKEN_MINUS_EQUAL,
    TOKEN_STAR_EQUAL,
    TOKEN_SLASH_EQUAL,
    TOKEN_PERCENT_EQUAL,
    TOKEN_PLUS_PLUS,
    TOKEN_MINUS_MINUS,
    TOKEN_ARROW,
    /* Puntuación y delimitadores */
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    /* Misceláneos */
    TOKEN_UNKNOWN,
    TOKEN_EOF
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