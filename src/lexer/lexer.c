/**
 * @file lexer.c
 * @brief Implementación de estructuras y funciones del lexer
 */
#include "lexer.h"
#include "keywords.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NUM_STATES 26
#define NUM_CHAR_TYPES 24

/*@brief Definición de los estados del autómata*/
typedef enum {
    STATE_START,
    STATE_IDENTIFIER,
    STATE_INT,
    STATE_BIN_PREFIX,
    STATE_BIN,
    STATE_HEX_PREFIX,
    STATE_HEX,
    STATE_REAL_FRACTION,
    STATE_EXPONENT_MARK,
    STATE_EXPONENT_SIGN,
    STATE_EXPONENT,
    STATE_STRING,
    STATE_STRING_ESCAPE,
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

// Eliminado bloque de depuración no utilizado

/*
 * Prototipos de funciones (declaración primero, implementación después).
 */

/**
 * @brief Clasifica un carácter en un CharType.
 * @param c Carácter a clasificar (int para soportar EOF).
 * @return CharType correspondiente.
 */
static CharType get_char_type(int c);

/**
 * @brief Devuelve el carácter actual sin consumirlo.
 */
static inline char lxr_peek(const Lexer *lxr);

/**
 * @brief Devuelve el siguiente carácter sin consumirlo.
 */
static inline char lxr_peek_next(const Lexer *lxr);

/**
 * @brief Avanza un carácter actualizando línea y columna.
 */
static inline void lxr_advance(Lexer *lxr);

/**
 * @brief Crea una copia de [start, end) como cadena terminada en NUL.
 */
static char* make_lexeme(const char *start, const char *end);

/**
 * @brief Comprueba dígitos hexadecimales completos.
 */
static inline int is_hex_digit_full(char c);

/**
 * @brief Comprueba dígitos binarios (0 o 1).
 */
static inline int is_bin_digit_full(char c);

/**
 * @brief Inicializa la matriz de transiciones del AFD.
 */
static void dfa_init(void);

/**
 * @brief Obtiene el siguiente token usando el AFD (maximal munch).
 */
static token_t* dfa_next_token(Lexer *lxr);

/**
 * @brief Lee el archivo completo a memoria (para main de prueba).
 */
static char *read_file(const char *filename);

/**
 * @brief Texto legible para un tipo de token (para main de prueba).
 */
static const char* token_type_name(TokenType t);

/**
 * @brief Clasifica un carácter en un CharType
 */
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

/* Tabla de transición del AFD */
static State DFA[NUM_STATES][NUM_CHAR_TYPES];
static int DFA_INIT = 0;

static inline int is_hex_digit_full(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static inline int is_bin_digit_full(char c) {
    return (c == '0' || c == '1');
}

static void dfa_init(void) {
    if (DFA_INIT) return;
    DFA_INIT = 1;

    // Por defecto, todo lleva a ERROR
    for (int s = 0; s < NUM_STATES; ++s) {
        for (int c = 0; c < NUM_CHAR_TYPES; ++c) {
            DFA[s][c] = STATE_ERROR;
        }
    }

    // START
    DFA[STATE_START][CHAR_LETTER]     = STATE_IDENTIFIER;
    DFA[STATE_START][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
    DFA[STATE_START][CHAR_DIGIT]      = STATE_INT;
    DFA[STATE_START][CHAR_QUOTE]      = STATE_STRING;
    DFA[STATE_START][CHAR_SLASH]      = STATE_SLASH;
    DFA[STATE_START][CHAR_PLUS]       = STATE_OPERATOR;
    DFA[STATE_START][CHAR_MINUS]      = STATE_OPERATOR;
    DFA[STATE_START][CHAR_STAR]       = STATE_OPERATOR;
    DFA[STATE_START][CHAR_PERCENT]    = STATE_OPERATOR;
    DFA[STATE_START][CHAR_EQUAL]      = STATE_OPERATOR_EQ;     // '=' o '=='
    DFA[STATE_START][CHAR_EXCLAMATION]= STATE_OPERATOR;        // '!' o '!='
    DFA[STATE_START][CHAR_AMPERSAND]  = STATE_OPERATOR_AND;    // '&' o '&&'
    DFA[STATE_START][CHAR_PIPE]       = STATE_OPERATOR_OR;     // '|' o '||'
    DFA[STATE_START][CHAR_LT]         = STATE_OPERATOR;        // '<' o '<='
    DFA[STATE_START][CHAR_GT]         = STATE_OPERATOR;        // '>' o '>='
    DFA[STATE_START][CHAR_DOT]        = STATE_OPERATOR;        // '.'
    DFA[STATE_START][CHAR_DELIMITER]  = STATE_DELIMITER;       // ; , ( ) { } [ ]
    DFA[STATE_START][CHAR_WHITESPACE] = STATE_WHITESPACE;
    DFA[STATE_START][CHAR_NEWLINE]    = STATE_WHITESPACE;
    DFA[STATE_START][CHAR_EOF]        = STATE_EOF;

    // IDENTIFIER
    DFA[STATE_IDENTIFIER][CHAR_LETTER]     = STATE_IDENTIFIER;
    DFA[STATE_IDENTIFIER][CHAR_DIGIT]      = STATE_IDENTIFIER;
    DFA[STATE_IDENTIFIER][CHAR_UNDERSCORE] = STATE_IDENTIFIER;

    // INT
    DFA[STATE_INT][CHAR_DIGIT] = STATE_INT;

    // REAL (parte fraccional)
    DFA[STATE_REAL_FRACTION][CHAR_DIGIT] = STATE_REAL_FRACTION;

    // Exponente
    DFA[STATE_EXPONENT_MARK][CHAR_PLUS]  = STATE_EXPONENT_SIGN;
    DFA[STATE_EXPONENT_MARK][CHAR_MINUS] = STATE_EXPONENT_SIGN;
    DFA[STATE_EXPONENT_MARK][CHAR_DIGIT] = STATE_EXPONENT;

    DFA[STATE_EXPONENT_SIGN][CHAR_DIGIT] = STATE_EXPONENT;
    DFA[STATE_EXPONENT][CHAR_DIGIT]      = STATE_EXPONENT;

    // STRING y escapes
    DFA[STATE_STRING][CHAR_BACKSLASH] = STATE_STRING_ESCAPE;
    DFA[STATE_STRING_ESCAPE][CHAR_UNKNOWN]    = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_LETTER]     = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_DIGIT]      = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_UNDERSCORE] = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_QUOTE]      = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_APOSTROPHE] = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_BACKSLASH]  = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_PLUS]       = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_MINUS]      = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_STAR]       = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_SLASH]      = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_PERCENT]    = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_EQUAL]      = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_EXCLAMATION]= STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_AMPERSAND]  = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_PIPE]       = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_LT]         = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_GT]         = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_DOT]        = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_DELIMITER]  = STATE_STRING;
    DFA[STATE_STRING_ESCAPE][CHAR_WHITESPACE] = STATE_STRING;

    // SLASH y comentarios
    DFA[STATE_SLASH][CHAR_SLASH] = STATE_COMMENT_LINE;  // //
    DFA[STATE_SLASH][CHAR_STAR]  = STATE_COMMENT_BLOCK; // /*

    DFA[STATE_COMMENT_LINE][CHAR_NEWLINE] = STATE_FINAL; // fin del comentario de línea
    DFA[STATE_COMMENT_LINE][CHAR_EOF]     = STATE_FINAL;

    for (int c = 0; c < NUM_CHAR_TYPES; ++c) {
        DFA[STATE_COMMENT_BLOCK][c] = STATE_COMMENT_BLOCK; // permanece en bloque
    }
    DFA[STATE_COMMENT_BLOCK][CHAR_STAR] = STATE_COMMENT_BLOCK_END; // posible cierre
    for (int c = 0; c < NUM_CHAR_TYPES; ++c) {
        DFA[STATE_COMMENT_BLOCK_END][c] = STATE_COMMENT_BLOCK; // vuelve al cuerpo si no es '/'
    }
    DFA[STATE_COMMENT_BLOCK_END][CHAR_STAR]  = STATE_COMMENT_BLOCK_END; // **...
    DFA[STATE_COMMENT_BLOCK_END][CHAR_SLASH] = STATE_FINAL;            // */

    // Operadores
    DFA[STATE_OPERATOR_EQ][CHAR_EQUAL] = STATE_FINAL;     // '=='
    DFA[STATE_OPERATOR_AND][CHAR_AMPERSAND] = STATE_FINAL; // '&&'
    DFA[STATE_OPERATOR_OR][CHAR_PIPE] = STATE_FINAL;        // '||'

    DFA[STATE_OPERATOR][CHAR_EQUAL] = STATE_FINAL; // +=, -=, *=, %=
    DFA[STATE_OPERATOR][CHAR_PLUS]  = STATE_FINAL; // ++
    DFA[STATE_OPERATOR][CHAR_MINUS] = STATE_FINAL; // --

    // Delimitadores y espacios
    DFA[STATE_WHITESPACE][CHAR_WHITESPACE] = STATE_WHITESPACE;
    DFA[STATE_WHITESPACE][CHAR_NEWLINE]    = STATE_WHITESPACE;
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

// Driver del AFD con maximal munch;

static token_t* dfa_next_token(Lexer *lxr) {
    dfa_init();

    for (;;) { // repetir mientras sólo consumamos ignorables
    size_t sl = lxr->line;
    size_t sc = lxr->col;
        const char *start = lxr->p;

        State state = STATE_START;
    State last_accept_state = STATE_ERROR; (void)last_accept_state; 
        const char *last_accept_pos = start;

        typedef enum {
            ACC_NONE,
            ACC_IDENT,
            ACC_NUMBER,
            ACC_STRING,
            ACC_OPERATOR,
            ACC_DELIM
        } AcceptKind;
        AcceptKind last_accept_kind = ACC_NONE;

        while (1) {
            int ch = (unsigned char)lxr_peek(lxr);
            CharType ct = get_char_type(ch);

            // Modo STRING: control de cierre
            if (state == STATE_STRING) {
                if (ct == CHAR_EOF || ct == CHAR_NEWLINE) {
                    break; 
                }
                if (ct == CHAR_QUOTE) {
                    lxr_advance(lxr);
                    last_accept_state = STATE_FINAL;
                    last_accept_pos = lxr->p;
                    last_accept_kind = ACC_STRING;
                    break;
                }
            }

            // Números: lookahead para 0x/0b, fracción y exponente
            if (state == STATE_INT) {
                if ((lxr->p == start + 1) && start[0] == '0' && (ch == 'x' || ch == 'X')) {
                    state = STATE_HEX_PREFIX;
                    lxr_advance(lxr);
                    continue;
                }
                if ((lxr->p == start + 1) && start[0] == '0' && (ch == 'b' || ch == 'B')) {
                    state = STATE_BIN_PREFIX;
                    lxr_advance(lxr);
                    continue;
                }
                if (ct == CHAR_DOT) {
                    char n = lxr_peek_next(lxr);
                    if (get_char_type(n) == CHAR_DIGIT) {
                        state = STATE_REAL_FRACTION;
                        lxr_advance(lxr);
                        continue;
                    } else {
                        break;
                    }
                }
                if (ch == 'e' || ch == 'E') {
                    state = STATE_EXPONENT_MARK;
                    lxr_advance(lxr);
                    continue;
                }
            } else if (state == STATE_REAL_FRACTION) {
                if (ch == 'e' || ch == 'E') {
                    state = STATE_EXPONENT_MARK;
                    lxr_advance(lxr);
                    continue;
                }
            } else if (state == STATE_HEX_PREFIX) {
                if (!is_hex_digit_full(ch)) {
                    break; // requiere al menos 1 dígito
                }
                state = STATE_HEX;
                lxr_advance(lxr);
                continue;
            } else if (state == STATE_HEX) {
                if (!is_hex_digit_full(ch)) {
                    break; // fin de hex
                }
                lxr_advance(lxr);
                continue;
            } else if (state == STATE_BIN_PREFIX) {
                if (!is_bin_digit_full(ch)) {
                    break;
                }
                state = STATE_BIN;
                lxr_advance(lxr);
                continue;
            } else if (state == STATE_BIN) {
                if (!is_bin_digit_full(ch)) {
                    break;
                }
                lxr_advance(lxr);
                continue;
            }

            // Transición por matriz
            State next = (ct >= 0 && ct < NUM_CHAR_TYPES) ? DFA[state][ct] : STATE_ERROR;

            // Operadores de 2 chars
            if (state == STATE_OPERATOR && ch == '=') {
                next = STATE_FINAL; // += -= *= %=
            }
            if (state == STATE_OPERATOR &&
                ((start[0] == '+' && ch == '+') || (start[0] == '-' && ch == '-'))) {
                next = STATE_FINAL; // ++, --
            }
            if (state == STATE_SLASH && ch == '=') {
                next = STATE_FINAL; // '/='
            }

            if (next == STATE_ERROR) break;

            // Consumir
            lxr_advance(lxr);
            state = next;

            // Registrar aceptaciones
            switch (state) {
                case STATE_IDENTIFIER:
                    last_accept_state = state;
                    last_accept_pos = lxr->p;
                    last_accept_kind = ACC_IDENT;
                    break;
                case STATE_INT:
                case STATE_REAL_FRACTION:
                case STATE_EXPONENT:
                case STATE_HEX:
                case STATE_BIN:
                    last_accept_state = state;
                    last_accept_pos = lxr->p;
                    last_accept_kind = ACC_NUMBER;
                    break;
                case STATE_SLASH:
                    // aceptar '/' como operador simple si no deriva a comentario ni '/='
                    last_accept_state = state;
                    last_accept_pos = lxr->p;
                    last_accept_kind = ACC_OPERATOR;
                    break;
                case STATE_OPERATOR:
                case STATE_OPERATOR_EQ:
                case STATE_OPERATOR_AND:
                case STATE_OPERATOR_OR:
                case STATE_FINAL:
                    last_accept_state = state;
                    last_accept_pos = lxr->p;
                    last_accept_kind = ACC_OPERATOR;
                    break;
                case STATE_DELIMITER:
                    last_accept_state = state;
                    last_accept_pos = lxr->p;
                    last_accept_kind = ACC_DELIM;
                    break;
                default:
                    break;
            }

            // Ignorables
            if (state == STATE_COMMENT_LINE) {
                while (lxr_peek(lxr) != '\0' && lxr_peek(lxr) != '\n') {
                    lxr_advance(lxr);
                }
                break; // reiniciar
            }
            if (state == STATE_COMMENT_BLOCK || state == STATE_COMMENT_BLOCK_END) {
                while (1) {
                    if (lxr_peek(lxr) == '\0') {
                        break; // unterminated
                    }
                    CharType ctt = get_char_type(lxr_peek(lxr));
                    State ns = DFA[state][ctt];
                    if (ns == STATE_ERROR) {
                        lxr_advance(lxr);
                        continue;
                    }
                    lxr_advance(lxr);
                    state = ns;
                    if (state == STATE_FINAL) {
                        break; // */
                    }
                }
                break; // reiniciar
            }
            if (state == STATE_WHITESPACE) {
                while (get_char_type(lxr_peek(lxr)) == CHAR_WHITESPACE || get_char_type(lxr_peek(lxr)) == CHAR_NEWLINE) {
                    lxr_advance(lxr);
                }
                break; // reiniciar
            }

            if (state == STATE_EOF) {
                return create_token(TOKEN_EOF, "EOF", sl, sc);
            }
        }

        
        if (state == STATE_COMMENT_LINE ||
            state == STATE_COMMENT_BLOCK ||
            state == STATE_COMMENT_BLOCK_END ||
            state == STATE_WHITESPACE) {
            if (lxr_peek(lxr) == '\0') {
                return create_token(TOKEN_EOF, "EOF", lxr->line, lxr->col);
            }
            continue;
        }

        
        if (last_accept_kind == ACC_NONE) {
            if (lxr->p == start) {
                lxr_advance(lxr);
            }
            char *lex = make_lexeme(start, lxr->p);
            token_t *tok = create_token(TOKEN_UNKNOWN, lex, sl, sc);
            free(lex);
            return tok;
        }

        // Volver a última aceptación
        lxr->p = last_accept_pos;
        char *lex = make_lexeme(start, last_accept_pos);
        token_t *tok = NULL;

        switch (last_accept_kind) {
            case ACC_IDENT: {
                TokenType ttype = is_keyword(lex) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
                tok = create_token(ttype, lex, sl, sc);
                break;
            }
            case ACC_NUMBER: {
                tok = create_token(TOKEN_NUMBER, lex, sl, sc);
                break;
            }
            case ACC_STRING: {
                tok = create_token(TOKEN_STRING, lex, sl, sc);
                break;
            }
            case ACC_OPERATOR: {
                tok = create_token(TOKEN_OPERATOR, lex, sl, sc);
                break;
            }
            case ACC_DELIM: {
                tok = create_token(TOKEN_DELIMITER, lex, sl, sc);
                break;
            }
            default: {
                tok = create_token(TOKEN_UNKNOWN, lex, sl, sc);
                break;
            }
        }
        free(lex);
        return tok;
    }
}

void lexer_init(Lexer *lxr, const char *source){
    lxr->source = source ? source : "";
    lxr->p = lxr->source;
    lxr->line = 1;
    lxr->col = 1;
}

token_t* lexer_next_token(Lexer *lxr){
    if (!lxr || !lxr->p) return NULL;
    if (lxr_peek(lxr) == '\0') {
        return create_token(TOKEN_EOF, "EOF", lxr->line, lxr->col);
    }
    return dfa_next_token(lxr);
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


static const char* token_type_name(TokenType t) {
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
        printf("line: %zu, column: %zu, Type: %s Lexeme: %s\n",token->line, token->column,token_type_name(token->type), token->lexeme ? token->lexeme : "NULL");

        if (token->type == TOKEN_EOF) {
            free_token(token);
            break;
        }
        free_token(token);
    }
    free(source);
    return 0;
}