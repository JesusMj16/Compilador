# Guía de uso (versión automatizada)

Esta rama usa **Flex + Bison**. La interfaz del ejecutable es deliberadamente mínima: recibe un archivo y devuelve `PARSE_OK` si no hay errores léxicos/sintácticos/semánticos, o `PARSE_FAIL` en caso contrario.

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

## Mensajes de error

El compilador reporta:

- Errores léxicos como `Error léxico [linea:col]: ...`
- Errores sintácticos como `Error sintáctico [linea:col]: ...`
- Errores semánticos como `Error semántico [linea:col]: ...`

