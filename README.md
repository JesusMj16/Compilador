# Compilador (versión automatizada)

Este repositorio contiene la versión **automatizada** del compilador usando **Flex** (analizador léxico) y **Bison** (analizador sintáctico).

## Estructura

- `src/automatizado/lexer/lexer.l`: lexer Flex.
- `src/automatizado/lexer/flex_runner.c`: runner para validar el lexer contra `.expected.txt`.
- `src/automatizado/parser/parser.y`: parser Bison.

## Docs y casos

- Lexer (especificación): `docs/Analizador-Lexico/docs/`
- Lexer (tests): `docs/Analizador-Lexico/examples-flex/`
- Parser (gramática): `docs/Analizador-sintactico/docs/`
- Parser (tests): `docs/Analizador-sintactico/examples-bison/`
- Programas de entrada (parser): `docs/Analizador-sintactico/archivos_parser/`

## Compilar

### Windows (MinGW)

Requiere `mingw32-make`, `gcc`, `flex` y `bison` en `PATH`.

```powershell
mingw32-make clean
mingw32-make all
```

### Linux/macOS

```bash
make clean
make all
```

## Ejecutar

El ejecutable parsea un archivo y reporta `PARSE_OK` o `PARSE_FAIL`.

```powershell
mingw32-make run-file FILE=docs/Analizador-sintactico/archivos_parser/exito-01.txt
```

## Pruebas (lexer)

```powershell
mingw32-make test-lexer
```

