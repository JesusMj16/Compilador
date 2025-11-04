# Guía de Uso del Parser

## Introducción

## Comandos Básicos

### 1. Análisis Sintáctico Simple

```bash
./bin/compilador -p archivo.txt
```

**Qué hace:**
- Lee el archivo fuente
- Ejecuta el lexer para obtener tokens
- Ejecuta el parser para construir el AST
- Muestra el árbol de sintaxis abstracta

**Ejemplo:**
```bash
./bin/compilador -p test_parser.txt
```

**Salida esperada:**
```
=== ANÁLISIS SINTÁCTICO ===
Archivo: test_parser.txt

ÁRBOL DE SINTAXIS ABSTRACTA (AST):
═══════════════════════════════════════

Program (1 children)
  Function: main
    Block (2 children)
      LetStmt: x
        Number: 5
      ...
```

### 2. Análisis Sintáctico con Estadísticas

```bash
./bin/compilador -p -s archivo.txt
```

**Qué hace:**
- Todo lo del comando anterior
- Además muestra estadísticas del parser

**Ejemplo:**
```bash
./bin/compilador -p -s test_parser.txt
```

**Salida adicional:**
```
   Estadísticas del Parser:
   Desplazamientos (shift): 45
   Reducciones (reduce):    23
   Tamaño de pila:          1
```

### 3. Solo Análisis Léxico

```bash
./bin/compilador -l archivo.txt
```

**Qué hace:**
- Solo ejecuta el lexer
- Muestra la tabla de tokens

**Salida esperada:**
```
=== ANÁLISIS LÉXICO ===
Archivo: archivo.txt

Línea Columna  Tipo         Lexema
-----  -------  ----         ------
1      1        KW_FN        fn
1      4        IDENT        main
...
```

### 4. Análisis Completo (Recomendado)

```bash
./bin/compilador archivo.txt
```

**Qué hace:**
- Ejecuta análisis léxico
- Ejecuta análisis sintáctico
- Muestra ambos resultados

**Salida esperada:**
```
╔════════════════════════════════════════════╗
║   COMPILADOR - ANÁLISIS COMPLETO          ║
╚════════════════════════════════════════════╝

Fase 1: Análisis Léxico
────────────────────────────
...

Fase 2: Análisis Sintáctico
────────────────────────────
...

╔════════════════════════════════════════════╗
║       COMPILACIÓN EXITOSA                  ║
╚════════════════════════════════════════════╝
```

### 5. Generar Archivo de Tokens

```bash
./bin/compilador -t archivo.txt
```

**Qué hace:**
- Genera un archivo con todos los tokens
- Útil para debug y testing

**Archivo generado:**
```
docs/Analizador-sintactico/archivos_parser/archivo_tokens.txt
```

## Ejemplos Prácticos

### Ejemplo 1: Programa Simple

**Archivo: `simple.txt`**
```rust
fn main() {
    let x = 5;
    let y = 10;
    let suma = x + y;
}
```

**Comando:**
```bash
./bin/compilador -p simple.txt
```

**AST Generado:**
```
Program (1 children)
  Function: main
    Block (3 children)
      LetStmt: x
        Number: 5
      LetStmt: y
        Number: 10
      LetStmt: suma
        BinaryExpr: +
          Identifier: x
          Identifier: y
```

### Ejemplo 2: Condicional

**Archivo: `condicional.txt`**
```rust
fn test() {
    let edad = 20;
    if edad >= 18 {
        let es_mayor = true;
    }
}
```

**Comando:**
```bash
./bin/compilador -p -s condicional.txt
```

**AST Generado:**
```
Program (1 children)
  Function: test
    Block (2 children)
      LetStmt: edad
        Number: 20
      IfStmt
        Condition:
          BinaryExpr: >=
            Identifier: edad
            Number: 18
        Then:
          Block (1 children)
            LetStmt: es_mayor
              Bool: true
```

### Ejemplo 3: Expresiones Complejas

**Archivo: `expresiones.txt`**
```rust
fn calcular() {
    let a = 5;
    let b = 3;
    let c = 2;
    let resultado = a + b * c;
}
```

**Comando:**
```bash
./bin/compilador -p expresiones.txt
```

**AST Generado (nota la precedencia correcta):**
```
LetStmt: resultado
  BinaryExpr: +
    Identifier: a
    BinaryExpr: *
      Identifier: b
      Identifier: c

# Correcto: a + (b * c), no (a + b) * c
```

### Ejemplo 4: Bucle While

**Archivo: `loop.txt`**
```rust
fn contar() {
    let contador = 0;
    while contador < 10 {
        let temp = contador + 1;
    }
}
```

**Comando:**
```bash
./bin/compilador -p loop.txt
```

**AST Generado:**
```
Program (1 children)
  Function: contar
    Block (2 children)
      LetStmt: contador
        Number: 0
      WhileStmt
        Condition:
          BinaryExpr: <
            Identifier: contador
            Number: 10
        Body:
          Block (1 children)
            LetStmt: temp
              BinaryExpr: +
                Identifier: contador
                Number: 1
```

## Casos de Uso Comunes

### Verificar Sintaxis

```bash
# Solo quiero saber si mi código es sintácticamente correcto
./bin/compilador -p mi_codigo.txt
```

Si no hay errores, verás:
```
 Análisis sintáctico exitoso
```

Si hay errores, verás:
```
   Error de parsing en línea X, columna Y:
   [mensaje descriptivo]
```

### Debug de Precedencia

```bash
# Quiero verificar cómo se parsean mis expresiones
./bin/compilador -p mi_codigo.txt
```

Busca en el AST las expresiones binarias para verificar el orden.

### Generar Tokens para Otra Herramienta

```bash
# Genero tokens para procesarlos con otra herramienta
./bin/compilador -t mi_codigo.txt
```

El archivo generado tendrá formato:
```
tipo_token nombre_token lexema linea columna
```

## Manejo de Errores

### Error: Se esperaba punto y coma

**Código con error:**
```rust
fn main() {
    let x = 5  // Falta el ;
}
```

**Salida:**
```
   Error de parsing en línea 2, columna 14:
   Se esperaba ';' después de la sentencia let
```

### Error: Se esperaba llave de cierre

**Código con error:**
```rust
fn main() {
    let x = 5;
    // Falta el }
```

**Salida:**
```
   Error de parsing en línea 3, columna 1:
   Se esperaba '}' al final del bloque
```

### Error: Paréntesis sin cerrar

**Código con error:**
```rust
fn main() {
    let x = (5 + 3;  // Falta el )
}
```

**Salida:**
```
   Error de parsing en línea 2, columna 19:
   Se esperaba ')' después de la expresión
```

## Interpretar las Estadísticas

Cuando usas `-s`, obtienes:

```
   Estadísticas del Parser:
   Desplazamientos (shift): 45
   Reducciones (reduce):    23
   Tamaño de pila:          1
```

**¿Qué significa?**

- **Desplazamientos (shift)**: Número de veces que se movieron tokens a la pila
- **Reducciones (reduce)**: Número de veces que se aplicaron producciones de la gramática
- **Tamaño de pila**: Debería ser 1 al final (estado inicial + programa completo)

**Reglas generales:**
- Más tokens = más shifts
- Más estructura = más reduces
- Pila final = 1 (éxito)

## Flujo de Trabajo Recomendado

### 1. Desarrollo Inicial
```bash
# Verificar tokens primero
./bin/compilador -l mi_codigo.txt

# Luego verificar sintaxis
./bin/compilador -p mi_codigo.txt
```

### 2. Debug de Problemas
```bash
# Análisis completo para ver todo
./bin/compilador mi_codigo.txt

# Con estadísticas para análisis profundo
./bin/compilador -p -s mi_codigo.txt
```

### 3. Testing Automatizado
```bash
# Generar tokens para comparación
./bin/compilador -t test_caso1.txt
./bin/compilador -t test_caso2.txt

# Verificar sintaxis de múltiples archivos
for file in tests/*.txt; do
    echo "Probando: $file"
    ./bin/compilador -p "$file"
done
```

## Ejemplos del Repositorio

Puedes probar con los ejemplos incluidos:

```bash
# Ejemplos de éxito
./bin/compilador -p docs/Analizador-Lexico/examples/exito-01.txt
./bin/compilador -p docs/Analizador-Lexico/examples/exito-02.txt
./bin/compilador -p docs/Analizador-Lexico/examples/exito-03.txt

# Ejemplos de límite
./bin/compilador -p docs/Analizador-Lexico/examples/limit-01.txt
```

## Tips y Trucos

### Redirigir Salida a Archivo

```bash
# Guardar AST en archivo
./bin/compilador -p mi_codigo.txt > ast_output.txt

# Guardar solo errores
./bin/compilador -p mi_codigo.txt 2> errores.txt

# Guardar todo
./bin/compilador -p mi_codigo.txt &> completo.txt
```

### Comparar ASTs

```bash
# Comparar dos versiones
./bin/compilador -p version1.txt > ast1.txt
./bin/compilador -p version2.txt > ast2.txt
diff ast1.txt ast2.txt
```

### Verificar Múltiples Archivos

```bash
# Crear script de prueba
for file in *.txt; do
    echo "=== $file ==="
    ./bin/compilador -p "$file"
    echo ""
done
```

## Casos de Uso Avanzados

### 1. Integración en Pipeline

```bash
# Pipeline completo
cat input.txt | \
  ./bin/compilador -l - | \
  grep "IDENT" | \
  wc -l
```

### 2. Testing Continuo

```bash
# Watch mode (requiere inotifywait)
while inotifywait -e modify mi_codigo.txt; do
    clear
    ./bin/compilador -p mi_codigo.txt
done
```

### 3. Comparación de Performance

```bash
# Medir tiempo
time ./bin/compilador -p archivo_grande.txt

# Con estadísticas
./bin/compilador -p -s archivo_grande.txt
```

