/**
 * @file lexer.c
 * @brief Implementación de estructuras y funciones del lexer
 */
#include "lexer.h"
#include "keywords.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NUM_STATES 31
#define NUM_CHAR_TYPES 24

/*
 * @brief Definición de los estados del autómata
 */
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

/*
 * @brief Tabla de operadores de dos caracteres
 */
static const char *two_char_operators[] = {
    "==",  
    "!=",  
    "<=",  
    ">=",  
    "&&",  
    "||",  
    "++", 
    "--",  
    "+=",  
    "-=", 
    "*=", 
    "/=",  
    "%=",  
    NULL   
};


/*
 * @brief Definición de los tipos de caracteres
 */
typedef enum CharType{
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

/* Utilidad opcional para depuración */
static const char* char_type_to_string(CharType type) {
    static const char *names[] = {
        [CHAR_LETTER]       = "CHAR_LETTER",
        [CHAR_DIGIT]        = "CHAR_DIGIT",
        [CHAR_UNDERSCORE]   = "CHAR_UNDERSCORE",
        [CHAR_QUOTE]        = "CHAR_QUOTE",
        [CHAR_APOSTROPHE]   = "CHAR_APOSTROPHE",
        [CHAR_BACKSLASH]    = "CHAR_BACKSLASH",
        [CHAR_PLUS]         = "CHAR_PLUS",
        [CHAR_MINUS]        = "CHAR_MINUS",
        [CHAR_STAR]         = "CHAR_STAR",
        [CHAR_SLASH]        = "CHAR_SLASH",
        [CHAR_PERCENT]      = "CHAR_PERCENT",
        [CHAR_EQUAL]        = "CHAR_EQUAL",
        [CHAR_EXCLAMATION]  = "CHAR_EXCLAMATION",
        [CHAR_AMPERSAND]    = "CHAR_AMPERSAND",
        [CHAR_PIPE]         = "CHAR_PIPE",
        [CHAR_LT]           = "CHAR_LT",
        [CHAR_GT]           = "CHAR_GT",
        [CHAR_HEXLETTER]    = "CHAR_HEXLETTER",
        [CHAR_DOT]          = "CHAR_DOT",
        [CHAR_DELIMITER]    = "CHAR_DELIMITER",
        [CHAR_WHITESPACE]   = "CHAR_WHITESPACE",
        [CHAR_NEWLINE]      = "CHAR_NEWLINE",
        [CHAR_EOF]          = "CHAR_EOF",
        [CHAR_UNKNOWN]      = "CHAR_UNKNOWN"
    };
    if (type >= 0 && type <= CHAR_UNKNOWN) return names[type];
    return "CHAR_INVALID";
}

/* Clasificación de caracteres: importante reconocer '\0' como EOF */
static CharType get_char_type(int c) {
    if (c == '\0') { return CHAR_EOF; }
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) { return CHAR_LETTER; }
    if (c >= '0' && c <= '9') { return CHAR_DIGIT; }
    if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) { return CHAR_HEXLETTER; }
    if (c == '_') { return CHAR_UNDERSCORE; } 
    if (c == '"') { return CHAR_QUOTE; }
    if (c == '\'') { return CHAR_APOSTROPHE; }
    if (c == '\\') { return CHAR_BACKSLASH; }
    if (c == '+') { return CHAR_PLUS; }
    if (c == '-') { return CHAR_MINUS; }
    if (c == '*') { return CHAR_STAR; }
    if (c == '/') { return CHAR_SLASH; }
    if (c == '%') { return CHAR_PERCENT; }
    if (c == '=') { return CHAR_EQUAL; }
    if (c == '!') { return CHAR_EXCLAMATION; }
    if (c == '&') { return CHAR_AMPERSAND; }
    if (c == '|') { return CHAR_PIPE; }
    if (c == '<') { return CHAR_LT; }
    if (c == '>') { return CHAR_GT; }
    if (c == '.') { return CHAR_DOT; }
    if (c == ';' || c == ',' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']') { return CHAR_DELIMITER; }
    if (c == ' ' || c == '\t') { return CHAR_WHITESPACE; }
    if (c == '\n') { return CHAR_NEWLINE; }
    return CHAR_UNKNOWN;
}

token_t *create_token(TokenType type, const char *lexeme,size_t line, size_t column) {
    token_t *new_token = (token_t *) malloc(sizeof(token_t));
    if (new_token == NULL) {
        printf("Error: No se pudo agregar un nuevo elemento por falta de memoria.\n");
        return NULL;
    }
    new_token->type = type;
    new_token->line = line;
    new_token->column = column;
    new_token->next = NULL;
    if (lexeme != NULL) {
        size_t n = strlen(lexeme);
        new_token->lexeme = (char*)malloc(n + 1);
        if (new_token->lexeme) {
            memcpy(new_token->lexeme, lexeme, n + 1);
        } else {
            printf("Error: No se pudo reservar memoria para el lexema del token.\n");
        }
    } else {
        new_token->lexeme = NULL;
    }
    return new_token;
}

void free_token(token_t *token) {
    if (token != NULL) {
        free(token->lexeme);
        free(token);
    }
}

/**
 * @brief Lee el contenido de un archivo y lo devuelve como una cadena
 */
char *read_file(const char *filename){
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Ocurrió un error al abrir el archivo '%s' o no existe.\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = (char*)malloc((size_t)size + 1);
    if (buffer == NULL) {
        printf("Error al asignar memoria.\n");
        fclose(file);
        return NULL;
    }
    size_t readn = fread(buffer, 1, (size_t)size, file);
    buffer[readn] = '\0';
    fclose(file);
    return buffer;
}


static inline char lxr_peek(const Lexer *lxr) {
    return *(lxr->p);
}

static inline char lxr_peek_next(const Lexer *lxr) {
    if (lxr->p[0] != '\0') {
        return lxr->p[1];
    }
    return '\0';
}

static inline void lxr_advance(Lexer *lxr) {
    char c = *(lxr->p);
    if (c == '\0')
        return;
    lxr->p++;
    if (c == '\n') {
        lxr->line++;
        lxr->col = 1;
    } else {
        lxr->col++;
    }
}


static char* make_lexeme(const char *start, const char *end) {
    size_t length = (size_t)(end - start);
    char *s = (char *) malloc(length + 1);
    if (s == NULL) {
        printf("Error: No se pudo reservar memoria para el lexema.\n");
        return NULL;
    }
    memcpy(s, start, length);
    s[length] = '\0';
    return s;
}


static void skip_ignorable(Lexer *lxr){
    for (;;) {
        CharType type = get_char_type(lxr_peek(lxr));
        if (type == CHAR_WHITESPACE || type == CHAR_NEWLINE) {
            lxr_advance(lxr);
            continue;
        }
        if (type == CHAR_SLASH) {
            char next = lxr_peek_next(lxr);
            if (next == '/') {
                
                lxr_advance(lxr); 
                lxr_advance(lxr);
                while (lxr_peek(lxr) != '\0' && lxr_peek(lxr) != '\n') {
                    lxr_advance(lxr);
                }
                continue;
            } else if (next == '*') {
                lxr_advance(lxr);
                lxr_advance(lxr);
                while (lxr_peek(lxr) != '\0') {
                    if (lxr_peek(lxr) == '*' && lxr_peek_next(lxr) == '/') {
                        lxr_advance(lxr);
                        lxr_advance(lxr);
                        break;
                    }
                    lxr_advance(lxr);
                }
                continue;
            }
        }
        break;
    }
}


static token_t* lex_identifier_or_keyword(Lexer *lxr, size_t sl, size_t sc){
    const char *start = lxr->p;
    lxr_advance(lxr); 
    while (1) {
        CharType type = get_char_type(lxr_peek(lxr));
        if (type == CHAR_LETTER || type == CHAR_DIGIT || type == CHAR_UNDERSCORE) {
            lxr_advance(lxr);
        } else {
            break;
        }
    }
    char *lexeme = make_lexeme(start, lxr->p);
    if (lexeme == NULL) {
        return create_token(TOKEN_UNKNOWN, NULL, sl, sc);
    }
    TokenType ttype = is_keyword(lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    token_t *token = create_token(ttype, lexeme, sl, sc);
    free(lexeme);
    return token;
}

static int is_hex_digit(char c) {
    if (c >= '0' && c <= '9') {
        return 1;
    } else if (c >= 'a' && c <= 'f') {
        return 1;
    } else if (c >= 'A' && c <= 'F') {
        return 1;
    }
    return 0;
}

static int is_bin_digit(char c) {
    return c == '0' || c == '1';
}


static token_t* lex_number(Lexer *lxr, size_t sl, size_t sc){
    const char  *start  = lxr->p;

    if (lxr_peek(lxr) == '0' && (lxr_peek_next(lxr) == 'x' || lxr_peek_next(lxr) == 'X')) {
        lxr_advance(lxr); 
        lxr_advance(lxr); 
        int have = 0;
        while (is_hex_digit(lxr_peek(lxr))) {
            have = 1;
            lxr_advance(lxr);
        }
        char *lexeme = make_lexeme(start, lxr->p);
        token_t *tok = create_token(have ? TOKEN_NUMBER : TOKEN_UNKNOWN, lexeme, sl, sc);
        free(lexeme);
        return tok;
    }

    if (lxr_peek(lxr) == '0' && (lxr_peek_next(lxr) == 'b' || lxr_peek_next(lxr) == 'B')) {
        lxr_advance(lxr);
        lxr_advance(lxr); 
        int have = 0;
        while (is_bin_digit(lxr_peek(lxr))) {
            have = 1;
            lxr_advance(lxr);
        }
        char *lexeme = make_lexeme(start, lxr->p);
        token_t *tok = create_token(have ? TOKEN_NUMBER : TOKEN_UNKNOWN, lexeme, sl, sc);
        free(lexeme);
        return tok;
    }

    while (get_char_type(lxr_peek(lxr)) == CHAR_DIGIT) {
        lxr_advance(lxr);
    }

    if (get_char_type(lxr_peek(lxr)) == CHAR_DOT && get_char_type(lxr_peek_next(lxr)) == CHAR_DIGIT) {
        lxr_advance(lxr); // '.'
        while (get_char_type(lxr_peek(lxr)) == CHAR_DIGIT) {
            lxr_advance(lxr);
        }
    }

    if (lxr_peek(lxr) == 'e' || lxr_peek(lxr) == 'E') {
        const char *save_p = lxr->p;
        size_t save_line = lxr->line;
        size_t save_col  = lxr->col;

        lxr_advance(lxr); //
        if (lxr_peek(lxr) == '+' || lxr_peek(lxr) == '-') {
            lxr_advance(lxr);
        }
        if (get_char_type(lxr_peek(lxr)) == CHAR_DIGIT) {
            while (get_char_type(lxr_peek(lxr)) == CHAR_DIGIT) {
                lxr_advance(lxr);
            }
        } else {
            lxr->p = save_p;
            lxr->line = save_line;
            lxr->col = save_col;
        }
    }

    char *lexeme = make_lexeme(start, lxr->p);
    token_t *token_num = create_token(TOKEN_NUMBER, lexeme, sl, sc);
    free(lexeme);
    return token_num;
}

static token_t* lex_string(Lexer *lxr, size_t sl, size_t sc){
    const char *start = lxr->p;
    lxr_advance(lxr); 
    int ok = 0;
    while (lxr_peek(lxr) != '\0') {
        char c = lxr_peek(lxr);
        if (c == '\\') {
            lxr_advance(lxr);
            if (lxr_peek(lxr) != '\0') {
                lxr_advance(lxr);
            }
        } else if (c == '"') {
            lxr_advance(lxr);
            ok = 1;
            break;
        } else {
            lxr_advance(lxr);
        }
    }
    char *lexeme = make_lexeme(start, lxr->p);
    token_t *token_str = create_token(ok ? TOKEN_STRING : TOKEN_UNKNOWN, lexeme, sl, sc);
    free(lexeme);
    return token_str;
}


static int lxr_match2(const Lexer *lxr, const char *op) {
    // Verificar que tenemos un operador válido de exactamente 2 caracteres
    if (!op || strlen(op) != 2) {
        return 0;
    }  
    char current_char = lxr_peek(lxr);
    char next_char = lxr_peek_next(lxr);
    
    if (current_char == op[0] && next_char == op[1]) {
        return 1; 
    }
    
    return 0; 
}


static token_t* lex_operator_or_delimiter(Lexer *lxr, size_t sl, size_t sc){
    const char *start = lxr->p;

    for (const char **op = two_char_operators; *op; ++op) {
        if (lxr_match2(lxr, *op)) {
            lxr_advance(lxr);
            lxr_advance(lxr);
            char *lex = make_lexeme(start, lxr->p);
            token_t *tok = create_token(TOKEN_OPERATOR, lex, sl, sc);
            free(lex);
            return tok;
        }
    }

    // Delimitadores de un carácter
    char c = lxr_peek(lxr);
    if (strchr(";,(){}[]", c)) {
        lxr_advance(lxr);
        char *lex = make_lexeme(start, lxr->p);
        token_t *tok = create_token(TOKEN_DELIMITER, lex, sl, sc);
        free(lex);
        return tok;
    }

    // Operadores de un carácter 
    if (strchr("+-*/%=!<>&|.", c)) {
        lxr_advance(lxr);
        char *lex = make_lexeme(start, lxr->p);
        token_t *tok = create_token(TOKEN_OPERATOR, lex, sl, sc);
        free(lex);
        return tok;
    }

    // Carácter desconocido
    lxr_advance(lxr);
    char *lex = make_lexeme(start, lxr->p);
    token_t *tok = create_token(TOKEN_UNKNOWN, lex, sl, sc);
    free(lex);
    return tok;
}


void lexer_init(Lexer *lxr, const char *source){
    lxr->source = source ? source : "";
    lxr->p = lxr->source;
    lxr->line = 1;
    lxr->col = 1;
}

token_t* lexer_next_token(Lexer *lxr){
    if (!lxr || !lxr->p) {
        return NULL;
    }
    skip_ignorable(lxr);

    size_t start_line = lxr->line;
    size_t start_col = lxr->col;
    CharType type = get_char_type(lxr_peek(lxr));

    if (type == CHAR_EOF) {
        return create_token(TOKEN_EOF, "EOF", start_line, start_col);
    }
    if (type == CHAR_LETTER || type == CHAR_UNDERSCORE) {
        return lex_identifier_or_keyword(lxr, start_line, start_col);
    }
    if (type == CHAR_DIGIT) {
        return lex_number(lxr, start_line, start_col);
    }
    if (type == CHAR_QUOTE) {
        return lex_string(lxr, start_line, start_col);
    }
    if (type == CHAR_APOSTROPHE) {
        const char *start = lxr->p;
        lxr_advance(lxr); // '
        if (lxr_peek(lxr) == '\\') {
            lxr_advance(lxr);
            if (lxr_peek(lxr) != '\0') lxr_advance(lxr);
        } else if (lxr_peek(lxr) != '\0' && lxr_peek(lxr) != '\'' && lxr_peek(lxr) != '\n') {
            lxr_advance(lxr);
        }
        if (lxr_peek(lxr) == '\'') lxr_advance(lxr);
        char *lex = make_lexeme(start, lxr->p);
        token_t *token = create_token(TOKEN_UNKNOWN, lex, start_line, start_col);
        free(lex);
        return token;
    }

    return lex_operator_or_delimiter(lxr, start_line, start_col);
}


token_t *get_next_token(const char *source){
    static Lexer lexer;
    static int initialized = 0;
    if (!initialized) {
        lexer_init(&lexer, source);
        initialized = 1;
    }
    token_t *token = lexer_next_token(&lexer);
    if (token && token->type == TOKEN_EOF) {
        initialized = 0; 
    }
    return token;
}


int main(void) {
    char *source = read_file("src/lexer/test.txt");
    if (source == NULL) {
        return 1;
    }
    Lexer lexer;
    lexer_init(&lexer, source);

    for (;;) {
        token_t *token = lexer_next_token(&lexer);
        if (!token) {
            printf("Error al obtener el siguiente token.\n");
            break;
        }
        printf("[%zu,%zu] Type: %d Lexeme: %s\n", token->line, token->column, token->type, token->lexeme ? token->lexeme : "NULL");
        if (token->type == TOKEN_EOF) {
            free_token(token);
            break;
        }
        free_token(token);
    }
    free(source);
    return 0;
}