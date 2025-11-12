# Gramática (modo producción / BNF, sin recursión por la izquierda)

BNF derivada de la EBNF anterior, sin operadores `{}` ni `[]`, apta para parser LL/LR. Mantiene la misma semántica y precedencia.

```bnf
Programa -> ListaItems EOF
ListaItems -> Item ListaItems | epsilon
Item -> Funcion | Sentencia

Funcion -> 'fn' IDENT '(' ListaParametrosOpt ')' Bloque
ListaParametrosOpt -> ListaParametros | epsilon
ListaParametros -> Parametro ListaParametrosTail
ListaParametrosTail -> ',' Parametro ListaParametrosTail | epsilon
Parametro -> IDENT ':' Tipo
Tipo -> 'i32' | 'f64' | 'bool' | IDENT

Bloque -> '{' ListaSentencias '}'
ListaSentencias -> Sentencia ListaSentencias | epsilon
Sentencia -> LetSentencia ';'
          | ExprSentencia ';'
          | Bloque
          | ReturnSentencia ';'

LetSentencia -> 'let' MutOpt IDENT AnotacionTipoOpt InicializacionOpt
MutOpt -> 'mut' | epsilon
AnotacionTipoOpt -> ':' Tipo | epsilon
InicializacionOpt -> '=' Expresion | epsilon

ExprSentencia -> Expresion
ReturnSentencia -> 'return' ExpresionOpt
ExpresionOpt -> Expresion | epsilon

Expresion -> Asignacion
Asignacion -> LogicoOR AsignacionTail
AsignacionTail -> '=' Asignacion | epsilon

LogicoOR -> LogicoAND LogicoORTail
LogicoORTail -> '||' LogicoAND LogicoORTail | epsilon

LogicoAND -> Igualdad LogicoANDTail
LogicoANDTail -> '&&' Igualdad LogicoANDTail | epsilon

Igualdad -> Comparacion IgualdadTail
IgualdadTail -> ('==' | '!=') Comparacion IgualdadTail | epsilon

Comparacion -> Aditivo ComparacionTail
ComparacionTail -> ('<' | '>' | '<=' | '>=') Aditivo ComparacionTail | epsilon

Aditivo -> Multiplicativo AditivoTail
AditivoTail -> ('+' | '-') Multiplicativo AditivoTail | epsilon

Multiplicativo -> Unario MultiplicativoTail
MultiplicativoTail -> ('*' | '/' | '%') Unario MultiplicativoTail | epsilon

Unario -> OperadorUnario Unario | Postfijo
OperadorUnario -> '!' | '-' | '+'

Postfijo -> Primario PostfijoTail
PostfijoTail -> Llamada PostfijoTail | epsilon
Llamada -> '(' ListaArgumentosOpt ')'
ListaArgumentosOpt -> ListaArgumentos | epsilon
ListaArgumentos -> Expresion ListaArgumentosTail
ListaArgumentosTail -> ',' Expresion ListaArgumentosTail | epsilon

Primario -> Literal
         | IDENT
         | '(' Expresion ')'

Literal -> NUMBER | Booleano
Booleano -> 'true' | 'false'
```

Observaciones
- No hay recursión por la izquierda: se utilizaron reglas `X -> Y X'` con colas (`Tail`) para secuencias.
- `AsignacionTail` garantiza asociatividad a la derecha.
- Esta BNF es 1:1 con la EBNF de `gramatica-ebnf.md`.
