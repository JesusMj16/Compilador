# Gramática (EBNF simplificada, sin recursión por la izquierda)

Esta gramática describe el LENGUAJE REDUCIDO: solo funciones, declaraciones (`let`/`mut`), asignaciones simples (`=`), `return` y expresiones aritmético-lógicas (con comparaciones). No hay condicionales, ciclos, `match`, arreglos, cadenas ni caracteres.

Convenciones visuales
- Se usa la notación con corchetes angulares y `::=` al estilo del ejemplo adjunto.
- Opcionalidad con `[ ... ]` y repetición con `{ ... }`.
- Tipos: `i32 | f64 | bool | IDENT`.

Programa
Programa ::= ListaItems EOF

Lista de items
ListaItems ::= Item ListaItems| epsilon
Item       ::= Funcion | Sentencia

Funciones y parámetros
Funcion            ::= 'fn' IDENT '(' ListaParametrosOpt ')' Bloque
ListaParametrosOpt ::= ListaParametros| epsilon
ListaParametros     ::= Parametro { ',' Parametro }
Parametro           ::= IDENT ':' Tipo
Tipo                ::= 'i32' | 'f64' | 'bool' | IDENT

Bloques y sentencias
Bloque          ::= '{' ListaSentencias '}'
ListaSentencias ::= Sentencia ListaSentencias| epsilon
Sentencia       ::= LetSentencia ';'| ExprSentencia ';'| Bloque| ReturnSentencia ';'
LetSentencia::= 'let' [ 'mut' ] IDENT [ ':' Tipo ] [ '=' Expresion ]
ExprSentencia   ::= Expresion
ReturnSentencia ::= 'return' [ Expresion ]

Expresiones (precedencia y asociatividad)
Expresion     ::= Asignacion
Asignacion    ::= LogicoOR [ '=' Asignacion ]
LogicoOR      ::= LogicoAND { '||' LogicoAND }
LogicoAND     ::= Igualdad { '&&' Igualdad }
Igualdad      ::= Comparacion { ( '==' | '!=' ) Comparacion }
Comparacion   ::= Aditivo { ( '' | '' | '=' | '=' ) Aditivo }
Aditivo       ::= Multiplicativo { ( '+' | '-' ) Multiplicativo }
Multiplicativo::= Unario { ( '*' | '/' | '%' ) Unario }
Unario        ::= ( '!' | '-' | '+' ) Unario| Postfijo

Postfijos y llamadas
Postfijo            ::= Primario { Llamada }
Llamada             ::= '(' ListaArgumentosOpt ')'
ListaArgumentosOpt  ::= ListaArgumentos| epsilon
ListaArgumentos     ::= Expresion { ',' Expresion }

Primarios y literales
Primario ::= Literal| IDENT | '(' Expresion ')'

Literal  ::= NUMBER | Booleano
Booleano ::= 'true' | 'false'

Notas
- Estructura libre de recursión por la izquierda gracias a `[ ... ]` y `{ ... }`.
- Asignación es asociativa a la derecha: `a = b = c`.
- Las llamadas a función se modelan con `Postfijo>` para admitir anidamiento.
