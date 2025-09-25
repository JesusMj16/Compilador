#include "keywords.h"
#include <string.h>

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