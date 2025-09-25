# Analizador Léxico con AFD

## Descripción
Este proyecto corresponde a la materia de **Compiladores**.  
El objetivo es **diseñar e implementar un analizador léxico utilizando un Autómata Finito Determinista (AFD)**.  

El analizador recibirá un archivo fuente escrito en un mini-lenguaje y generará como salida:  
- Lista de tokens reconocidos (con lexema, tipo de token, línea y columna).  
- Archivos de tokens en formato numérico para el analizador sintáctico.
- Manejo robusto de errores léxicos.

---

## Tecnologías utilizadas
- **Lenguaje**: C11 (ISO/IEC 9899:2011)
- **Compilador**: GCC con flags `-Wall -Wextra -std=c11`
- **Control de versiones**: Git y GitHub  
- **Build system**: Makefile con compilación modular
- **Documentación**: Doxygen, Markdown y PDF  
- **Modelado de autómatas**: JFLAP

---

## 🚀 Instalación y Configuración

### Prerrequisitos
Para compilar y ejecutar este proyecto en una máquina Linux, necesitas:

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential gcc make git

# Fedora/RHEL/CentOS
sudo dnf install gcc make git

# Arch Linux
sudo pacman -S gcc make git
```

### Descargar el Proyecto

1. **Clonar el repositorio**:
```bash
git clone https://github.com/JesusMj16/Compilador.git
cd Compilador
```

2. **Verificar la estructura del proyecto**:
```bash
ls -la
# Deberías ver: src/, include/, docs/, Makefile, README.md, etc.
```

---

## Compilación

### Compilación Básica
```bash
# Compilar el proyecto completo
make

# Limpiar archivos compilados
make clean

# Recompilar desde cero
make clean && make
```

### Estructura de Compilación
```
build/           # Archivos objeto (.o)
├── main.o
└── lexer/
    ├── lexer.o
    └── keywords.o

bin/             # Ejecutable final
└── compilador
```

### Verificar Compilación
```bash
# El ejecutable se genera en bin/compilador
./bin/compilador -h
```

---

## Uso del Compilador

### Comandos Disponibles

#### Análisis Léxico (Terminal)
Analiza un archivo y muestra los tokens en pantalla:
```bash
./bin/compilador <archivo>

# Ejemplo:
./bin/compilador docs/Analizador-Lexico/examples/exito-01.txt
```

**Salida esperada**:
```
=== ANÁLISIS LÉXICO ===
Archivo: docs/Analizador-Lexico/examples/exito-01.txt

Línea Columna  Tipo         Lexema
-----  -------  ----         ------
1      1        KEYWORD      fn
1      4        IDENTIFIER   main
1      8        DELIMITER    (
1      9        DELIMITER    )
...
Total de tokens: 64
```

#### Generar Archivo de Tokens
Genera un archivo de tokens en formato numérico para el parser:
```bash
./bin/compilador -t <archivo>

# Ejemplo:
./bin/compilador -t docs/Analizador-Lexico/examples/exito-01.txt
```

**Salida**: Se crea `docs/Analizador-sintactico/archivos_parser/exito-01_tokens.txt`

#### Ayuda
```bash
./bin/compilador -h
./bin/compilador --help
```

---

## Archivos de Prueba

El proyecto incluye varios archivos de ejemplo en `docs/Analizador-Lexico/examples/`:

### Casos de Éxito
```bash
./bin/compilador docs/Analizador-Lexico/examples/exito-01.txt  # Programa básico
./bin/compilador docs/Analizador-Lexico/examples/exito-02.txt  # Estructuras de control
./bin/compilador docs/Analizador-Lexico/examples/exito-03.txt  # Operaciones matemáticas
./bin/compilador docs/Analizador-Lexico/examples/exito-04.txt  # Funciones y arrays
```

### Casos Límite
```bash
./bin/compilador docs/Analizador-Lexico/examples/limit-01.txt  # Archivo vacío
./bin/compilador docs/Analizador-Lexico/examples/limit-02.txt  # Solo espacios
./bin/compilador docs/Analizador-Lexico/examples/limit-03.txt  # Cadenas complejas
./bin/compilador docs/Analizador-Lexico/examples/limit-04.txt  # 200 líneas (prueba de rendimiento)
```

### Casos de Error
```bash
./bin/compilador docs/Analizador-Lexico/examples/error-01.txt  # Caracteres inválidos
./bin/compilador docs/Analizador-Lexico/examples/error-02.txt  # Tokens mal formados
./bin/compilador docs/Analizador-Lexico/examples/error-03.txt  # Cadenas sin cerrar
./bin/compilador docs/Analizador-Lexico/examples/error-04.txt  # Números inválidos
```

---

## Características del Lenguaje

### Tipos de Tokens Reconocidos
- **IDENTIFIER**: Variables y nombres de función (`main`, `contador`, `resultado`)
- **NUMBER**: Enteros, reales, hexadecimales, binarios (`42`, `3.14`, `0xFF`, `0b1010`)
- **STRING**: Cadenas de texto (`"Hola mundo"`, `'c'`)
- **OPERATOR**: Operadores (`+`, `-`, `*`, `/`, `=`, `==`, `!=`, `>`, `<`, etc.)
- **DELIMITER**: Delimitadores (`(`, `)`, `{`, `}`, `[`, `]`, `;`, `,`)
- **KEYWORD**: Palabras reservadas (15 total)
- **UNKNOWN**: Tokens no reconocidos
- **EOF**: Fin de archivo

### Palabras Reservadas (Keywords)
```rust
fn let mut if else match while loop for in break continue return true false
```

### Ejemplos de Código Válido
```rust
fn main() {
    let mut contador: i32 = 0;
    let limite: i32 = 100;
    
    while contador < limite {
        if contador % 2 == 0 {
            contador = contador + 1;
        } else {
            break;
        }
    }
    
    let resultado = fibonacci(contador);
    return resultado;
}
```

---

## Formato de Salida

### Archivo de Tokens (Formato Numérico)
Al usar `-t`, se genera un archivo con formato:
```
# Comentarios explicativos
# Tipos: IDENTIFIER=0, NUMBER=1, STRING=2, OPERATOR=3, DELIMITER=4, KEYWORD=5, UNKNOWN=6, EOF=7
5 fn 1 1 0
0 main 1 4
4 ( 1 8
4 ) 1 9
...
```

**Formato**: `tipo_token lexema linea columna [indice_keyword]`

---

## Targets del Makefile

```bash
make              # Compilar proyecto completo
make clean        # Limpiar archivos compilados
make tokens       # Generar tokens de archivos de ejemplo
make test         # Ejecutar todas las pruebas
make help         # Mostrar ayuda del Makefile
```

---

## Desarrollo

### Estructura del Código
```
src/
├── main.c              # Interfaz principal
└── lexer/
    ├── lexer.c         # Analizador léxico principal
    └── keywords.c      # Manejo de palabras reservadas

include/
├── lexer.h             # Definiciones principales
└── keywords.h          # Definiciones de keywords
```

### Compilar con Debug
```bash
gcc -Wall -Wextra -std=c11 -g -DDEBUG -Iinclude src/main.c src/lexer/*.c -o debug_compilador
```

---

## Documentación Adicional

### Generar Documentación con Doxygen
```bash
# Si tienes Doxygen instalado
doxygen Doxyfile

# La documentación se genera en docs/html/index.html
```

### Archivos de Documentación
- **Automátas**: `docs/Analizador-Lexico/automatas/` - Diseños en JFLAP
- **Especificaciones**: `docs/Analizador-Lexico/docs/` - Reglas, tokens, reportes
- **Ejemplos**: `docs/Analizador-Lexico/examples/` - Casos de prueba

---

## Solución de Problemas

### Error: "command not found"
```bash
# Verificar que el ejecutable existe
ls -la bin/compilador

# Usar ruta relativa o absoluta
./bin/compilador archivo.txt
```

### Error de Compilación
```bash
# Verificar dependencias
gcc --version
make --version

# Limpiar y recompilar
make clean && make
```

### Problemas con Archivos
```bash
# Verificar que el archivo existe
ls -la docs/Analizador-Lexico/examples/exito-01.txt

# Verificar permisos de lectura
chmod +r archivo.txt
```

---

## Estructura Académica

Este proyecto está desarrollado como parte del curso de **Compiladores** y incluye:

- **Análisis Léxico** - Implementado con AFD
- **Análisis Sintáctico** - En desarrollo
- **Análisis Semántico** - Planificado
- **Generación de Código** - Planificado

---

## Autores

**JesusMj16** - [GitHub](https://github.com/JesusMj16)
**NeilDMJ**   - [GitHub](https://github.com/NeilDMJ)

---

## Licencia

Este proyecto es desarrollado con fines académicos para la materia de Compiladores.

---

## Versión

**v1.0.0** - Analizador Léxico Completo
- Reconocimiento de 8 tipos de tokens
- 15 palabras reservadas 
- Generación de archivos para parser
- Interfaz de línea de comandos simplificada
- Manejo robusto de errores

---

*Última actualización: Septiembre 2025*

