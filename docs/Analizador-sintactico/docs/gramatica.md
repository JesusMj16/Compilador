# Gramatica del Analizador Sintactico

Este documento describe la gramatica operativa que complementa al analizador lexico definido en `docs/Analizador-Lexico`. La notacion empleada es EBNF simplificada y sigue la misma linea editorial usada en `reglas.md`, enumerando las construcciones de alto nivel del lenguaje.

## Convenciones de tokens
- `IDENT` representa cualquier identificador valido emitido por el lexer.
- `NUMBER`, `STRING` y `CHAR` representan los literales numericos, de cadena y de caracter, respectivamente.
- Las palabras reservadas y operadores se muestran literalmente entre comillas simples.
- `epsilon` indica producciones vacias.

## Programa y declaraciones
```ebnf
Programa        -> ListaItems EOF
ListaItems      -> Item ListaItems | epsilon
Item            -> Funcion | Sentencia
Funcion         -> 'fn' IDENT '(' ListaParametrosOpt ')' Bloque
ListaParametrosOpt -> ListaParametros | epsilon
ListaParametros -> Parametro ListaParametrosTail
ListaParametrosTail -> ',' Parametro ListaParametrosTail | epsilon
Parametro       -> IDENT ':' Tipo
Tipo            -> 'i32' | 'f64' | 'bool' | 'char' | IDENT
```

## Bloques y sentencias
```ebnf
Bloque          -> '{' ListaSentencias '}'
ListaSentencias -> Sentencia ListaSentencias | epsilon
Sentencia       -> LetSentencia ';'
                 | ExprSentencia ';'
                 | IfSentencia
                 | LoopSentencia
                 | MatchSentencia
                 | Bloque
                 | ReturnSentencia ';'
                 | BreakSentencia ';'
                 | ContinueSentencia ';'
LetSentencia    -> 'let' MutOpt IDENT AnotacionTipoOpt InicializacionOpt
MutOpt          -> 'mut' | epsilon
AnotacionTipoOpt -> ':' Tipo | epsilon
InicializacionOpt -> '=' Expresion | epsilon
ExprSentencia   -> Expresion
IfSentencia     -> 'if' Expresion Bloque ElseOpt
ElseOpt         -> 'else' (IfSentencia | Bloque) | epsilon
LoopSentencia   -> WhileSentencia | ForSentencia | LoopForever
WhileSentencia  -> 'while' Expresion Bloque
ForSentencia    -> 'for' IDENT 'in' Expresion Bloque
LoopForever     -> 'loop' Bloque
ReturnSentencia -> 'return' ExpresionOpt
BreakSentencia  -> 'break'
ContinueSentencia -> 'continue'
ExpresionOpt    -> Expresion | epsilon

MatchSentencia  -> 'match' Expresion '{' ListaMatchBrazos '}'
ListaMatchBrazos -> MatchBrazo ListaMatchBrazosTail
ListaMatchBrazosTail -> MatchBrazo ListaMatchBrazosTail | epsilon
MatchBrazo      -> MatchPatron '=>' MatchResultado ';'
MatchPatron     -> Literal | IDENT
MatchResultado  -> Bloque | Expresion
```

## Expresiones
```ebnf
Expresion       -> Asignacion
Asignacion      -> LogicoOR AsignacionTail
AsignacionTail  -> OperadorAsignacion Asignacion | epsilon
OperadorAsignacion -> '=' | '+=' | '-=' | '*=' | '/=' | '%='
LogicoOR        -> LogicoAND LogicoORTail
LogicoORTail    -> '||' LogicoAND LogicoORTail | epsilon
LogicoAND       -> Igualdad LogicoANDTail
LogicoANDTail   -> '&&' Igualdad LogicoANDTail | epsilon
Igualdad        -> Comparacion IgualdadTail
IgualdadTail    -> ('==' | '!=') Comparacion IgualdadTail | epsilon
Comparacion     -> Term TermCompTail
TermCompTail    -> ('<' | '>' | '<=' | '>=') Term TermCompTail | epsilon
Term            -> Factor TermTail
TermTail        -> ('+' | '-') Factor TermTail | epsilon
Factor          -> Unario FactorTail
FactorTail      -> ('*' | '/' | '%') Unario FactorTail | epsilon
Unario          -> OperadorUnario Unario | Postfijo
OperadorUnario  -> '!' | '-' | '+'
Postfijo        -> Primario PostfijoTail
PostfijoTail    -> '.' IDENT PostfijoTail | Llamada PostfijoTail | epsilon
Llamada         -> '(' ListaArgumentosOpt ')'
ListaArgumentosOpt -> ListaArgumentos | epsilon
ListaArgumentos -> Expresion ListaArgumentosTail
ListaArgumentosTail -> ',' Expresion ListaArgumentosTail | epsilon
Primario        -> Literal
                 | IDENT
                 | '(' Expresion ')'
                 | ArregloLiteral
ArregloLiteral  -> '[' ListaExpresionesOpt ']'
ListaExpresionesOpt -> ListaExpresiones | epsilon
ListaExpresiones -> Expresion ListaExpresionesTail
ListaExpresionesTail -> ',' Expresion ListaExpresionesTail | epsilon
```

## Literales
```ebnf
Literal         -> NUMBER | STRING | CHAR | Booleano
Booleano        -> 'true' | 'false'
```

La gramatica anterior sirve como referencia unificada para la fase de analisis sintactico. Cada produccion ha sido pensada para acoplarse con los tokens ya definidos por el analizador lexico y preparar el terreno para la generacion de un parser LR.
