/*
 * flex_runner.c
 * Runner mínimo para ejecutar el lexer de Flex y comparar contra archivos .expected.txt.
 *
 * Compilación (ejemplo):
 *   flex -o build/lexer.yy.c src/lexer_flex/lexer.l
 *   gcc -Ibuild -o bin/flex-runner build/lexer.yy.c src/automatizado/lexer/flex_runner.c
 *
 * Uso:
 *   bin/flex-runner --lex <archivo>
 *   bin/flex-runner --check <archivo> <archivo.expected.txt>
 *   bin/flex-runner --check-all docs/Analizador-Lexico/examples-flex
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tokens/YYSTYPE generados por Bison (Makefile los genera en build/) */
#include "parser.tab.h"

/* Interfaz Flex */
extern int yylex(void);
extern FILE *yyin;
extern char *yytext;

/* Estado de posición (definido en lexer.l) */
extern int yylineno;
extern int yycolumn;

/* yylval viene declarado por Bison (parser.tab.h) */
extern YYSTYPE yylval;

static const char *token_name(int t) {
    switch (t) {
        case TOKEN_IDENTIFIER:     return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:         return "TOKEN_NUMBER";
        case TOKEN_KW_FN:          return "TOKEN_KW_FN";
        case TOKEN_KW_LET:         return "TOKEN_KW_LET";
        case TOKEN_KW_MUT:         return "TOKEN_KW_MUT";
        case TOKEN_KW_IF:          return "TOKEN_KW_IF";
        case TOKEN_KW_ELSE:        return "TOKEN_KW_ELSE";
        case TOKEN_KW_RETURN:      return "TOKEN_KW_RETURN";
        case TOKEN_KW_TRUE:        return "TOKEN_KW_TRUE";
        case TOKEN_KW_FALSE:       return "TOKEN_KW_FALSE";
        case TOKEN_KW_I32:         return "TOKEN_KW_I32";
        case TOKEN_KW_F64:         return "TOKEN_KW_F64";
        case TOKEN_KW_BOOL:        return "TOKEN_KW_BOOL";
        case TOKEN_PLUS:           return "TOKEN_PLUS";
        case TOKEN_MINUS:          return "TOKEN_MINUS";
        case TOKEN_STAR:           return "TOKEN_STAR";
        case TOKEN_SLASH:          return "TOKEN_SLASH";
        case TOKEN_PERCENT:        return "TOKEN_PERCENT";
        case TOKEN_EQUAL:          return "TOKEN_EQUAL";
        case TOKEN_EQUAL_EQUAL:    return "TOKEN_EQUAL_EQUAL";
        case TOKEN_BANG:           return "TOKEN_BANG";
        case TOKEN_BANG_EQUAL:     return "TOKEN_BANG_EQUAL";
        case TOKEN_LESS:           return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL:     return "TOKEN_LESS_EQUAL";
        case TOKEN_GREATER:        return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL:  return "TOKEN_GREATER_EQUAL";
        case TOKEN_AND_AND:        return "TOKEN_AND_AND";
        case TOKEN_OR_OR:          return "TOKEN_OR_OR";
        case TOKEN_SEMICOLON:      return "TOKEN_SEMICOLON";
        case TOKEN_COMMA:          return "TOKEN_COMMA";
        case TOKEN_COLON:          return "TOKEN_COLON";
        case TOKEN_LPAREN:         return "TOKEN_LPAREN";
        case TOKEN_RPAREN:         return "TOKEN_RPAREN";
        case TOKEN_LBRACE:         return "TOKEN_LBRACE";
        case TOKEN_RBRACE:         return "TOKEN_RBRACE";
        case TOKEN_EOF:            return "TOKEN_EOF";
        case TOKEN_UNKNOWN:        return "TOKEN_UNKNOWN";
        default:                   return "TOKEN_???";
    }
}

static void reset_lexer_state(void) {
    yylineno = 1;
    yycolumn = 1;
    yylval.lexeme = NULL;
}

static char *read_entire_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }

    char *buf = (char *)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[r] = '\0';
    return buf;
}

static char *normalize_newlines(const char *s) {
    /* Convierte CRLF -> LF para comparación estable. */
    size_t n = strlen(s);
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        if (s[i] == '\r') {
            if (i + 1 < n && s[i + 1] == '\n') {
                continue;
            }
            out[j++] = '\n';
        } else {
            out[j++] = s[i];
        }
    }
    out[j] = '\0';
    return out;
}

static int append_line(char **buf, size_t *cap, size_t *len, const char *line) {
    size_t n = strlen(line);
    if (*len + n + 1 > *cap) {
        size_t new_cap = (*cap == 0) ? 1024 : *cap;
        while (*len + n + 1 > new_cap) new_cap *= 2;
        char *new_buf = (char *)realloc(*buf, new_cap);
        if (!new_buf) return 0;
        *buf = new_buf;
        *cap = new_cap;
    }
    memcpy(*buf + *len, line, n);
    *len += n;
    (*buf)[*len] = '\0';
    return 1;
}

static char *lex_to_string(FILE *in) {
    reset_lexer_state();
    yyin = in;

    char *out = NULL;
    size_t cap = 0;
    size_t len = 0;

    for (;;) {
        int tok = yylex();
        const char *name = token_name(tok);
        const char *lex = (tok == TOKEN_EOF) ? "" : yytext;

        char line[512];
        snprintf(line, sizeof(line), "%-15s \"%s\"\n", name, lex);
        if (!append_line(&out, &cap, &len, line)) {
            free(out);
            return NULL;
        }

        if (tok == TOKEN_IDENTIFIER || tok == TOKEN_NUMBER) {
            free(yylval.lexeme);
            yylval.lexeme = NULL;
        }

        if (tok == TOKEN_EOF) break;
    }

    return out;
}

static int check_one(const char *input_path, const char *expected_path) {
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        fprintf(stderr, "No se pudo abrir input: %s\n", input_path);
        return 2;
    }

    char *actual = lex_to_string(in);
    fclose(in);
    if (!actual) {
        fprintf(stderr, "Error: no se pudo generar salida de tokens\n");
        return 2;
    }

    char *expected_raw = read_entire_file(expected_path);
    if (!expected_raw) {
        fprintf(stderr, "No se pudo abrir expected: %s\n", expected_path);
        free(actual);
        return 2;
    }

    char *expected = normalize_newlines(expected_raw);
    char *actual_n = normalize_newlines(actual);

    int ok = (expected && actual_n && strcmp(expected, actual_n) == 0);
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", input_path);
        fprintf(stderr, "--- expected (%s) ---\n%s\n", expected_path, expected ? expected : "<NULL>");
        fprintf(stderr, "--- actual ---\n%s\n", actual_n ? actual_n : "<NULL>");
    } else {
        printf("PASS: %s\n", input_path);
    }

    free(expected_raw);
    free(expected);
    free(actual);
    free(actual_n);

    return ok ? 0 : 1;
}

#ifdef _WIN32
#include <windows.h>
static int check_all_in_dir(const char *dir) {
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*.expected.txt", dir);

    WIN32_FIND_DATAA ffd;
    HANDLE h = FindFirstFileA(pattern, &ffd);
    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "No se encontraron expected en: %s\n", dir);
        return 2;
    }

    int failures = 0;
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        char expected_path[MAX_PATH];
        snprintf(expected_path, sizeof(expected_path), "%s\\%s", dir, ffd.cFileName);

        char input_path[MAX_PATH];
        snprintf(input_path, sizeof(input_path), "%s\\%s", dir, ffd.cFileName);
        char *p = strstr(input_path, ".expected.txt");
        if (p) *p = '\0';

        int r = check_one(input_path, expected_path);
        if (r != 0) failures++;

    } while (FindNextFileA(h, &ffd));

    FindClose(h);
    return failures == 0 ? 0 : 1;
}
#else
#include <dirent.h>
static int ends_with(const char *s, const char *suffix) {
    size_t n = strlen(s), m = strlen(suffix);
    if (m > n) return 0;
    return strcmp(s + (n - m), suffix) == 0;
}

static int check_all_in_dir(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) {
        fprintf(stderr, "No se pudo abrir dir: %s\n", dir);
        return 2;
    }

    int failures = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (!ends_with(ent->d_name, ".expected.txt")) continue;

        char expected_path[1024];
        snprintf(expected_path, sizeof(expected_path), "%s/%s", dir, ent->d_name);

        char input_path[1024];
        snprintf(input_path, sizeof(input_path), "%s/%s", dir, ent->d_name);
        char *p = strstr(input_path, ".expected.txt");
        if (p) *p = '\0';

        int r = check_one(input_path, expected_path);
        if (r != 0) failures++;
    }

    closedir(d);
    return failures == 0 ? 0 : 1;
}
#endif

static void usage(const char *prog) {
    fprintf(stderr,
        "Uso:\n"
        "  %s --lex <archivo>\n"
        "  %s --check <archivo> <archivo.expected.txt>\n"
        "  %s --check-all <dir>\n",
        prog, prog, prog);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argv[0]);
        return 2;
    }

    if (strcmp(argv[1], "--lex") == 0) {
        const char *input_path = argv[2];
        FILE *in = fopen(input_path, "rb");
        if (!in) {
            fprintf(stderr, "No se pudo abrir input: %s\n", input_path);
            return 2;
        }
        char *out = lex_to_string(in);
        fclose(in);
        if (!out) return 2;
        fputs(out, stdout);
        free(out);
        return 0;
    }

    if (strcmp(argv[1], "--check") == 0) {
        if (argc < 4) {
            usage(argv[0]);
            return 2;
        }
        return check_one(argv[2], argv[3]);
    }

    if (strcmp(argv[1], "--check-all") == 0) {
        return check_all_in_dir(argv[2]);
    }

    usage(argv[0]);
    return 2;
}
