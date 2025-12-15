Este documento define las reglas léxicas del LENGUAJE REDUCIDO. Se consideran: funciones, declaraciones (con posible mutabilidad), asignaciones, expresiones aritmético-lógicas y condicionales `if`/`else`, además de `return`. Se eliminan ciclos, patrón `match`, arreglos, cadenas y caracteres.

## Modelo (DFA) usado por el lexer

El lexer puede verse como un autómata determinista (DFA). En una implementación con Flex, el DFA se genera automáticamente a partir de expresiones regulares. Cada carácter de la entrada se clasifica primero en una de las siguientes categorías: `LETTER`, `DIGIT`, `_`, operadores individuales (`+ - * / %`), comparadores (`= ! < >`), conectores (`& |`), delimitadores (`( ) { } ; , :`), punto (`.`), espacios (` `, `\t`), saltos de línea (`\n`, `\r`) y fin de archivo.

En esta versión automatizada, no se mantiene una tabla de transición manual: Flex construye el DFA internamente a partir de las reglas REGEX definidas en el lexer. La prioridad se determina por (1) el lexema más largo posible y (2) el orden de las reglas dentro del archivo.

El contrato lexer→parser es:

- Las reglas viven en `src/automatizado/lexer/lexer.l`.
- El lexer devuelve tokens compatibles con Bison (definidos en `parser.tab.h`).
- Espacios en blanco y comentarios se consumen (no producen tokens).
- Se mantiene ubicación (`yylineno`, `yycolumn`) para reportar errores léxicos y para que el parser pueda reportar errores sintácticos.

## Identificadores
### Forma EBNF
```ebnf
identificador -> ( letra | '_' ) ( letra | dígito | '_' )*
letra         -> [a-zA-Z]
dígito        -> [0-9]
```

### Forma REGEX
```regex
[_a-zA-Z](?:[_a-zA-Z0-9]|_(?!_))*
```

Restricción adicional (normativa): un identificador NO puede contener la subcadena `__` (dos guiones bajos consecutivos).

---

## Números
### Forma EBNF
```ebnf
entero   -> dígito+
dígito   -> [0-9]
```

### Forma REGEX
```regex
[0-9]+
```

Nota: el signo (`+`/`-`) se tokeniza como operador (TOKEN_PLUS/TOKEN_MINUS). La negación/unario se resuelve en la fase sintáctica.

---

## Reales
### Forma EBNF
```ebnf
real      -> dígito+ "." dígito+ ( exponente )?
exponente -> ( 'e' | 'E' ) signo? dígito+
signo     -> '+' | '-'
```

### Forma REGEX
```regex
[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?
```

Nota: el punto (`.`) solo es válido dentro de un real con dígitos a ambos lados. Un `.` aislado se considera error léxico.

---

## Cadenas
### Forma EBNF
```ebnf
cadena   -> '"' ( caracter | escape )* '"'
caracter -> cualquier símbolo excepto " o \
escape   -> '\' ( 'n' | 't' | '"' | '\'' | '\\' )
```

### Forma REGEX
```regex
// Cadenas removidas
```

---

## Caracteres
### Forma EBNF
```ebnf
caracter_literal -> "'" ( caracter | escape ) "'"
```

### Forma REGEX
```regex
// Caracter literal removido
```

---

## Comentarios
### Forma EBNF
```ebnf
comentario_linea   -> "//" ( cualquier_caracter )* fin_linea
comentario_bloque  -> "/*" ( cualquier_caracter | salto_linea )* "*/"
```

### Forma REGEX
```regex
//.*
/\*([^*]|\*+[^*/])*\*+/
```

---

## Operadores
### Forma EBNF
```ebnf
operador_aritmetico -> '+' | '-' | '*' | '/' | '%'
operador_relacional -> '==' | '!=' | '<' | '>' | '<=' | '>='
operador_logico     -> '&&' | '||' | '!'
operador_asignacion -> '='
```

### Forma REGEX
```regex
(\+|\-|\*|/|%)
(==|!=|<=|>=|<|>)
(&&|\|\||!)
(=)
```

---

## Delimitadores
### Forma EBNF
```ebnf
delimitador -> '(' | ')' | '{' | '}' | ';' | ',' | ':'
```

### Forma REGEX
```regex
[(){};,:]
```

---

## Espacios en blanco
### Forma EBNF
```ebnf
espacio_blanco -> ' ' | '\t' | salto_linea
salto_linea    -> '\n' | '\r\n' | '\r'
```

### Forma REGEX
```regex
[\t\n\r ]+
```
