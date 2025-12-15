## Estándar de Documentación – Proyecto Compilador en C

Este documento define cómo se debe **documentar el proyecto**, tanto en el repositorio como dentro del código fuente, siguiendo la filosofía de **Google C Style**.

---

## 🔹 1. Documentación en el repositorio

### Estructura recomendada

* `README.md` → descripción general, instrucciones de compilación y uso.
* `CODING_STYLE.md` → reglas de codificación.
* `DOCS_STYLE.md` → este documento.
* `docs/` → documentación extendida (diagramas, gramáticas, reportes).
* `tests/` → pruebas con casos de entrada/salida documentados.

---

## 🔹 2. Estilo de archivos Markdown (`.md`)

* Encabezados con `#` claros y jerárquicos.
* Tablas para tokens, gramática y operadores.
* Listas numeradas para pasos, listas con `-` para elementos generales.
* Código en bloques triple backtick \`\`\` con el lenguaje especificado (`c`, `bash`, `ebnf`, etc.).
* Imágenes o diagramas guardados en `/docs/img/` y referenciados con rutas relativas.

Ejemplo de tabla para tokens:

| Token         | Regex                    | Descripción                  |
| ------------- | ------------------------ | ---------------------------- |
| IDENTIFICADOR | `[a-zA-Z_][a-zA-Z0-9_]*` | Nombre de variable o función |
| NUMERO        | `[0-9]+`                 | Entero decimal               |

---

## 🔹 3. Documentación dentro del código

Usar **comentarios Doxygen** para que la documentación pueda generarse automáticamente con herramientas.

### 3.1 Archivos `.h` y `.c`

Cada archivo debe iniciar con un bloque:

```c
/**
 * @file flex_runner.c
 * @brief Runner de pruebas del lexer (Flex).
 *
 * Contiene utilidades para ejecutar el lexer y comparar salidas esperadas.
 */
```

### 3.2 Funciones

Cada función documentada así:

```c
/**
 * @brief Escanea el siguiente token de la entrada.
 *
 * @param lexer Estado actual del analizador léxico.
 * @return Token encontrado.
 */
Token scan_token(Lexer *lexer);
```

### 3.3 Estructuras y enums

```c
/**
 * @brief Representa un token del lenguaje.
 */
typedef struct {
  TokenType type;  /**< Tipo de token */
  char *lexeme;    /**< Texto del token */
  int line;        /**< Línea en el código fuente */
} Token;
```

---

## 🔹 4. Documentación de diseño

Toda especificación más extensa va en `/docs`.

1. **Lenguaje soportado**

   * Descripción del alfabeto.
   * Reglas léxicas (tokens en regex y en EBNF).
   * Gramática sintáctica (si aplica).

2. **Diagramas**

   * Autómatas (AFD).
   * Árboles de análisis.
   * Flujo del compilador (pipeline: entrada → lexer → parser → salida).

3. **Casos de prueba documentados**
   Cada archivo de prueba debe tener:

   * Descripción breve.
   * Entrada del lenguaje.
   * Salida esperada.

Ejemplo:

```
Caso: Identificadores válidos
Entrada:
    int x;
    float var123;
Salida esperada:
    TOKEN_INT "int"
    TOKEN_IDENTIFIER "x"
    TOKEN_FLOAT "float"
    TOKEN_IDENTIFIER "var123"
```

---