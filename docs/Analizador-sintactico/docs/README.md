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

El ejecutable principal parsea un archivo y reporta `PARSE_OK` si no hay errores léxicos/sintácticos/semánticos, o `PARSE_FAIL` en caso contrario.

En Windows (MinGW):

```powershell
mingw32-make run-file FILE=docs/Analizador-sintactico/examples-bison/parse-exito-01.txt
```

En Linux/macOS:

```bash
make run-file FILE=docs/Analizador-sintactico/examples-bison/parse-exito-01.txt
```

Nota: esta rama no construye AST ni expone estadísticas; se centra en reconocimiento y validaciones mínimas.
