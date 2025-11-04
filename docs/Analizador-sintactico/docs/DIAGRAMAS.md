# Diagrama de Funcionamiento del Parser

## 🔄 Flujo del Parser Ascendente

```
┌─────────────────────────────────────────────────────────────────┐
│                    COMPILADOR - FLUJO COMPLETO                  │
└─────────────────────────────────────────────────────────────────┘

    CÓDIGO FUENTE
         │
         ▼
    ┌────────┐
    │ LEXER  │ ────────► Tokens: fn, main, (, ), {, let, ...
    └────────┘
         │
         ▼
    ┌────────────────────────────────────────────────────┐
    │                   PARSER LR                        │
    │                                                    │
    │  ┌──────────┐     ┌─────────────┐     ┌────────┐ │
    │  │  PILA    │◄────┤  ALGORITMO  ├────►│ TABLAS │ │
    │  │          │     │   LR        │     │ ACTION │ │
    │  │ [0]      │     │             │     │  GOTO  │ │
    │  │ [0, 5]   │     │  shift/     │     └────────┘ │
    │  │ [0, 12]  │     │  reduce     │                │
    │  └──────────┘     └─────────────┘                │
    │                          │                        │
    │                          ▼                        │
    │                    Construye AST                  │
    └────────────────────────────────────────────────────┘
         │
         ▼
    ┌────────────────────────────┐
    │   ÁRBOL DE SINTAXIS       │
    │   ABSTRACTA (AST)         │
    │                           │
    │   Program                 │
    │     └─ Function: main     │
    │         └─ Block          │
    │             ├─ LetStmt    │
    │             └─ IfStmt     │
    └────────────────────────────┘
```

## 📚 Estructura de la Pila

```
┌─────────────────────────────────────────────────────────┐
│                    STACK ELEMENT                        │
├─────────────────────────────────────────────────────────┤
│  state:       int        │ Estado LR (0, 1, 2, ...)   │
│  node:        ASTNode*   │ Nodo del árbol parcial     │
│  token_type:  TokenType  │ Tipo de token (si terminal)│
└─────────────────────────────────────────────────────────┘

Ejemplo de evolución de la pila:

Entrada: let x = 5;

Paso 1:  [0]                          # Estado inicial
Paso 2:  [0, (let, 3)]                # Shift: let
Paso 3:  [0, (let, 3), (x, 8)]        # Shift: identificador
Paso 4:  [0, (let, 3), (x, 8), (=, 12)] # Shift: =
Paso 5:  [0, (let, 3), (x, 8), (=, 12), (5, 15)] # Shift: número
Paso 6:  [0, (let, 3), (x, 8), (=, 12), (Expr, 18)] # Reduce: número → Expresion
Paso 7:  [0, (LetStmt, 25)]           # Reduce: let x = Expr → LetStmt
```

## 🌳 Construcción del AST

```
Input: let x = 5 + 3;

Paso 1: Parse literal "5"
    Number(5)

Paso 2: Parse operador "+"
    (esperando operando derecho)

Paso 3: Parse literal "3"
    Number(3)

Paso 4: Reduce: 5 + 3
    BinaryExpr(+)
    ├─ Number(5)
    └─ Number(3)

Paso 5: Reduce: let x = (5 + 3)
    LetStmt
    ├─ name: "x"
    └─ init:
        BinaryExpr(+)
        ├─ Number(5)
        └─ Number(3)
```

## 🔢 Tabla ACTION (Simplificada)

```
┌────────┬────────┬──────────┬──────────┬──────────┬─────────┐
│ Estado │   fn   │   let    │    {     │    }     │   EOF   │
├────────┼────────┼──────────┼──────────┼──────────┼─────────┤
│   0    │  S3    │   S5     │    -     │    -     │    -    │
│   1    │   -    │    -     │    -     │    -     │  ACCEPT │
│   2    │  S3    │   S5     │    -     │   R2     │   R2    │
│   3    │   -    │    -     │    -     │    -     │    -    │
│   5    │   -    │    -     │    -     │    -     │    -    │
└────────┴────────┴──────────┴──────────┴──────────┴─────────┘

Leyenda:
  S3  = Shift y pasar al estado 3
  R2  = Reduce usando la producción 2
  ACC = Aceptar (análisis exitoso)
  -   = Error sintáctico
```

## 🎯 Ejemplo Completo: `a + b * c`

### Tokenización
```
Tokens: [IDENT(a), PLUS, IDENT(b), STAR, IDENT(c)]
```

### Parsing con Precedencia

```
1. Parse a
   └─► Identifier(a)

2. Ver PLUS (precedencia: suma)
   └─► Esperar operando derecho

3. Parse b
   └─► Identifier(b)

4. Ver STAR (precedencia: multiplicación > suma)
   └─► Multiplicación tiene mayor precedencia
   └─► Parse primero b * c

5. Parse c
   └─► Identifier(c)

6. Reduce: b * c
   └─► BinaryExpr(*)
        ├─ Identifier(b)
        └─ Identifier(c)

7. Reduce: a + (b * c)
   └─► BinaryExpr(+)
        ├─ Identifier(a)
        └─ BinaryExpr(*)
            ├─ Identifier(b)
            └─ Identifier(c)
```

### AST Resultante

```
BinaryExpr(+)
├─ Identifier: a
└─ BinaryExpr(*)
   ├─ Identifier: b
   └─ Identifier: c

Correctamente parseado como: a + (b * c)
```

## 📊 Comparación: Tablas vs Recursivo

```
┌──────────────────────┬──────────────────┬────────────────────┐
│   Característica     │   Tablas LR      │   Recursivo Desc.  │
├──────────────────────┼──────────────────┼────────────────────┤
│ Complejidad          │ Alta             │ Media              │
│ Precedencia          │ En tablas        │ En funciones       │
│ Mantenimiento        │ Difícil          │ Fácil              │
│ Extensibilidad       │ Media            │ Alta               │
│ Errores              │ Numéricos        │ Descriptivos       │
│ Performance          │ Muy rápida       │ Rápida             │
│ Implementación       │ ~3000 líneas     │ ~1200 líneas       │
└──────────────────────┴──────────────────┴────────────────────┘

Nuestra implementación: Híbrida
  ✓ Estructura de pila (LR)
  ✓ Parsing recursivo (simplicidad)
  ✓ Lo mejor de ambos mundos
```

## 🔍 Algoritmo de Parsing (Pseudocódigo)

```python
def parse_expression():
    # Parsing con precedencia
    return parse_assignment()

def parse_assignment():
    left = parse_logical_or()
    
    if current_token in [=, +=, -=, *=, /=, %=]:
        op = current_token
        advance()
        right = parse_assignment()  # Asociativo derecha
        return BinaryExpr(op, left, right)
    
    return left

def parse_logical_or():
    left = parse_logical_and()
    
    while current_token == '||':
        op = current_token
        advance()
        right = parse_logical_and()
        left = BinaryExpr(op, left, right)
    
    return left

# ... más funciones de precedencia ...

def parse_primary():
    if current_token == NUMBER:
        value = current_token.lexeme
        advance()
        return Number(value)
    
    if current_token == IDENTIFIER:
        name = current_token.lexeme
        advance()
        return Identifier(name)
    
    if current_token == '(':
        advance()
        expr = parse_expression()
        expect(')')
        return expr
    
    error("Expresión esperada")
```

## 🎨 Visualización del AST

```
fn main() {
    let x = 5;
    if x > 3 {
        return x + 1;
    }
}

AST Generado:

Program
└─ Function: main
   └─ Block
      ├─ LetStmt: x
      │  └─ Number: 5
      └─ IfStmt
         ├─ Condition:
         │  └─ BinaryExpr: >
         │     ├─ Identifier: x
         │     └─ Number: 3
         └─ Then:
            └─ Block
               └─ ReturnStmt
                  └─ BinaryExpr: +
                     ├─ Identifier: x
                     └─ Number: 1

Representación Visual:

         [Program]
             |
        [Function:main]
             |
          [Block]
          /     \
    [LetStmt]  [IfStmt]
        |       /    \
    [Number:5] Cond  Then
               |      |
             [>]   [Block]
            /  \      |
         [x]  [3]  [Return]
                      |
                    [+]
                   /   \
                 [x]   [1]
```

## 📈 Flujo de Datos

```
┌────────────┐
│   LEXER    │
│            │
│ Genera     │
│ Tokens     │
└─────┬──────┘
      │
      ▼
┌────────────────────────────┐
│        PARSER              │
│                            │
│  ┌──────────────────────┐ │
│  │   1. Leer Token      │ │
│  └──────────┬───────────┘ │
│             ▼              │
│  ┌──────────────────────┐ │
│  │ 2. Consultar Pila    │ │
│  └──────────┬───────────┘ │
│             ▼              │
│  ┌──────────────────────┐ │
│  │ 3. Aplicar Acción:   │ │
│  │   - Shift (empujar)  │ │
│  │   - Reduce (reducir) │ │
│  └──────────┬───────────┘ │
│             ▼              │
│  ┌──────────────────────┐ │
│  │ 4. Construir Nodo    │ │
│  │    del AST           │ │
│  └──────────┬───────────┘ │
│             ▼              │
│  ┌──────────────────────┐ │
│  │ 5. ¿EOF? ──► Sí ──┐  │ │
│  │          └► No ──┐ │  │ │
│  └───────────────┘ │ │  │ │
│                  │ │ │  │ │
└──────────────────┼─┼─┼──┘ │
                   │ │ │    │
                   └─┘ │    │
                       ▼    ▼
                   [Loop]  [AST]
```

## 💡 Conceptos Clave

### Shift (Desplazamiento)
```
Acción: Mover el token actual a la pila
Efecto: Avanzar en la lectura de entrada

Antes:  Pila=[0, 5]     Input=[let, x, =, ...]
                        ^
Shift:  
Después: Pila=[0, 5, let]  Input=[x, =, ...]
                                   ^
```

### Reduce (Reducción)
```
Acción: Aplicar una producción de la gramática
Efecto: Combinar símbolos en un no-terminal

Producción: Expr → Term + Term

Antes:  Pila=[..., Term, +, Term]
Reduce:
Después: Pila=[..., Expr]

AST: Crea BinaryExpr(+, Term1, Term2)
```

## 🎯 Ventajas de Este Diseño

1. ✅ **Compatible** con tu lexer existente
2. ✅ **Extensible** - Fácil agregar nuevas construcciones
3. ✅ **Mantenible** - Código claro y organizado
4. ✅ **Eficiente** - Parsing en una sola pasada
5. ✅ **Robusto** - Manejo de errores con mensajes claros
6. ✅ **Documentado** - Comentarios y documentación completa

---

**Autor**: Sistema de Parser Ascendente LR
**Fecha**: 2025
**Versión**: 1.0
