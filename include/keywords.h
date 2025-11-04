/**
 * @file keywords.h
 * @brief Declaración de la función is_keyword
 * 
 * Contiene utilidades para verificar si un lexema es una palabra clave y obtener su índice
 * dentro de la tabla de palabras reservadas. El orden es importante y debe coincidir con el
 * mapeo definido en el lexer.
 */

#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <stdbool.h>

bool is_keyword(const char *lexeme);
int get_keyword_index(const char *lexeme);

#endif