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

#define NUM_STATES 15
#define NUM_CHAR_TYPES 10

typedef enum {
    STATE_START,
    STATE_IDENTIFIER,
    STATE_INT,
    STATE_SIGN,
    STATE_BIN_PREFIX,
    STATE_BIN,
    STATE_HEX_PREFIX,
    STATE_HEX,
    STATE_REAL,
    STATE_REAL_FRACTION,
    STATE_EXPONENT_MARK,
    STATE_EXPONENT_SIGN,
    STATE_EXPONENT,
    STATE_STRING,
    STATE_STRING_ESCAPE,
    STATE_CHAR,
    STATE_CHAR_ESCAPE,
    STATE_CHAR_END,
    STATE_SLASH,
    STATE_COMMENT_LINE,
    STATE_COMMENT_BLOCK,
    STATE_COMMENT_BLOCK_END,
    STATE_OPERATOR,
    STATE_OPERATOR_EQ, 
    STATE_OPERATOR_AND,
    STATE_OPERATOR_OR,
    STATE_DELIMITER,
    STATE_WHITESPACE,
    STATE_FINAL,
    STATE_ERROR,
    STATE_EOF
} State;



typedef enum {
    CHAR_LETTER,       
    CHAR_DIGIT,        
    CHAR_UNDERSCORE,   
    CHAR_QUOTE,        
    CHAR_APOSTROPHE,  
    CHAR_BACKSLASH,    
    CHAR_PLUS,         
    CHAR_MINUS,        
    CHAR_STAR,         
    CHAR_SLASH,        
    CHAR_PERCENT,      
    CHAR_EQUAL,        
    CHAR_EXCLAMATION,  
    CHAR_AMPERSAND,    
    CHAR_PIPE,         
    CHAR_LT,           
    CHAR_GT,           
    CHAR_HEXLETTER,    
    CHAR_DOT,          
    CHAR_DELIMITER,    
    CHAR_WHITESPACE,   
    CHAR_NEWLINE,       
    CHAR_EOF,
    CHAR_UNKNOWN
} CharType;



CharType get_char_type(int c) {
    if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) { return CHAR_HEXLETTER;}
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) { return CHAR_LETTER; }
    if (c >= '0' && c <= '9') { return CHAR_DIGIT;}
    if (c == '_') { return CHAR_UNDERSCORE;} 
    if (c == '"') { return CHAR_QUOTE;}
    if (c == '\'') { return CHAR_APOSTROPHE;}
    if (c == '\\') { return CHAR_BACKSLASH;}
    if (c == '+') { return CHAR_PLUS;}
    if (c == '-') { return CHAR_MINUS;}
    if (c == '*') { return CHAR_STAR;}
    if (c == '/') { return CHAR_SLASH;}
    if (c == '%') { return CHAR_PERCENT;}
    if (c == '=') { return CHAR_EQUAL;}
    if (c == '!') { return CHAR_EXCLAMATION;}
    if (c == '&') { return CHAR_AMPERSAND;}
    if (c == '|') { return CHAR_PIPE;}
    if (c == '<') { return CHAR_LT;}
    if (c == '>') { return CHAR_GT;}
    if (c == '.') { return CHAR_DOT;}
    if (c == ';' || c == ',' || c == '(' || c == ')'|| c == '{' || c == '}' || c == '[' || c == ']') { return CHAR_DELIMITER;}
    if (c == ' ' || c == '\t') { return CHAR_WHITESPACE;}
    if (c == '\n') { return CHAR_NEWLINE;}
    if (c == EOF) { return CHAR_EOF;}
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

