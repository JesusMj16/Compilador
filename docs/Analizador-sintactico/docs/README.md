 # Parser (versión automatizada con Bison)

Este directorio documenta el analizador sintáctico del proyecto en su versión **automatizada**, implementada con **Bison**.

## Documentos disponibles

- [gramatica-ebnf.md](gramatica-ebnf.md): gramática EBNF del lenguaje.
- [gramatica-bnf.md](gramatica-bnf.md): gramática en BNF (derivada).

## Ejemplos

Los casos de prueba del parser están en:

- `docs/Analizador-sintactico/examples-bison/`

Cada caso suele incluir el archivo `.txt` y su salida esperada `.expected.txt`.

## Ejecución

El ejecutable principal parsea un archivo y reporta `PARSE_OK` o `PARSE_FAIL`.

En Windows (MinGW):

```powershell
mingw32-make run-file FILE=docs/Analizador-sintactico/examples-bison/parse-exito-01.txt
```

En Linux/macOS:

```bash
make run-file FILE=docs/Analizador-sintactico/examples-bison/parse-exito-01.txt
```

### 2. Árbol de Sintaxis Abstracta (AST)
- Representa la estructura del programa
- 20+ tipos de nodos
- Funciones de construcción y liberación

### 3. Matriz de Transiciones
- Tabla ACTION (shift, reduce, accept, error)
- Tabla GOTO (transiciones de estados)
- 66 producciones de la gramática

## Ejemplos

### Ejemplo Simple
```rust
let x = 5 + 3;
```

**AST Generado:**
```
LetStmt: x
└─ BinaryExpr: +
   ├─ Number: 5
   └─ Number: 3
```

### Ejemplo Completo
```rust
fn main() {
    let x: i32 = 5;
    if x > 3 {
        return x + 1;
    }
}
```

**AST Generado:**
```
Program
└─ Function: main
   └─ Block
      ├─ LetStmt: xi32
      │  └─ Number: 5
      └─ IfStmt
         ├─ Condition: x > 3
         └─ Then: Block
            └─ Return: x + 1
```

## Nota

El contenido anterior sobre implementación manual y estadísticas no aplica en esta rama automatizada.
- [x] Pila dinámica con estados LR
- [x] Construcción de AST
- [x] Precedencia de operadores
- [x] Manejo de errores descriptivo
- [x] Compatible con lexer existente
- [x] Extensible y mantenible
- [x] Completamente documentado

## Conceptos Cubiertos

- **Parser LR**: Left-to-right, Rightmost derivation
- **Shift-Reduce**: Operaciones fundamentales
- **AST**: Árbol de Sintaxis Abstracta
- **Precedencia**: Orden de evaluación de operadores
- **Tablas de transición**: ACTION y GOTO
- **Producciones**: Reglas de la gramática
- **Análisis ascendente**: Bottom-up parsing
