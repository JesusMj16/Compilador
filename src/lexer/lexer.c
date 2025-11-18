/**
 * @file lexer.c
 * @brief Implementación de estructuras y funciones del lexer
 */
#include "../../include/lexer.h"
#include "../../include/keywords.h"
#include "../../include/symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Especificación de tokens de varios caracteres (principalmente operadores)
 */
typedef struct {
    const char *lexeme;
    TokenType type;
} MultiCharToken;

static inline void lxr_advance(Lexer *lxr);
static inline char lxr_peek(const Lexer *lxr);
static inline char lxr_peek_next(const Lexer *lxr);

static const MultiCharToken multi_char_tokens[] = {
    {"==", TOKEN_EQUAL_EQUAL},
    {"!=", TOKEN_BANG_EQUAL},
    {"<=", TOKEN_LESS_EQUAL},
    {">=", TOKEN_GREATER_EQUAL},
    {"&&", TOKEN_AND_AND},
    {"||", TOKEN_OR_OR},
    {NULL, TOKEN_UNKNOWN}
};

/**
 * @brief Traduce el índice de palabra reservada a su TokenType concreto.
 */
static TokenType keyword_token_from_index(int index) {
    static const TokenType map[] = {
        TOKEN_KW_FN,
        TOKEN_KW_LET,
        TOKEN_KW_MUT,
        TOKEN_KW_RETURN,
        TOKEN_KW_TRUE,
        TOKEN_KW_FALSE,
        TOKEN_KW_I32,
        TOKEN_KW_F64,
        TOKEN_KW_BOOL
    };

    if (index < 0) {
        return TOKEN_IDENTIFIER;
    }
    size_t map_size = sizeof(map) / sizeof(map[0]);
    if ((size_t)index >= map_size) {
        return TOKEN_IDENTIFIER;
    }
    return map[index];
}

typedef enum CharType {
    CHAR_LETTER,
    CHAR_DIGIT,
    CHAR_UNDERSCORE,
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
    CHAR_DOT,
    CHAR_DELIMITER,
    CHAR_WHITESPACE,
    CHAR_NEWLINE,
    CHAR_EOF,
    CHAR_UNKNOWN
} CharType;

#define AUT_ACCEPT_BASE 100

typedef enum AutomatonState {
    AUTO_START = 0,
    AUTO_WHITESPACE,
    AUTO_SLASH,
    AUTO_STATE_COUNT,
    AUTO_ACCEPT_IDENTIFIER = AUT_ACCEPT_BASE,
    AUTO_ACCEPT_NUMBER,
    AUTO_ACCEPT_OPERATOR,
    AUTO_ACCEPT_DELIMITER,
    AUTO_ACCEPT_WHITESPACE,
    AUTO_ACCEPT_COMMENT_LINE,
    AUTO_ACCEPT_COMMENT_BLOCK,
    AUTO_ACCEPT_SLASH,
    AUTO_ACCEPT_EOF,
    AUTO_ERROR_STATE
} AutomatonState;

#define CHAR_TYPE_COUNT (CHAR_UNKNOWN + 1)

static const AutomatonState transition_table[AUTO_STATE_COUNT][CHAR_TYPE_COUNT] = {
    [AUTO_START] = {
        [CHAR_LETTER] = AUTO_ACCEPT_IDENTIFIER,
        [CHAR_DIGIT] = AUTO_ACCEPT_NUMBER,
        [CHAR_UNDERSCORE] = AUTO_ACCEPT_IDENTIFIER,
        [CHAR_PLUS] = AUTO_ACCEPT_OPERATOR,
        [CHAR_MINUS] = AUTO_ACCEPT_OPERATOR,
        [CHAR_STAR] = AUTO_ACCEPT_OPERATOR,
        [CHAR_SLASH] = AUTO_SLASH,
        [CHAR_PERCENT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_EQUAL] = AUTO_ACCEPT_OPERATOR,
        [CHAR_EXCLAMATION] = AUTO_ACCEPT_OPERATOR,
        [CHAR_AMPERSAND] = AUTO_ACCEPT_OPERATOR,
        [CHAR_PIPE] = AUTO_ACCEPT_OPERATOR,
        [CHAR_LT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_GT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_DOT] = AUTO_ERROR_STATE,
        [CHAR_DELIMITER] = AUTO_ACCEPT_DELIMITER,
        [CHAR_WHITESPACE] = AUTO_WHITESPACE,
        [CHAR_NEWLINE] = AUTO_WHITESPACE,
        [CHAR_EOF] = AUTO_ACCEPT_EOF,
        [CHAR_UNKNOWN] = AUTO_ERROR_STATE
    },
    [AUTO_WHITESPACE] = {
        [CHAR_LETTER] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_DIGIT] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_UNDERSCORE] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_PLUS] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_MINUS] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_STAR] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_SLASH] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_PERCENT] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_EQUAL] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_EXCLAMATION] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_AMPERSAND] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_PIPE] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_LT] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_GT] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_DOT] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_DELIMITER] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_WHITESPACE] = AUTO_WHITESPACE,
        [CHAR_NEWLINE] = AUTO_WHITESPACE,
        [CHAR_EOF] = AUTO_ACCEPT_WHITESPACE,
        [CHAR_UNKNOWN] = AUTO_ACCEPT_WHITESPACE
    },
    [AUTO_SLASH] = {
        [CHAR_LETTER] = AUTO_ACCEPT_OPERATOR,
        [CHAR_DIGIT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_UNDERSCORE] = AUTO_ACCEPT_OPERATOR,
        [CHAR_PLUS] = AUTO_ACCEPT_OPERATOR,
        [CHAR_MINUS] = AUTO_ACCEPT_OPERATOR,
        [CHAR_STAR] = AUTO_ACCEPT_COMMENT_BLOCK,
        [CHAR_SLASH] = AUTO_ACCEPT_COMMENT_LINE,
        [CHAR_PERCENT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_EQUAL] = AUTO_ACCEPT_OPERATOR,
        [CHAR_EXCLAMATION] = AUTO_ACCEPT_OPERATOR,
        [CHAR_AMPERSAND] = AUTO_ACCEPT_OPERATOR,
        [CHAR_PIPE] = AUTO_ACCEPT_OPERATOR,
        [CHAR_LT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_GT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_DOT] = AUTO_ACCEPT_OPERATOR,
        [CHAR_DELIMITER] = AUTO_ACCEPT_OPERATOR,
        [CHAR_WHITESPACE] = AUTO_ACCEPT_OPERATOR,
        [CHAR_NEWLINE] = AUTO_ACCEPT_OPERATOR,
        [CHAR_EOF] = AUTO_ACCEPT_OPERATOR,
        [CHAR_UNKNOWN] = AUTO_ACCEPT_OPERATOR
    }
};

#ifdef LEXER_DEBUG
static const char* char_type_to_string(CharType type) {
    static const char *names[] = {
        [CHAR_LETTER]       = "CHAR_LETTER",
        [CHAR_DIGIT]        = "CHAR_DIGIT",
        [CHAR_UNDERSCORE]   = "CHAR_UNDERSCORE",
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
    if (c == ';' || c == ',' || c == '(' || c == ')' || c == '{' || c == '}' || c == ':') { return CHAR_DELIMITER; }
    if (c == ' ' || c == '\t') { return CHAR_WHITESPACE; }
    if (c == '\n' || c == '\r') { return CHAR_NEWLINE; }
    return CHAR_UNKNOWN;
}

static CharType peek_char_type(const Lexer *lxr, size_t offset) {
    if (!lxr || !lxr->p) return CHAR_UNKNOWN;
    return get_char_type(lxr->p[offset]);
}

static AutomatonState automaton_classify(const Lexer *lxr) {
    AutomatonState state = AUTO_START;
    size_t offset = 0;

    while (1) {
        CharType type = peek_char_type(lxr, offset);
        AutomatonState next = transition_table[state][type];
        if (next >= AUT_ACCEPT_BASE || next == AUTO_ERROR_STATE) {
            return next;
        }
        state = next;
        offset++;
    }
}

static void consume_whitespace(Lexer *lxr) {
    while (lxr && lxr->p && lxr->p[0] != '\0') {
        CharType type = get_char_type(lxr->p[0]);
        if (type == CHAR_WHITESPACE || type == CHAR_NEWLINE) {
            lxr_advance(lxr);
            continue;
        }
        break;
    }
}

static void consume_line_comment(Lexer *lxr) {
    if (!lxr || !lxr->p) return;
    if (lxr->p[0] != '/' || lxr->p[1] != '/') {
        return;
    }
    while (lxr->p[0] != '\0' && lxr->p[0] != '\n') {
        lxr_advance(lxr);
    }
}

static bool consume_block_comment(Lexer *lxr) {
    if (!lxr || !lxr->p) return false;
    if (lxr->p[0] != '/' || lxr->p[1] != '*') {
        return false;
    }
    lxr_advance(lxr);
    lxr_advance(lxr);

    while (lxr->p[0] != '\0') {
        if (lxr->p[0] == '*' && lxr->p[1] == '/') {
            lxr_advance(lxr);
            lxr_advance(lxr);
            return true;
        }
        lxr_advance(lxr);
    }
    return false;
}

static void lexer_register_identifier(Lexer *lxr, const char *lexeme, size_t line) {
    if (!lxr || !lxr->symtab || !lexeme) return;
    symbol_table_insert(lxr->symtab, lexeme, TOKEN_IDENTIFIER, line);
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
 * @return El token creado (identificador o palabra reservada específica).
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
    
    int keyword_index = get_keyword_index(lexeme);
    TokenType ttype = keyword_token_from_index(keyword_index);
    token_t *token = create_token(ttype, lexeme, sl, sc);
    if (ttype == TOKEN_IDENTIFIER) {
        lexer_register_identifier(lxr, lexeme, sl);
    }
    free(lexeme);
    return token;
}

/**
 * @brief Analiza y crea un token para números decimales (enteros o reales con exponente).
 *
 * @param lxr El lexer.
 * @param sl Línea de inicio del token.
 * @param sc Columna de inicio del token.
 * @return El token creado (TOKEN_NUMBER o TOKEN_UNKNOWN).
 */
static token_t* lex_number(Lexer *lxr, size_t sl, size_t sc){
    const char  *start  = lxr->p;

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
 * @return El token creado para el operador/delimitador, o TOKEN_UNKNOWN en caso contrario.
 */
static token_t* lex_operator_or_delimiter(Lexer *lxr, size_t sl, size_t sc){
    const char *start = lxr->p;

    for (const MultiCharToken *op = multi_char_tokens; op->lexeme; ++op) {
        if (lxr_match2(lxr, op->lexeme)) {
            lxr_advance(lxr);
            lxr_advance(lxr);
            char *lex = make_lexeme(start, lxr->p);
            token_t *tok = create_token(op->type, lex, sl, sc);
            if (lex) {
                free(lex);
            }
            return tok;
        }
    }

    char c = lxr_peek(lxr);
    TokenType ttype = TOKEN_UNKNOWN;
    int recognized = 1;

    switch (c) {
        case ';': ttype = TOKEN_SEMICOLON; break;
        case ',': ttype = TOKEN_COMMA; break;
        case '(': ttype = TOKEN_LPAREN; break;
        case ')': ttype = TOKEN_RPAREN; break;
        case '{': ttype = TOKEN_LBRACE; break;
        case '}': ttype = TOKEN_RBRACE; break;
        case ':': ttype = TOKEN_COLON; break;
        case '+': ttype = TOKEN_PLUS; break;
        case '-': ttype = TOKEN_MINUS; break;
        case '*': ttype = TOKEN_STAR; break;
        case '/': ttype = TOKEN_SLASH; break;
        case '%': ttype = TOKEN_PERCENT; break;
        case '=': ttype = TOKEN_EQUAL; break;
        case '!': ttype = TOKEN_BANG; break;
        case '<': ttype = TOKEN_LESS; break;
        case '>': ttype = TOKEN_GREATER; break;
        default:
            recognized = 0;
            break;
    }

    lxr_advance(lxr);
    char *lex = make_lexeme(start, lxr->p);

    token_t *tok = create_token(recognized ? ttype : TOKEN_UNKNOWN, lex, sl, sc);
    if (lex) {
        free(lex);
    }
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
    lxr->symtab = NULL;
}

void lexer_set_symbol_table(Lexer *lxr, SymbolTable *symtab) {
    if (!lxr) return;
    lxr->symtab = symtab;
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
    for (;;) {
        AutomatonState decision = automaton_classify(lxr);
        size_t start_line = lxr->line;
        size_t start_col = lxr->col;

        switch (decision) {
            case AUTO_ACCEPT_IDENTIFIER:
                return lex_identifier_or_keyword(lxr, start_line, start_col);
            case AUTO_ACCEPT_NUMBER:
                return lex_number(lxr, start_line, start_col);
            case AUTO_ACCEPT_OPERATOR:
            case AUTO_ACCEPT_DELIMITER:
            case AUTO_ACCEPT_SLASH:
                return lex_operator_or_delimiter(lxr, start_line, start_col);
            case AUTO_ACCEPT_WHITESPACE:
                consume_whitespace(lxr);
                continue;
            case AUTO_ACCEPT_COMMENT_LINE:
                consume_line_comment(lxr);
                continue;
            case AUTO_ACCEPT_COMMENT_BLOCK:
                if (!consume_block_comment(lxr)) {
                    return create_token(TOKEN_UNKNOWN, "Unclosed comment", start_line, start_col);
                }
                continue;
            case AUTO_ACCEPT_EOF:
                return create_token(TOKEN_EOF, "EOF", start_line, start_col);
            case AUTO_ERROR_STATE:
            default: {
                char bad[2] = { lxr_peek(lxr), '\0' };
                token_t *unknown = create_token(TOKEN_UNKNOWN, bad, start_line, start_col);
                if (lxr_peek(lxr) != '\0') {
                    lxr_advance(lxr);
                }
                return unknown;
            }
        }
    }
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
    static const char *names[] = {
        [TOKEN_IDENTIFIER]  = "IDENT",
        [TOKEN_NUMBER]      = "NUMBER",
        [TOKEN_KW_FN]       = "KW_FN",
        [TOKEN_KW_LET]      = "KW_LET",
        [TOKEN_KW_MUT]      = "KW_MUT",
        [TOKEN_KW_RETURN]   = "KW_RETURN",
        [TOKEN_KW_TRUE]     = "KW_TRUE",
        [TOKEN_KW_FALSE]    = "KW_FALSE",
        [TOKEN_KW_I32]      = "KW_I32",
        [TOKEN_KW_F64]      = "KW_F64",
        [TOKEN_KW_BOOL]     = "KW_BOOL",
        [TOKEN_PLUS]        = "PLUS",
        [TOKEN_MINUS]       = "MINUS",
        [TOKEN_STAR]        = "STAR",
        [TOKEN_SLASH]       = "SLASH",
        [TOKEN_PERCENT]     = "PERCENT",
        [TOKEN_EQUAL]       = "EQUAL",
        [TOKEN_EQUAL_EQUAL] = "EQUAL_EQUAL",
        [TOKEN_BANG]        = "BANG",
        [TOKEN_BANG_EQUAL]  = "BANG_EQUAL",
        [TOKEN_LESS]        = "LESS",
        [TOKEN_LESS_EQUAL]  = "LESS_EQUAL",
        [TOKEN_GREATER]     = "GREATER",
        [TOKEN_GREATER_EQUAL] = "GREATER_EQUAL",
        [TOKEN_AND_AND]     = "AND_AND",
        [TOKEN_OR_OR]       = "OR_OR",
        [TOKEN_SEMICOLON]   = "SEMICOLON",
        [TOKEN_COMMA]       = "COMMA",
        [TOKEN_COLON]       = "COLON",
        [TOKEN_LPAREN]      = "LPAREN",
        [TOKEN_RPAREN]      = "RPAREN",
        [TOKEN_LBRACE]      = "LBRACE",
        [TOKEN_RBRACE]      = "RBRACE",
        [TOKEN_EOF]         = "EOF",
        [TOKEN_UNKNOWN]     = "UNKNOWN"
    };

    size_t count = sizeof(names) / sizeof(names[0]);
    if ((size_t)t < count && names[t]) {
        return names[t];
    }
    return "INVALID";
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
 * @brief Escribe los tokens de un archivo fuente a un archivo de salida con formato legible.
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
    fprintf(output, "# Formato: id_token nombre_token lexema linea columna\n");
    fprintf(output, "# Consulte token_type_name() para la correspondencia completa de identificadores.\n");
    fprintf(output, "\n");
    
    Lexer lexer;
    lexer_init(&lexer, source);
    SymbolTable temp_table;
    symbol_table_init(&temp_table);
    lexer_set_symbol_table(&lexer, &temp_table);
    
    int token_count = 0;
    for (;;) {
        token_t *token = lexer_next_token(&lexer);
        if (!token) {
            fprintf(output, "# Error: No se pudo obtener el siguiente token\n");
            break;
        }
    
    // Escribir en formato: id nombre lexema linea columna
        fprintf(output, "%d %s %s %zu %zu\n", 
               token->type,
               token_type_name(token->type),
               token->lexeme ? token->lexeme : "NULL",
               token->line, 
               token->column);
        
        token_count++;
        
        if (token->type == TOKEN_EOF) {
            free_token(token);
            break;
        }
        free_token(token);
    }
    
    fprintf(output, "\n# Total de tokens: %d\n", token_count);
    
    fclose(output);
    symbol_table_free(&temp_table);
    free(source);
    
    printf("✓ Tokens escritos en: %s (%d tokens)\n", output_file, token_count);
    return 0;
}