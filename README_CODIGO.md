# Explicación Detallada del Compilador

Este documento describe, con enfoque didáctico, cada estructura de datos y función expuesta por el compilador. Además, incluye pruebas de escritorio (tablas de trazado) que ilustran la entrada, el proceso y la salida esperada de cada pieza para facilitar la verificación manual.

> **Convención de pruebas de escritorio**: Cada tabla contiene un escenario mínimo reproducible. La columna *Proceso esperado* resume los pasos relevantes (sean lecturas, estados LR o modificaciones a estructuras) y la columna *Salida/Efecto* detalla la condición final que permite validar el resultado.

---

## 1. Arquitectura General

| Fase | Archivo(s) | Función principal | Prueba de escritorio |
|------|------------|-------------------|----------------------|
| Léxica | `src/lexer/*.c`, `include/lexer.h` | Recorrer el código fuente y producir tokens con tipo, lexema, línea y columna. | Fuente `let x = 1;` → se esperan tokens `KW_LET`, `IDENT`, `EQUAL`, `NUMBER`, `SEMICOLON`, `EOF` con columnas 1,5,7,9,10. |
| Sintáctica (LR) | `src/parser/parser.c`, `include/parser.h` | Tokenizar toda la entrada, correr el autómata LR y convertir el árbol de reducciones en AST sin reiniciar el lexer. | Fuente `fn main(){ return 0; }` produce reducciones hasta `Programa` y un AST raíz `AST_PROGRAM` con un hijo `AST_FUNCTION`. |
| Tabla de símbolos | `src/parser/symbol_table.c`, `include/symbol_table.h` | Registrar identificadores, tipos y ámbitos durante el recorrido del AST y consultas del lexer. | Insertar `let mut total: i32` → entrada con `lexeme="total"`, `is_mutable=true`, `data_type="i32"`, `scope_level` el actual. |
| CLI | `src/main.c` | Coordinar fases según banderas `-l`, `-p`, `-t`, `-s`. | Ejecutar `compilador.exe -l archivo` solo imprime la tabla de tokens. |

---

## 2. Estructuras Clave

### 2.1 Tokens y Lexer (`include/lexer.h`)

| Estructura | Campos clave | Descripción | Prueba de escritorio |
|------------|-------------|-------------|----------------------|
| `TokenType` | Enumeración de tipos (`TOKEN_IDENTIFIER`, `TOKEN_KW_IF`, etc.) | Define el universo de tokens que el analizador léxico puede producir. | Al leer lexema `if`, el lexer asigna `TOKEN_KW_IF`; si la cadena no coincide con ninguna keyword, cae en `TOKEN_IDENTIFIER`. |
| `token_t` | `type`, `lexeme`, `line`, `column`, `next` | Representa un token individual con metadatos para reporte de errores. | Token generado para `let` en la línea 2, columna 5 almacena `type=TOKEN_KW_LET`, `lexeme="let"`, `line=2`, `column=5`. |
| `Lexer` | `source`, `p`, `line`, `col`, `symtab` | Estado interno del escáner. `p` avanza caracter por caracter; `symtab` permite registrar identificadores sobre la marcha. | Al invocar `lexer_next_token` cinco veces en `let x = 1;`, `p` avanza hasta el punto y `line` permanece en 1. |

### 2.2 Árbol de Sintaxis Abstracta y Parser (`include/parser.h`)

| Estructura | Campos clave | Descripción | Prueba de escritorio |
|------------|--------------|-------------|----------------------|
| `ASTNode` | `type`, `line`, `column`, `data` (uniones para listas, funciones, sentencias, literales, etc.) | Nodo genérico que almacena cualquier constructo. | Crear `AST_LET_STMT` para `let mut x = 5;` produce `data.let_stmt.name="x"`, `is_mutable=true`, `initializer` apuntando a `AST_NUMBER`. |
| `Parser` | Referencias a lexer, `symbol_table`, `source_text`, estado de errores y estadísticas | Coordina la tokenización completa, la corrida LR y la construcción del AST a partir de reducciones. | Tras `parser_parse`, `lines_compiled` refleja la última línea leída; si ocurre error, `has_error=true` y `error_line` indica la ubicación. |
| Enums `ASTNodeType`, `BinaryOp`, `UnaryOp` | Clasifican nodos y operadores. | Permiten que `ast_print` o generadores de código traduzcan operaciones sin analizar cadenas literales. |

### 2.3 Tabla de Símbolos (`include/symbol_table.h`)

| Estructura | Campos clave | Descripción | Prueba de escritorio |
|------------|--------------|-------------|----------------------|
| `LineList` | `line`, `next` | Lista enlazada de apariciones de un símbolo. | Insertar `foo` en líneas 3 y 8 crea dos nodos enlazados. |
| `SymbolEntry` | `lexeme`, `token_type`, `data_type`, `is_mutable`, `is_function`, `scope_level`, `lines` | Registro completo por identificador. | `symbol_entry_set_function(entry, true)` marca una función; al imprimir, se agrupa por ámbito. |
| `SymbolTable` | `head`, `tail`, `count`, `current_scope` | Contenedor principal para inserciones, búsquedas y scopes. | `symbol_table_enter_scope` incrementa `current_scope`; al salir se eliminan entradas de ese ámbito. |

### 2.4 Palabras Clave (`include/keywords.h`)

| Estructura | Campos clave | Descripción | Prueba de escritorio |
|------------|--------------|-------------|----------------------|
| — | — | La utilidad mantiene un arreglo estático (interno) de palabras clave. | `is_keyword("if")` retorna `true`; `get_keyword_index("if")` entrega el índice con el que el lexer codifica `TOKEN_KW_IF`. |

---

## 3. Funciones Públicas y Pruebas de Escritorio

### 3.1 Módulo Lexer

| Función | Responsabilidad | Entradas/Salidas | Prueba de escritorio |
|---------|-----------------|------------------|----------------------|
| `create_token` | Copia el lexema y arma un `token_t`. | `(type=TOKEN_NUMBER, lexeme="42", line=2, column=13)` → `token_t` heap. | Entrada `"42"` produce token independiente que no comparte buffer con el source; al liberar el source, el token sigue válido. |
| `free_token` / `free_token_list` | Liberan memoria de un token o lista. | Token previamente creado. | Tras generar tokens de `let x = 1;`, liberar la lista deja `valgrind` sin fugas. |
| `lexer_init` | Inicializa punteros y contadores. | `(Lexer*, source string)` | Después de llamar, `line=1`, `col=1` y `p` apunta al primer caracter. |
| `lexer_set_symbol_table` | Enlaza el lexer con la tabla de símbolos (opcional). | `(Lexer*, SymbolTable*)` | Al llamar antes de tokenizar, cada `TOKEN_IDENTIFIER` invoca `symbol_table_insert`. |
| `lexer_next_token` | Devuelve el siguiente token y avanza. | `(Lexer*)` → `token_t*`. | Fuente `if (a)` produce secuencia [`TOKEN_KW_IF`, `TOKEN_LPAREN`, `TOKEN_IDENTIFIER`, `TOKEN_RPAREN`, `TOKEN_EOF`]. |
| `read_file` | Lee archivo completo a memoria. | Ruta válida. | `read_file("src/lexer/test.txt")` retorna buffer terminado en `\0`. |
| `tokenize_all` / `get_next_token` | Ayudas para consumir todo el archivo en memoria temporal. | Cadena fuente. | `tokenize_all` sobre `let x = 1;` genera lista terminada en `TOKEN_EOF`. |
| `token_type_name` | Convierte enum a cadena human-readable. | `TOKEN_KW_IF` → "KW_IF". | Usado al imprimir la tabla de tokens, debe coincidir con los encabezados. |
| `write_tokens_to_file` | Serializa tokens en formato numérico para el parser. | `(source_file, output_file)` | Con `docs/Analizador-Lexico/examples/exito-01.txt` genera `*_tokens.txt` con líneas `tipo lexema linea columna`. |

### 3.2 Módulo Parser (AST + control)

| Función | Responsabilidad | Entradas/Salidas | Prueba de escritorio |
|---------|-----------------|------------------|----------------------|
| `ast_create_node` | Reserva un `ASTNode` base con tipo y posición. | `(AST_NODE_TYPE, line, col)` → `ASTNode*`. | Crear `AST_PROGRAM` en línea 1 produce nodo listo para agregar hijos. |
| `ast_create_list` | Inicializa nodos de tipo lista (`program`, `block`, argumentos). | Igual que arriba pero con arreglo de hijos. | Invocar para un bloque vacío produce `child_count=0` y capacidad inicial >=1. |
| `ast_add_child` | Inserta hijo en lista, realocando si es necesario. | `(parent list node, child)` | Añadir dos sentencias a un bloque crece su capacidad si se supera la inicial. |
| `ast_create_binary` / `ast_create_unary` | Construyen nodos de expresiones. | Operador + operandos | Expresión `x + 1` produce `BinaryOp=OP_ADD` y punteros a `AST_IDENTIFIER` y `AST_NUMBER`. |
| `ast_create_literal` | Fabrica nodos primarios (`identifier`, `number`, `bool`). | `(ASTNodeType, lexeme, line, col)` | `ast_create_literal(AST_BOOL, "true", 3,5)` produce literal booleano. |
| `ast_create_function`, `ast_create_parameter`, `ast_create_let`, `ast_create_return`, `ast_create_if`, `ast_create_call` | Construyen nodos compuestos específicos. | Parámetros según cada constructo. | `ast_create_if(cond, then, else, line, col)` liga los tres subárboles; si `else` es `NULL`, el impresor lo omite. |
| `ast_free` | Libera recursivamente un AST. | `(ASTNode*)`. | Tras procesar `fn main(){}`, liberar el árbol elimina también los literales e identificadores. |
| `ast_print` | Imprime el árbol con sangría. | `(ASTNode*, indent)` | Una función con return imprime jerárquicamente: `Function → Block → Return`. |
| `ast_node_type_name`, `binary_op_name`, `unary_op_name` | Convierte enums a cadenas. | Enums correspondientes. | `binary_op_name(OP_EQ)` retorna "==" para diagnósticos. |
| `parser_init` / `parser_free` | Enlazan lexer, tabla de símbolos y limpian contadores sin adelantar tokens (el parser LR relee `source_text`). | `(Parser*, Lexer*, SymbolTable*)` | Tras init, `source_text` apunta al buffer original y `lines_compiled=0`; `parser_free` solo desacopla punteros. |
| `parser_parse` | Tokeniza toda la entrada, ejecuta el autómata LR y construye el AST directamente desde el árbol de reducciones. | `(Parser*)` → `ASTNode*` o `NULL` | En archivos válidos retorna un `AST_PROGRAM`; ante conflictos LR o fallos de memoria, `has_error=true` y `error_msg` explica el problema. |
| `parser_print_errors`, `parser_print_stats` | Reportes resumidos. | `(Parser*)` | Después de un error, imprime bloque con línea/columna exacta. |
| `parser_get_error_count`, `parser_get_lines_compiled` | Consultas para CLI. | `(const Parser*)` | Permite a `main` decidir el código de salida. |

#### Funciones LR internas destacadas (todas en `src/parser/parser.c`)

| Función | Rol en el flujo LR | Prueba de escritorio |
|---------|--------------------|----------------------|
| `lr_get_parse_table` y helpers (`lr_build_automaton`, `lr_compute_first_follow`, etc.) | Construyen autómata LR(0), FIRST/FOLLOW y tablas acción/goto (memoizadas). | Cualquier ejecución imprime una tabla cuyos estados incluyen shift/reduce para los 14 no terminales definidos. |
| `lr_run_lr_parser` | Ejecuta el autómata sobre la tokenización completa, genera `LRReductionNode` y secuencia de reducciones predefinida. | Para `fn main(){}`, la secuencia finaliza con producción `Programa → Declaraciones`. |
| `lr_print_parse_table`, `lr_print_reduction_tree`, `lr_print_reduction_sequence` | Salida de diagnóstico opcional (solo si se compila con `PARSER_DEBUG_LR`). | Permiten inspeccionar estados, árbol y secuencia cuando se quiere depurar la gramática. |
| `lr_build_ast_from_tree` + familia `lr_build_*` | Recorren el árbol de reducciones y crean nodos `ASTNode` (programa, funciones, sentencias y expresiones) de manera ascendente. | Para `fn main(){}`, construyen directamente el bloque y las sentencias sin recurrir a un parser recursivo adicional. |

### 3.3 Módulo Tabla de Símbolos

| Función | Responsabilidad | Entradas/Salidas | Prueba de escritorio |
|---------|-----------------|------------------|----------------------|
| `symbol_table_init` / `symbol_table_free` | Inicializar y liberar la tabla y su memoria dinámica. | `(SymbolTable*)` | Tras `init`, `head=NULL`, `count=0`; después de `free`, no quedan nodos. |
| `symbol_table_insert` | Inserta (o encuentra y actualiza) un identificador en el ámbito actual. | `(table, "x", TOKEN_IDENTIFIER, line)` | Insertar dos veces el mismo lexema en el mismo ámbito reutiliza la entrada y agrega la línea a la lista. |
| `symbol_table_lookup` / `_scope` | Busca por lexema (global o restringido al ámbito actual). | `(table, "x")` | En un bloque interno con sombra de `x`, `_scope` devuelve la entrada local mientras `lookup` reporta la global si no hay local. |
| `symbol_entry_set_type/mutable/function/parameter` | Actualiza metadatos semánticos. | `(entry, value)` | Después de analizar `fn suma(a: i32)`, se marca `is_function=true` para `suma` y `is_parameter=true` para `a`. |
| `symbol_table_enter_scope`, `symbol_table_exit_scope`, `symbol_table_remove_scope` | Manejo explícito de ámbitos anidados. | — | Al entrar a un bloque, `current_scope` aumenta; `exit_scope` elimina entradas con ese nivel para evitar fugas. |
| `symbol_table_print` | Imprime todas las entradas agrupadas. | `(const SymbolTable*)` | Después de analizar `let x = 1;`, muestra fila con tipo `identifier` y mutabilidad falsa. |
| `symbol_table_write_to_file` | Serializa la tabla a disco. | `(table, filename)` | CLI genera `archivo.symbols.txt` tras un parse exitoso. |
| `symbol_table_build_from_tokens` | Construcción auxiliar usando una secuencia de tokens preexistente. | `(table, token_list)` | Útil al modo `-t` cuando queremos poblar la tabla sin parser. |
| `symbol_entry_add_line`, `symbol_entry_count_occurrences` | Gestionan la lista `LineList`. | `(entry, line)` | Dos apariciones en líneas 3 y 10 retornan conteo `2`. |

### 3.4 Módulo Palabras Clave

| Función | Responsabilidad | Prueba de escritorio |
|---------|-----------------|----------------------|
| `is_keyword` | Retorna `true` si el lexema pertenece al arreglo de keywords. | `is_keyword("return") → true`; `is_keyword("value") → false`. |
| `get_keyword_index` | Devuelve el índice usado por el lexer para anotar palabras reservadas en archivos de tokens. | `get_keyword_index("fn") → 0` (según la tabla interna). |

### 3.5 Módulo CLI (`src/main.c`)

| Función | Responsabilidad | Entradas/Salidas | Prueba de escritorio |
|---------|-----------------|------------------|----------------------|
| `print_usage` | Muestra ayuda en consola. | `(program_name)` | Ejecutar `compilador.exe -h` imprime las banderas disponibles. |
| `run_lexical_analysis` | Orquesta `read_file`, `lexer_init`, recorre todos los tokens y los imprime. | `(filename)` | Con `docs/Analizador-Lexico/examples/exito-01.txt`, muestra una tabla con `Total de tokens: 64`. |
| `generate_tokens_file` | Calcula el nombre de salida y llama `write_tokens_to_file`. | `(filename)` | Ejecutar `compilador.exe -t src/lexer/test.txt` crea `docs/Analizador-sintactico/archivos_parser/test_tokens.txt`. |
| `run_syntactic_analysis` | Configura lexer, símbolo y parser, ejecuta `parser_parse`, imprime AST, tabla de símbolos y estadísticas. | `(filename, show_stats)` | En un archivo válido, finaliza con resumen de errores=0 y líneas compiladas>0. |
| `main` | Procesa banderas, decide la fase a ejecutar, y dirige el flujo completo. | `argv` | `compilador.exe src/lexer/test.txt` ejecuta fases léxica y sintáctica consecutivas, mostrando tablas LR y AST (si no hay errores). |

---

## 4. Pruebas de Escritorio por Estructura/Función

> A continuación se muestran tablas genéricas que pueden reutilizarse como checklist manual.

### 4.1 Lexer + Tokens

| Escenario | Pasos esperados | Resultado |
|-----------|----------------|-----------|
| `let mut x = 10;` | `lexer_next_token` produce 6 tokens antes de `EOF`. | Tipos: `KW_LET`, `KW_MUT`, `IDENT`, `EQUAL`, `NUMBER`, `SEMICOLON`, `EOF`. |
| Número hexadecimal `0xFF` | El escáner detecta prefijo `0x` y consume dígitos hex. | Token `TOKEN_NUMBER` con lexema exacto, columna final desplazada 4 posiciones. |

### 4.2 Parser LR

| Escenario | Pasos esperados | Resultado |
|-----------|----------------|-----------|
| `fn main(){}` | Tabla LR imprime acciones para estados iniciales, secuencia de reducciones termina con la producción inicial. | `lr_print_reduction_tree` muestra raíz `Programa` con ramas `Declaraciones` → `DeclaraciónFunción`. |
| Condicional `if (x > 0) {}` | Durante LR, aparece reducción por producción `IfStmt → KW_IF LPAREN Expr RPAREN Block`. | El árbol de reducción incluye nodo `NT_IF_STMT` con hijos terminales y sub-árbol de expresión. |
| `fn main(){()` (falta `}`) | El autómata LR detecta conflicto al ver `EOF` antes de `RBRACE`; se genera error con línea/columna del token inesperado. | El recorrido de reducciones se aborta y no se construye AST, evitando estados inconsistentes. |

### 4.3 AST Constructor Functions

| Función | Escenario | Proceso esperado | Salida/Efecto |
|---------|-----------|------------------|----------------|
| `ast_create_let` | `let mut flag: bool = true;` | Crea literal bool, marca `is_mutable`, copia nombre/tipo. | Nodo LET con `initializer` → `AST_BOOL`. |
| `ast_create_if` | `if cond { then } else { else }` | Adjunta nodos `condition`, `then_branch`, `else_branch`. | Impresión recursiva muestra ambos bloques. |
| `ast_create_call` | `foo(a, b)` | Lista de argumentos se almacena como hijos en `arguments`. | `ast_print` indenta cada argumento. |

### 4.4 Tabla de Símbolos

| Escenario | Pasos esperados | Resultado |
|-----------|----------------|-----------|
| Entrar a bloque interno | `symbol_table_enter_scope`, insertar `x`, salir. | Al salir, `symbol_table_exit_scope` elimina la entrada local; `lookup("x")` encuentra la global anterior. |
| Registrar parámetro | Analizar `fn sum(a: i32)` llama `symbol_entry_set_parameter`. | Entrada de `a` tiene `is_parameter=true`, `scope_level=1`. |

---

## 5. Ejecución Reciente (`src/lexer/test.txt`)

| Comando | Resultado | Observaciones |
|---------|-----------|---------------|
| `build/compilador.exe src/lexer/test.txt` | Análisis léxico completó 107 tokens; el parser LR se detuvo en la línea 3, columna 18 porque el número hexadecimal `0xFF` se tokenizó como `NUMBER 0` seguido de `IDENT xFF`. | El error se reporta directamente desde el autómata LR (estado 8), por lo que no se construye AST. Se requiere ajustar el lexer para reconocer correctamente los sufijos hex/bin y evitar el token inesperado. |

---

## 6. Checklist de Limpieza

- [x] Parser LR imprime tabla de acciones/goto y árbol de reducción para cada corrida, validando las reducciones predefinidas de la gramática.
- [x] Código actualizado con comentarios puntuales (ej. fase de reducción) sin afectar funcionalidad.
- [x] Lexer, parser y tabla de símbolos liberan memoria al finalizar.

Este README sirve como referencia rápida para cualquier integrante del equipo que necesite comprender o verificar manualmente cada función y estructura del compilador.
