#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <stdbool.h>

bool is_keyword(const char *lexeme);
int get_keyword_index(const char *lexeme);

#endif