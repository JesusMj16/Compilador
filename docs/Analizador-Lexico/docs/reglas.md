Este documento define las reglas léxicas del LENGUAJE REDUCIDO. Solo se consideran: funciones, declaraciones (con posible mutabilidad), asignaciones y expresiones aritmético-lógicas. Se eliminan ciclos, condicionales, patrón `match`, arreglos, cadenas y caracteres.

## Autómata determinista (DFA) usado por el lexer

El lexer ahora se implementa como un autómata determinista guiado por tablas. Cada carácter de la entrada se clasifica primero en una de las siguientes categorías: `LETTER`, `DIGIT`, `_`, operadores individuales (`+ - * / %`), comparadores (`= ! < >`), conectores (`& |`), delimitadores (`( ) { } ; , :`), punto (`.`), espacios (` `, `\t`), saltos de línea (`\n`, `\r`) y fin de archivo.

Con esa clasificación se consulta una tabla de transición explícita que cubre los estados principales:

- `AUTO_START`: punto de entrada para cada token. Decide entre identificadores, números, operadores/delimitadores y detecta comentarios.
- `AUTO_WHITESPACE`: acumula espacios y saltos de línea, reportando un estado aceptado que simplemente consume el bloque antes de volver a `AUTO_START`.
- `AUTO_SLASH`: desambiguador para `/`, el inicio de `//` o `/* */`.
- Estados de aceptación (`AUTO_ACCEPT_*`): indican qué analizador especializado debe ejecutarse (`lex_identifier_or_keyword`, `lex_number`, `lex_operator_or_delimiter`) o si debe consumirse un comentario/espacio.

El flujo es: se clasifica el prefijo usando la tabla, se ejecuta la rutina correspondiente y, si se detecta un identificador válido, el lexer lo registra inmediatamente en la tabla de símbolos. Este enfoque asegura que las reglas descritas más abajo se mantengan sincronizadas con el código, y facilita extender el lenguaje agregando columnas (tipos de carácter) o filas (estados) a la tabla.

## Identificadores
### Forma EBNF
```ebnf
identificador -> ( letra | '_' ) ( letra | dígito | '_' )*
letra         -> [a-zA-Z]
dígito        -> [0-9]
```

### Forma REGEX
```regex
[_a-zA-Z][_a-zA-Z0-9]*
```

---

## Números
### Forma EBNF
```ebnf
entero   -> signo? dígito+
signo    -> '+' | '-'
dígito   -> [0-9]
```

### Forma REGEX
```regex
[+-]?[0-9]+
// Hex y binario removidos en versión reducida
```

---

## Reales
### Forma EBNF
```ebnf
real      -> signo? dígito+ "." dígito+ ( exponente )?
exponente -> ( 'e' | 'E' ) signo? dígito+
```

### Forma REGEX
```regex
[+-]?[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?
```

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
salto_linea    -> '\n' | '\r\n'
```

### Forma REGEX
```regex
[\t\n\r ]+
```
