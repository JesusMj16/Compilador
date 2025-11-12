Este documento define las reglas léxicas del LENGUAJE REDUCIDO. Solo se consideran: funciones, declaraciones (con posible mutabilidad), asignaciones y expresiones aritmético-lógicas. Se eliminan ciclos, condicionales, patrón `match`, arreglos, cadenas y caracteres.

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
