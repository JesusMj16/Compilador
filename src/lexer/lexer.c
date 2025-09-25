/**
 * @file lexer.c
 * @brief Implementación de estructuras y funciones del lexer
 */
#include "../include/lexer.h"
#include "../include/keywords.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NUM_STATES 31
#define NUM_CHAR_TYPES 24

/**
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

/**
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


/**
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

#ifdef LEXER_DEBUG
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
#endif

/**
 * @brief Clasifica un carácter y devuelve su tipo correspondiente.
 * 
 * @param c El carácter a clasificar.
 * @return El tipo de carácter (CharType) correspondiente.
 */
static CharType get_char_type(int c) {
    if (c == '\0') { return CHAR_EOF; }
    if (c >= '0' && c <= '9') { return CHAR_DIGIT; }
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) { return CHAR_LETTER; }
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

/**
 * @brief Crea un nuevo token con los parámetros especificados.
 * 
 * @param type El tipo de token.
 * @param lexeme El lexema del token.
 * @param line El número de línea donde se encontró el token.
 * @param column El número de columna donde se encontró el token.
 * @return Un puntero al nuevo token creado, o NULL si hay error de memoria.
 */
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

/**
 * @brief Libera la memoria ocupada por un token.
 * 
 * @param token El token a liberar.
 */
void free_token(token_t *token) {
    if (token != NULL) {
        free(token->lexeme);
        free(token);
    }
}

/**
 * @brief Libera una lista enlazada completa de tokens.
 * 
 * @param head El primer token de la lista.
 */
void free_token_list(token_t *head) {
    token_t *current = head;
    while (current != NULL) {
        token_t *next = current->next;
        free_token(current);
        current = next;
    }
}

/**
 * @brief Lee el contenido de un archivo y lo devuelve como una cadena.
 * 
 * @param filename El nombre del archivo a leer.
 * @return El contenido del archivo como cadena, o NULL si hay error.
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

/**
 * @brief Obtiene el carácter actual sin avanzar el puntero del lexer.
 * 
 * @param lxr El lexer.
 * @return El carácter actual.
 */
static inline char lxr_peek(const Lexer *lxr) {
    return *(lxr->p);
}

/**
 * @brief Obtiene el siguiente carácter sin avanzar el puntero del lexer.
 * 
 * @param lxr El lexer.
 * @return El siguiente carácter.
 */
static inline char lxr_peek_next(const Lexer *lxr) {
    if (lxr->p[0] != '\0') {
        return lxr->p[1];
    }
    return '\0';
}

/**
 * @brief Avanza el puntero del lexer al siguiente carácter y actualiza posición.
 * 
 * @param lxr El lexer.
 */
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

/**
 * @brief Crea un lexema a partir de un rango de caracteres.
 * 
 * @param start Puntero al inicio del lexema.
 * @param end Puntero al final del lexema.
 * @return Una copia del lexema como cadena, o NULL si hay error de memoria.
 */
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

/**
 * @brief Omite caracteres ignorables como espacios en blanco y comentarios.
 * 
 * @param lxr El lexer.
 */
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

/**
 * @brief Analiza y crea un token para identificadores o palabras clave.
 * 
 * Reglas según reglas.md:
 * - EBNF: identificador -> ( letra | '_' ) ( letra | dígito | '_' )*
 * - REGEX: [_a-zA-Z][_a-zA-Z0-9]*
 * - Puede empezar con letra (a-z, A-Z) o guión bajo (_)
 * - Puede contener letras, dígitos (0-9) y guiones bajos (_)
 * - Permite múltiples guiones bajos consecutivos
 * 
 * @param lxr El lexer.
 * @param sl Línea de inicio del token.
 * @param sc Columna de inicio del token.
 * @return El token creado (TOKEN_IDENTIFIER o TOKEN_KEYWORD).
 */
static token_t* lex_identifier_or_keyword(Lexer *lxr, size_t sl, size_t sc){
    const char *start = lxr->p;
    
    // El primer carácter debe ser una letra o guión bajo
    CharType first_type = get_char_type(lxr_peek(lxr));
    if (first_type != CHAR_LETTER && first_type != CHAR_UNDERSCORE) {
        lxr_advance(lxr);
        char *lexeme = make_lexeme(start, lxr->p);
        token_t *token = create_token(TOKEN_UNKNOWN, lexeme, sl, sc);
        if (lexeme) free(lexeme);
        return token;
    }
    
    // Consumir el primer carácter
    lxr_advance(lxr);
    
    // Continuar con letras, dígitos o guiones bajos
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

/**
 * @brief Verifica si un carácter es un dígito hexadecimal.
 * 
 * @param c El carácter a verificar.
 * @return 1 si es un dígito hexadecimal, 0 en caso contrario.
 */
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

/**
 * @brief Verifica si un carácter es un dígito binario.
 * 
 * @param c El carácter a verificar.
 * @return 1 si es un dígito binario (0 o 1), 0 en caso contrario.
 */
static int is_bin_digit(char c) {
    return c == '0' || c == '1';
}

/**
 * @brief Analiza y crea un token para números (enteros, reales, hexadecimales, binarios).
 * 
 * @param lxr El lexer.
 * @param sl Línea de inicio del token.
 * @param sc Columna de inicio del token.
 * @return El token creado (TOKEN_NUMBER o TOKEN_UNKNOWN).
 */
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

    // Números binarios (0b o 0B)
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

/**
 * @brief Analiza y crea un token para cadenas de texto.
 * 
 * @param lxr El lexer.
 * @param sl Línea de inicio del token.
 * @param sc Columna de inicio del token.
 * @return El token creado (TOKEN_STRING o TOKEN_UNKNOWN).
 */
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

/**
 * @brief Verifica si los siguientes dos caracteres coinciden con un operador específico.
 * 
 * @param lxr El lexer.
 * @param op El operador de dos caracteres a verificar.
 * @return 1 si coincide, 0 en caso contrario.
 */
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

/**
 * @brief Analiza y crea un token para operadores o delimitadores.
 * 
 * @param lxr El lexer.
 * @param sl Línea de inicio del token.
 * @param sc Columna de inicio del token.
 * @return El token creado (TOKEN_OPERATOR, TOKEN_DELIMITER o TOKEN_UNKNOWN).
 */
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

/**
 * @brief Inicializa un lexer con el código fuente dado.
 * 
 * @param lxr El lexer a inicializar.
 * @param source El código fuente a analizar.
 */
void lexer_init(Lexer *lxr, const char *source){
    lxr->source = source ? source : "";
    lxr->p = lxr->source;
    lxr->line = 1;
    lxr->col = 1;
}

/**
 * @brief Obtiene el siguiente token del código fuente.
 * 
 * @param lxr El lexer.
 * @return El siguiente token encontrado, o NULL si hay error.
 */
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
        
        // Manejar caracter con escape
        if (lxr_peek(lxr) == '\\') {
            lxr_advance(lxr);
            if (lxr_peek(lxr) != '\0') lxr_advance(lxr);
        } else if (lxr_peek(lxr) != '\0' && lxr_peek(lxr) != '\'' && lxr_peek(lxr) != '\n') {
            // Caracter normal
            lxr_advance(lxr);
        }
        
        // Debe terminar con '
        if (lxr_peek(lxr) == '\'') {
            lxr_advance(lxr);
            char *lex = make_lexeme(start, lxr->p);
            token_t *token = create_token(TOKEN_STRING, lex, start_line, start_col);
            free(lex);
            return token;
        } else {
            // Caracter mal formado
            char *lex = make_lexeme(start, lxr->p);
            token_t *token = create_token(TOKEN_UNKNOWN, lex, start_line, start_col);
            free(lex);
            return token;
        }
    }

    return lex_operator_or_delimiter(lxr, start_line, start_col);
}

/**
 * @brief Función de conveniencia para obtener tokens usando un lexer estático.
 * 
 * @param source El código fuente a analizar.
 * @return El siguiente token, o NULL si hay error.
 */
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

/**
 * @brief Convierte un tipo de token a su representación en cadena.
 * 
 * @param t El tipo de token.
 * @return El nombre del tipo de token como cadena.
 */
const char* token_type_name(TokenType t) {
    switch (t) {
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_NUMBER:     return "NUMBER";
        case TOKEN_STRING:     return "STRING";
        case TOKEN_OPERATOR:   return "OPERATOR";
        case TOKEN_DELIMITER:  return "DELIMITER";
        case TOKEN_KEYWORD:    return "KEYWORD";
        case TOKEN_UNKNOWN:    return "UNKNOWN";
        case TOKEN_EOF:        return "EOF";
        default:               return "INVALID";
    }
}

/**
 * @brief Tokeniza todo el código fuente y devuelve una lista enlazada de tokens.
 * 
 * @param source El código fuente a tokenizar.
 * @return El primer token de la lista enlazada, o NULL si hay error.
 */
token_t *tokenize_all(const char *source) {
    if (!source) return NULL;
    
    Lexer lexer;
    lexer_init(&lexer, source);
    
    token_t *head = NULL;
    token_t *tail = NULL;
    
    for (;;) {
        token_t *token = lexer_next_token(&lexer);
        if (!token) break;
        
        if (!head) {
            head = tail = token;
        } else {
            tail->next = token;
            tail = token;
        }
        
        if (token->type == TOKEN_EOF) break;
    }
    
    return head;
}

/**
 * @brief Escribe los tokens de un archivo fuente a un archivo de salida con formato numérico.
 * 
 * @param source_file El archivo fuente a tokenizar.
 * @param output_file El archivo donde escribir los tokens.
 * @return 0 si es exitoso, 1 si hay error.
 */
int write_tokens_to_file(const char *source_file, const char *output_file) {
    if (!source_file || !output_file) return 1;
    
    // Leer el archivo fuente
    char *source = read_file(source_file);
    if (!source) {
        printf("Error: No se pudo leer el archivo '%s'\n", source_file);
        return 1;
    }
    
    // Abrir archivo de salida
    FILE *output = fopen(output_file, "w");
    if (!output) {
        printf("Error: No se pudo crear el archivo '%s'\n", output_file);
        free(source);
        return 1;
    }
    
    // Escribir header con información del formato
    fprintf(output, "# Tokens generados desde: %s\n", source_file);
    fprintf(output, "# Formato: tipo_token lexema linea columna [indice_palabra_clave]\n");
    fprintf(output, "# Tipos: IDENTIFIER=0, NUMBER=1, STRING=2, OPERATOR=3, DELIMITER=4, KEYWORD=5, UNKNOWN=6, EOF=7\n");
    fprintf(output, "# Palabras clave: fn=0, let=1, mut=2, if=3, else=4, match=5, while=6, loop=7, for=8, in=9, break=10, continue=11, return=12, true=13, false=14\n");
    fprintf(output, "\n");
    
    Lexer lexer;
    lexer_init(&lexer, source);
    
    int token_count = 0;
    for (;;) {
        token_t *token = lexer_next_token(&lexer);
        if (!token) {
            fprintf(output, "# Error: No se pudo obtener el siguiente token\n");
            break;
        }
    
    // Escribir en formato: tipo lexema linea columna [indice_keyword]
        if (token->type == TOKEN_KEYWORD) {
            int keyword_index = get_keyword_index(token->lexeme);
            fprintf(output, "%d %s %zu %zu %d\n", 
                   token->type, 
                   token->lexeme ? token->lexeme : "NULL",
                   token->line, 
                   token->column,
                   keyword_index);
        } else {
            fprintf(output, "%d %s %zu %zu\n", 
                   token->type, 
                   token->lexeme ? token->lexeme : "NULL",
                   token->line, 
                   token->column);
        }
        
        token_count++;
        
        if (token->type == TOKEN_EOF) {
            free_token(token);
            break;
        }
        free_token(token);
    }
    
    fprintf(output, "\n# Total de tokens: %d\n", token_count);
    
    fclose(output);
    free(source);
    
    printf("✓ Tokens escritos en: %s (%d tokens)\n", output_file, token_count);
    return 0;
}