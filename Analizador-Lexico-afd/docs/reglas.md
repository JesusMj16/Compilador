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
entero         -> signo? dígito+
signo          -> '+' | '-'
dígito         -> [0-9]

entero_binario -> signo? "0b" [0-1]+
entero_hex     -> signo? "0x" [0-9a-fA-F]+
```

### Forma REGEX
```regex
[+-]?[0-9]+
[+-]?0b[01]+
[+-]?0x[0-9a-fA-F]+
```

---

## Reales
### Forma EBNF
```ebnf
real -> signo? dígito+ "." dígito+ ( exponente )?
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
escape   -> '\\' ( 'n' | 't' | '"' | '\'' | '\\' )
```

### Forma REGEX
```regex
"([^"\\]|\\[nt"'])*"
```

---

## Caracteres
### Forma EBNF
```ebnf
caracter_literal -> "'" ( caracter | escape ) "'"
```

### Forma REGEX
```regex
'([^'\\]|\\[nt"'])'
```

---

## Comentarios
### Forma EBNF
```ebnf
comentario_linea   -> "//" ( cualquier_caracter )*
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
operador_asignacion -> '=' | '+=' | '-=' | '*=' | '/=' | '%='
```

### Forma REGEX
```regex
(\+|\-|\*|/|%)
(==|!=|<=|>=|<|>)
(&&|\|\||!)
(=|\+=|\-=|\*=|/=|%=)
```

---

## Delimitadores
### Forma EBNF
```ebnf
delimitador -> '(' | ')' | '{' | '}' | '[' | ']' | ';' | ',' | '.'
```

### Forma REGEX
```regex
[()\[\]{};,.]
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
