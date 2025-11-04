# Parser Ascendente LR - Documentación

## Descripción General

Este documento describe la implementación del **Parser Ascendente LR** (Left-to-Right, Rightmost derivation) para el compilador. El parser analiza la secuencia de tokens generada por el lexer y construye un Árbol de Sintaxis Abstracta (AST).

## Componentes Principales

### 1. PILA DEL PARSER

La pila es el componente fundamental del parsing ascendente. Mantiene:

#### Estructura `StackElement`
```c
typedef struct StackElement {
    int state;              // Estado LR actual
    ASTNode *node;          // Nodo del AST asociado
    TokenType token_type;   // Tipo de token (si es terminal)
} StackElement;
```

#### Estructura `ParserStack`
```c
typedef struct ParserStack {
    StackElement *elements;  // Array dinámico de elementos
    size_t size;            // Tamaño actual
    size_t capacity;        // Capacidad total
} ParserStack;
```

#### Operaciones de la Pila

1. **`stack_init()`**: Inicializa la pila con estado 0
2. **`stack_push()`**: Inserta un elemento (estado + nodo + tipo)
3. **`stack_pop()`**: Elimina y retorna el elemento del tope
4. **`stack_peek()`**: Consulta el tope sin eliminar
5. **`stack_top_state()`**: Obtiene el estado actual
6. **`stack_print()`**: Muestra el contenido (debug)

**Ejemplo de uso:**
```
Pila inicial:    [0]
Después de shift: [0, 5, 10]
Después de reduce: [0, 12]
```

---

### 2. ÁRBOL DE SINTAXIS ABSTRACTA (AST)

El AST es una representación estructurada del programa que elimina detalles sintácticos innecesarios.

#### Tipos de Nodos

```c
typedef enum ASTNodeType {
    // Estructurales
    AST_PROGRAM,           // Programa completo
    AST_FUNCTION,          // Definición de función
    AST_BLOCK,             // Bloque de código {}
    AST_STATEMENT_LIST,    // Lista de sentencias
    
    // Sentencias
    AST_LET_STMT,          // let x = ...
    AST_EXPR_STMT,         // Expresión como sentencia
    AST_IF_STMT,           // if condición { ... }
    AST_WHILE_STMT,        // while condición { ... }
    AST_RETURN_STMT,       // return valor
    AST_BREAK_STMT,        // break
    AST_CONTINUE_STMT,     // continue
    
    // Expresiones
    AST_BINARY_EXPR,       // a + b, a * b, etc.
    AST_UNARY_EXPR,        // !a, -a, +a
    AST_CALL_EXPR,         // función()
    
    // Literales
    AST_IDENTIFIER,        // variable
    AST_NUMBER,            // 42, 3.14
    AST_STRING,            // "texto"
    AST_CHAR,              // 'a'
    AST_BOOL,              // true, false
    AST_ARRAY              // [1, 2, 3]
} ASTNodeType;
```

#### Estructura del Nodo

```c
typedef struct ASTNode {
    ASTNodeType type;
    size_t line;
    size_t column;
    
    union {
        // Para listas (programa, bloque)
        struct {
            struct ASTNode **children;
            size_t child_count;
            size_t capacity;
        } list;
        
        // Para funciones
        struct {
            char *name;
            struct ASTNode *body;
        } function;
        
        // Para sentencia let
        struct {
            char *name;
            bool is_mutable;
            char *type;
            struct ASTNode *initializer;
        } let_stmt;
        
        // Para expresiones binarias
        struct {
            BinaryOp op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;
        
        // ... más variantes
    } data;
} ASTNode;
```

#### Operaciones del AST

1. **Creación:**
   - `ast_create_node()`: Nodo básico
   - `ast_create_list()`: Lista (programa, bloque)
   - `ast_create_binary()`: Expresión binaria
   - `ast_create_unary()`: Expresión unaria
   - `ast_create_literal()`: Literal
   - `ast_create_function()`: Función
   - `ast_create_let()`: Sentencia let
   - `ast_create_if()`: Sentencia if
   - `ast_create_while()`: Sentencia while
   - `ast_create_return()`: Sentencia return

2. **Manipulación:**
   - `ast_add_child()`: Agregar hijo a lista

3. **Liberación:**
   - `ast_free()`: Libera recursivamente el árbol

4. **Visualización:**
   - `ast_print()`: Imprime el árbol jerárquicamente

**Ejemplo de AST:**
```
Program (1 children)
  Function: main
    Block (2 children)
      LetStmt: x
        Number: 5
      BinaryExpr: +
        Identifier: x
        Number: 3
```

---

### 3. MATRIZ DE TRANSICIONES

El parser LR utiliza dos tablas para tomar decisiones:

#### Tabla ACTION

Define qué hacer al ver un token en un estado dado.

```c
typedef enum ActionType {
    ACTION_SHIFT,    // Desplazar token a la pila
    ACTION_REDUCE,   // Reducir según una producción
    ACTION_ACCEPT,   // Aceptar entrada (éxito)
    ACTION_ERROR     // Error sintáctico
} ActionType;

typedef struct {
    ActionType type;
    int value;  // Estado para SHIFT, producción para REDUCE
} Action;
```

**Conceptualmente:**
```
ACTION[estado][token] = acción

Ejemplo:
ACTION[0][fn]      = SHIFT 3
ACTION[5][;]       = REDUCE por producción 9
ACTION[12][EOF]    = ACCEPT
ACTION[7][NUMBER]  = ERROR
```

#### Tabla GOTO

Define el siguiente estado después de una reducción.

```
GOTO[estado][no_terminal] = nuevo_estado

Ejemplo:
GOTO[0][Programa]    = 1
GOTO[0][ListaItems]  = 2
GOTO[3][Funcion]     = 8
```

#### Producciones de la Gramática

Cada reducción corresponde a una producción:

```c
static const Production productions[] = {
    // 0: Programa -> ListaItems EOF
    {NT_PROGRAMA, 2, "Programa -> ListaItems EOF"},
    
    // 1-2: ListaItems
    {NT_LISTA_ITEMS, 2, "ListaItems -> Item ListaItems"},
    {NT_LISTA_ITEMS, 0, "ListaItems -> epsilon"},
    
    // 3-4: Item
    {NT_ITEM, 1, "Item -> Funcion"},
    {NT_ITEM, 1, "Item -> Sentencia"},
    
    // 5: Funcion -> fn IDENT ( ) Bloque
    {NT_FUNCION, 5, "Funcion -> fn IDENT ( ) Bloque"},
    
    // ... más producciones
};
```

**Estructura de una Producción:**
```c
typedef struct {
    NonTerminal left;      // Lado izquierdo (A)
    int right_size;        // Cantidad de símbolos en lado derecho
    const char *name;      // Descripción legible
} Production;
```

---

## Algoritmo de Parsing

### Parsing Ascendente LR

El algoritmo básico es:

```
1. Inicializar pila con estado 0
2. Repetir:
   a. consultar = stack_top_state()
   b. token = current_token
   c. acción = ACTION[consultar][token]
   
   d. Si acción == SHIFT:
      - Empujar token y nuevo estado
      - Avanzar al siguiente token
      
   e. Si acción == REDUCE:
      - producción = obtener producción
      - Quitar right_size elementos de la pila
      - nuevo_estado = GOTO[stack_top_state()][producción.left]
      - Empujar no-terminal y nuevo estado
      - Construir nodo del AST
      
   f. Si acción == ACCEPT:
      - ¡Éxito! Retornar AST
      
   g. Si acción == ERROR:
      - Reportar error sintáctico
```

### Implementación Actual

Por simplicidad, esta implementación utiliza **parsing recursivo descendente con precedencia de operadores** en lugar de tablas LR completas. Esto mantiene la misma interfaz pero simplifica la implementación.

**Ventajas del enfoque actual:**
- Más fácil de mantener y extender
- Mejor manejo de errores
- Precedencia de operadores natural
- Código más legible

**Las tablas LR completas** están preparadas para una implementación futura si se requiere.

---

## Precedencia de Operadores

El parser maneja la precedencia correctamente:

### Tabla de Precedencia (menor a mayor)

1. **Asignación**: `=`, `+=`, `-=`, `*=`, `/=`, `%=` (asociativa derecha)
2. **OR lógico**: `||`
3. **AND lógico**: `&&`
4. **Igualdad**: `==`, `!=`
5. **Comparación**: `<`, `<=`, `>`, `>=`
6. **Suma/Resta**: `+`, `-`
7. **Multiplicación/División**: `*`, `/`, `%`
8. **Unarios**: `!`, `-`, `+`
9. **Postfijo**: llamadas `()`

**Ejemplo:**
```
a + b * c      →  (a + (b * c))
a = b = c      →  (a = (b = c))
!a && b        →  ((!a) && b)
```

---

## Uso del Parser

### Desde Línea de Comandos

```bash
# Solo análisis sintáctico
./bin/compilador -p archivo.lang

# Con estadísticas
./bin/compilador -p -s archivo.lang

# Análisis completo (léxico + sintáctico)
./bin/compilador archivo.lang
```

### Desde Código

```c
// Inicializar lexer
Lexer lexer;
lexer_init(&lexer, source_code);

// Inicializar parser
Parser parser;
parser_init(&parser, &lexer);

// Parsear
ASTNode *ast = parser_parse(&parser);

if (parser.has_error) {
    parser_print_error(&parser);
} else {
    // Usar el AST
    ast_print(ast, 0);
    
    // Liberar
    ast_free(ast);
}

parser_free(&parser);
```

---

## Manejo de Errores

El parser detecta errores sintácticos y proporciona mensajes descriptivos:

```c
typedef struct Parser {
    // ... otros campos
    
    bool has_error;
    char error_msg[256];
    size_t error_line;
    size_t error_col;
} Parser;
```

**Ejemplos de mensajes:**
```
  Error de parsing en línea 5, columna 12:
   Se esperaba ';' después de la sentencia let

   Error de parsing en línea 10, columna 8:
   Se esperaba '}' al final del bloque
```

---

## Estadísticas

El parser lleva conteo de operaciones:

```c
typedef struct Parser {
    // ... otros campos
    
    int shift_count;    // Número de desplazamientos
    int reduce_count;   // Número de reducciones
} Parser;
```

**Ejemplo de salida:**
```
   Estadísticas del Parser:
   Desplazamientos (shift): 45
   Reducciones (reduce):    23
   Tamaño de pila:          1
```

---

## Extensiones Futuras

### 1. Tablas LR Completas

Implementar tablas ACTION y GOTO completas basadas en la gramática.

### 2. Recuperación de Errores

Mejorar el manejo de errores con:
- Modo pánico
- Sincronización en tokens clave
- Múltiples errores por archivo

### 3. Más Construcciones

- Parámetros de función
- Tipos genéricos
- Match expressions
- For loops
- Arrays y acceso a elementos

### 4. Optimizaciones

- Compartir nodos inmutables
- Pool de memoria para nodos
- Tabla de símbolos durante el parsing

---

## Ejemplo Completo

**Entrada:**
```rust
fn main() {
    let x: i32 = 5;
    let y = x + 3;
    if y > 5 {
        return y * 2;
    }
}
```

**AST Generado:**
```
Program (1 children)
  Function: main
    Block (3 children)
      LetStmt: xi32
        Number: 5
      LetStmt: y
        BinaryExpr: +
          Identifier: x
          Number: 3
      IfStmt
        Condition:
          BinaryExpr: >
            Identifier: y
            Number: 5
        Then:
          Block (1 children)
            ReturnStmt
              BinaryExpr: *
                Identifier: y
                Number: 2
```

---

## Referencias

- **Libro:** "Compilers: Principles, Techniques, and Tools" (Dragon Book)
- **Gramática:** Ver `docs/Analizador-sintactico/docs/gramatica.md`
- **Tablas PI/PD:** Ver `docs/Analizador-sintactico/docs/tabla-pi-pd.md`
- **Código fuente:** `src/parser/parser.c`, `include/parser.h`

---

## Resumen de Archivos

```
include/
  parser.h              # Definiciones del parser y AST

src/parser/
  parser.c              # Implementación completa
  
docs/Analizador-sintactico/
  docs/
    gramatica.md        # Gramática del lenguaje
    tabla-pi-pd.md      # Conjuntos FIRST y LAST
    parser-doc.md       # Este documento
```

---

**Nota:** Esta implementación proporciona una base sólida para el análisis sintáctico y puede extenderse fácilmente según las necesidades del proyecto.
