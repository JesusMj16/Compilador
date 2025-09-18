## Estándar de Codificación – Proyecto Compilador en C

Este documento define las reglas de estilo de código para garantizar **consistencia, legibilidad y mantenibilidad** en el proyecto. El estándar está basado en **Google C Style**.

---

## 🔹 1. Formato general

* Codificación de archivos: **UTF-8**.
* Longitud máxima de línea: **80 caracteres**.
* Indentación: **Tabulacion**
* Una declaración por línea.
* Evitar espacios en blanco innecesarios (al final de línea o antes de saltos).

---

## 🔹 2. Nombres

* **Variables y funciones** → `snake_case`.

  ```c
  int line_number;
  void scan_token();
  ```

* **Tipos (struct, enum, typedef)** → `UpperCamelCase`.

  ```c
  typedef struct Token {
    int type;
    char *lexeme;
  } Token;
  ```

* **Constantes y macros** → `ALL_CAPS`.

  ```c
  #define MAX_TOKEN_LEN 256
  ```

* **Archivos** → minúsculas, palabras separadas con guión bajo.

  ```
  lexer.c, lexer.h, symbol_table.c
  ```

---

## 🔹 3. Llaves y bloques

* Llaves **en la misma línea de la declaración**.
* Siempre usar llaves, incluso en bloques de una línea.

```c
if (is_digit(c)) {
  return TOKEN_NUMBER;
} else {
  return TOKEN_ERROR;
}
```

---

## 🔹 4. Espaciado

* Un espacio entre `if`, `for`, `while` y el paréntesis.
* Un espacio después de comas.
* Operadores binarios rodeados de espacios.

```c
for (int i = 0; i < n; i++) {
  array[i] = i + 1;
}
```

---

## 🔹 5. Funciones

* Declarar el tipo de retorno en su propia línea.
* Documentar cada función con comentario Doxygen.
* Nombre debe describir **acción clara**.

```c
/**
 * @brief Escanea el siguiente token de la entrada.
 *
 * @param lexer Estado actual del analizador léxico.
 * @return Token encontrado.
 */
Token scan_token(Lexer *lexer);
```

---

## 🔹 6. Estructuras y enums

* Cada campo en línea nueva, con 2 espacios de indentación.
* Usar `typedef` para simplificar.

```c
typedef struct Lexer {
  const char *source;
  int position;
  int line;
} Lexer;

typedef enum TokenType {
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER,
  TOKEN_EOF
} TokenType;
```

---

## 🔹 7. Comentarios

* **Doxygen** para funciones, estructuras, tipos públicos.
* Comentarios en línea con `//` solo cuando sean muy breves.
* Evitar comentarios obvios.

```c
// Avanzar al siguiente carácter en la entrada.
lexer->position++;
```

---

## 🔹 8. Manejo de errores

* Todas las funciones que puedan fallar deben:

  1. Retornar un código de error (`int`, `bool`) o un objeto válido/nulo.
  2. Documentar cómo manejan los errores.

```c
/**
 * @brief Inserta un símbolo en la tabla.
 *
 * @param table Puntero a la tabla de símbolos.
 * @param name Nombre del símbolo.
 * @return true si se insertó, false en caso de error.
 */
bool insert_symbol(SymbolTable *table, const char *name);
```

---

## 🔹 9. Organización del proyecto

* Código fuente en `/src`.
* Headers en `/include`.
* Pruebas en `/tests`.
* Documentación en `/docs`.

---

## 🔹 10. Ejemplo completo

```c
#include "lexer.h"
#include <stdio.h>

/**
 * @brief Punto de entrada del compilador.
 *
 * @return 0 si la ejecución fue exitosa, otro valor si hubo error.
 */
int main(void) {
  Lexer lexer;
  init_lexer(&lexer, "program.txt");

  Token token;
  while ((token = scan_token(&lexer)).type != TOKEN_EOF) {
    printf("Token: %s\n", token.lexeme);
  }

  return 0;
}
```

---