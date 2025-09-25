/**
 * @file keywords.c
 * @brief Implementación de la verificación de palabras clave
 * 
 * Contiene la implementación de la función para verificar si un lexema es una palabra clave.
 */

#include "keywords.h"
#include <string.h>

/**
 * @brief Lista de palabras clave del lenguaje
 * 
 * La lista debe coincidir con las definidas en el lexer.
 */
static const char *keywords[] = {
    "fn",
    "let",
    "mut",
    "if",
    "else",
    "match",
    "while",
    "loop",
    "for",
    "in",
    "break",
    "continue",
    "return",
    "true",
    "false",
    NULL
};

/**
 * @brief Verifica si un lexema es una palabra clave.
 * 
 * @param lexeme El lexema a verificar.
 * @return true si es una palabra clave, false en caso contrario.
 */
bool is_keyword(const char *lexeme) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(lexeme,keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Obtiene el índice de una palabra clave en el array.
 * 
 * @param lexeme La palabra a buscar.
 * @return El índice de la palabra clave, o -1 si no es una palabra clave.
 */
int get_keyword_index(const char *lexeme) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(lexeme, keywords[i]) == 0) {
            return i;
        }
    }
    return -1;
}