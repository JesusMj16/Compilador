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

# Guía de uso (versión automatizada)

Esta rama usa **Flex + Bison**. La interfaz del ejecutable es deliberadamente mínima: recibe un archivo y devuelve `PARSE_OK` o `PARSE_FAIL`.

## Compilar

Windows (MinGW):

```powershell
mingw32-make all
```

Linux/macOS:

```bash
make all
```

## Ejecutar

Ejemplos de entrada (programas) para el parser:

- `docs/Analizador-sintactico/examples-bison/`

Ejecutar:

```powershell
mingw32-make run-file FILE=docs/Analizador-sintactico/examples-bison/parse-exito-01.txt
```

## Pruebas del parser

Casos de prueba con expected output:

- `docs/Analizador-sintactico/examples-bison/`
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
<!-- Se eliminó contenido legacy (flags/AST/rutas antiguas) para mantener solo la versión automatizada. -->

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

