# Parser Ascendente LR - Documentación

Este directorio contiene la documentación completa del **Parser Ascendente (Bottom-Up)** implementado para el compilador.

## 📚 Documentos Disponibles

### 1. [RESUMEN.md](RESUMEN.md) - **Empieza Aquí** 
**Resumen ejecutivo de la implementación**
- Lista de componentes implementados
- Guía rápida de uso
- Ejemplos prácticos
- Checklist completo

### 2. [parser-doc.md](parser-doc.md) - **Documentación Técnica Detallada**
**Documentación completa del parser**
- Arquitectura del parser
- Descripción de la pila
- Estructura del AST
- Matriz de transiciones (ACTION y GOTO)
- Algoritmo de parsing
- API completa

### 3. [DIAGRAMAS.md](DIAGRAMAS.md) - **Visualizaciones**
**Diagramas y ejemplos visuales**
- Flujo del compilador
- Estructura de la pila
- Construcción del AST
- Tablas de transición
- Ejemplos paso a paso
- Comparaciones de enfoques

### 4. [gramatica.md](gramatica.md) - **Gramática del Lenguaje**
**Gramática EBNF completa**
- Definición de todas las producciones
- Reglas de sintaxis
- Estructura del lenguaje

### 5. [tabla-pi-pd.md](tabla-pi-pd.md) - **Conjuntos FIRST y LAST**
**Análisis de la gramática**
- Conjuntos Primera Izquierda (PI/FIRST)
- Conjuntos Primera Derecha (PD/LAST)
- Análisis de no terminales

## 🚀 Inicio Rápido

### Compilar el Proyecto
```bash
cd /home/dante/Documents/Universidad/Compiladores/Compilador
make clean && make all
```

### Ejecutar el Parser
```bash
# Solo análisis sintáctico
./bin/compilador -p archivo.txt

# Con estadísticas
./bin/compilador -p -s archivo.txt

# Análisis completo (léxico + sintáctico)
./bin/compilador archivo.txt
```

## 📖 Orden de Lectura Recomendado

Para entender completamente el parser, se recomienda leer en este orden:

1. **RESUMEN.md** - Para obtener una visión general
2. **DIAGRAMAS.md** - Para entender visualmente cómo funciona
3. **parser-doc.md** - Para detalles técnicos
4. **gramatica.md** - Para la gramática completa
5. **tabla-pi-pd.md** - Para análisis formal

## 🎯 Componentes Principales

### 1. 📚 Pila (Stack)
- Mantiene estados LR
- Almacena nodos parciales del AST
- Implementación dinámica con crecimiento automático

### 2. 🌳 Árbol de Sintaxis Abstracta (AST)
- Representa la estructura del programa
- 20+ tipos de nodos
- Funciones de construcción y liberación

### 3. 🔢 Matriz de Transiciones
- Tabla ACTION (shift, reduce, accept, error)
- Tabla GOTO (transiciones de estados)
- 66 producciones de la gramática

## 📊 Ejemplos

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

## 🔧 Archivos de Código

```
include/
  parser.h              # Declaraciones del parser

src/parser/
  parser.c              # Implementación completa

src/
  main.c                # Integración con main

Makefile                # Reglas de compilación
```

## 📈 Estadísticas

El parser puede mostrar estadísticas de su ejecución:

```
📊 Estadísticas del Parser:
   Desplazamientos (shift): 45
   Reducciones (reduce):    23
   Tamaño de pila:          1
```

## ✅ Características

- [x] Parsing ascendente (bottom-up)
- [x] Pila dinámica con estados LR
- [x] Construcción de AST
- [x] Precedencia de operadores
- [x] Manejo de errores descriptivo
- [x] Compatible con lexer existente
- [x] Extensible y mantenible
- [x] Completamente documentado

## 🎓 Conceptos Cubiertos

- **Parser LR**: Left-to-right, Rightmost derivation
- **Shift-Reduce**: Operaciones fundamentales
- **AST**: Árbol de Sintaxis Abstracta
- **Precedencia**: Orden de evaluación de operadores
- **Tablas de transición**: ACTION y GOTO
- **Producciones**: Reglas de la gramática
- **Análisis ascendente**: Bottom-up parsing

## 📞 Soporte

Para más información:
- Ver código fuente comentado en `src/parser/parser.c`
- Consultar header file en `include/parser.h`
- Revisar ejemplos en `docs/Analizador-Lexico/examples/`

## 🔮 Trabajo Futuro

- Parámetros de función completos
- Match expressions
- For loops con iteradores
- Arrays y acceso a elementos
- Tipos genéricos
- Recuperación de errores mejorada
- Tablas LR completas (opcional)

---

**Parser Ascendente LR v1.0**
Compatible con tu Lexer existente
Implementación completa con Pila, AST y Matriz de Transiciones
