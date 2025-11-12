# Tabla de Partes Izquierdas (PI) y Partes Derechas (PD)

Este documento calcula PI y PD siguiendo las láminas adjuntas y usando la gramática reducida definida en `gramatica-bnf.md`.

Criterio empleado:
- Para `A → a b c`, PI(A) = { a } y PD(A) = { c }.
- Si `A` tiene varias producciones, se toma la unión de las partes extremas de cada producción.
- Si `A → B x C` (con no terminales), se arrastran posibles extremos: PI(A) agrega PI(B) y PD(A) agrega PD(C).
- Si hay producción `ε`, se agrega `ε` al conjunto correspondiente.

A continuación, se listan las producciones relevantes y después la tabla 

## Producciones relevantes (resumen)
- Programa → ListaItems EOF
- ListaItems → Item ListaItems | ε
- Item → Funcion | Sentencia
- Funcion → 'fn' IDENT '(' ListaParametrosOpt ')' Bloque
- ListaParametrosOpt → ListaParametros | ε
- ListaParametros → Parametro ListaParametrosTail
- ListaParametrosTail → ',' Parametro ListaParametrosTail | ε
- Parametro → IDENT ':' Tipo
- Tipo → 'i32' | 'f64' | 'bool' | IDENT
- Bloque → '{' ListaSentencias '}'
- ListaSentencias → Sentencia ListaSentencias | ε
- Sentencia → LetSentencia ';' | ExprSentencia ';' | Bloque | ReturnSentencia ';'
- LetSentencia → 'let' MutOpt IDENT AnotacionTipoOpt InicializacionOpt
- MutOpt → 'mut' | ε
- AnotacionTipoOpt → ':' Tipo | ε
- InicializacionOpt → '=' Expresion | ε
- ExprSentencia → Expresion
- ReturnSentencia → 'return' ExpresionOpt
- ExpresionOpt → Expresion | ε
- Expresion → Asignacion
- Asignacion → LogicoOR AsignacionTail
- AsignacionTail → '=' Asignacion | ε
- LogicoOR → LogicoAND LogicoORTail
- LogicoORTail → '||' LogicoAND LogicoORTail | ε
- LogicoAND → Igualdad LogicoANDTail
- LogicoANDTail → '&&' Igualdad LogicoANDTail | ε
- Igualdad → Comparacion IgualdadTail
- IgualdadTail → ('==' | '!=') Comparacion IgualdadTail | ε
- Comparacion → Aditivo ComparacionTail
- ComparacionTail → ('<' | '>' | '<=' | '>=') Aditivo ComparacionTail | ε
- Aditivo → Multiplicativo AditivoTail
- AditivoTail → ('+' | '-') Multiplicativo AditivoTail | ε
- Multiplicativo → Unario MultiplicativoTail
- MultiplicativoTail → ('*' | '/' | '%') Unario MultiplicativoTail | ε
- Unario → OperadorUnario Unario | Postfijo
- OperadorUnario → '!' | '-' | '+'
- Postfijo → Primario PostfijoTail
- PostfijoTail → Llamada PostfijoTail | ε
- Llamada → '(' ListaArgumentosOpt ')'
- ListaArgumentosOpt → ListaArgumentos | ε
- ListaArgumentos → Expresion ListaArgumentosTail
- ListaArgumentosTail → ',' Expresion ListaArgumentosTail | ε
- Primario → Literal | IDENT | '(' Expresion ')'
- Literal → NUMBER | Booleano
- Booleano → 'true' | 'false'

---

## Tabla PI/PD (formato del ejemplo)

| Símbolo | PI | PD |
|---|---|---|
| Programa | ListaItems, ε, 'EOF' | 'EOF' |
| ListaItems | Item, ε | ListaItems, ε |
| Item | Funcion, Sentencia | ';', '}' |
| Funcion | 'fn' | '}' |
| ListaParametrosOpt | ListaParametros, ε | ListaParametrosTail, ε |
| ListaParametros | Parametro | ListaParametrosTail |
| ListaParametrosTail | ',', ε | ListaParametrosTail, ε |
| Parametro | IDENT | Tipo |
| Tipo | 'i32', 'f64', 'bool', IDENT | 'i32', 'f64', 'bool', IDENT |
| Bloque | '{' | '}' |
| ListaSentencias | Sentencia, ε | ListaSentencias, ε |
| Sentencia | 'let', 'return', '{', '(', IDENT, NUMBER, 'true', 'false', '!', '-', '+' | ';', '}' |
| LetSentencia | 'let' | IDENT, 'i32', 'f64', 'bool', IDENT, NUMBER, 'true', 'false', ')', ε |
| MutOpt | 'mut', ε | 'mut', ε |
| AnotacionTipoOpt | ':', ε | 'i32', 'f64', 'bool', IDENT, ε |
| InicializacionOpt | '=', ε | IDENT, NUMBER, 'true', 'false', ')', ε |
| ExprSentencia | '(', IDENT, NUMBER, 'true', 'false', '!', '-', '+' | IDENT, NUMBER, 'true', 'false', ')' |
| ReturnSentencia | 'return' | IDENT, NUMBER, 'true', 'false', ')' , ε |
| ExpresionOpt | Expresion, ε | ')', IDENT, NUMBER, 'true', 'false', ε |
| Expresion | Asignacion | Asignacion |
| Asignacion | LogicoOR | Asignacion, LogicoOR |
| AsignacionTail | '=', ε | Asignacion, ε |
| LogicoOR | LogicoAND | LogicoAND |
| LogicoORTail | '||', ε | LogicoAND, ε |
| LogicoAND | Igualdad | Igualdad |
| LogicoANDTail | '&&', ε | Igualdad, ε |
| Igualdad | Comparacion | Comparacion |
| IgualdadTail | '==', '!=', ε | Comparacion, ε |
| Comparacion | Aditivo | Aditivo |
| ComparacionTail | '<', '>', '<=', '>=', ε | Aditivo, ε |
| Aditivo | Multiplicativo | Multiplicativo |
| AditivoTail | '+', '-', ε | Multiplicativo, ε |
| Multiplicativo | Unario | Unario |
| MultiplicativoTail | '*', '/', '%', ε | Unario, ε |
| Unario | '!', '-', '+', Postfijo | Postfijo |
| OperadorUnario | '!', '-', '+' | '!', '-', '+' |
| Postfijo | Primario | PostfijoTail, ')', IDENT, NUMBER, 'true', 'false' |
| PostfijoTail | Llamada, ε | PostfijoTail, ')', ε |
| Llamada | '(' | ')' |
| ListaArgumentosOpt | ListaArgumentos, ε | ListaArgumentosTail, ε |
| ListaArgumentos | Expresion | ListaArgumentosTail |
| ListaArgumentosTail | ',', ε | ListaArgumentosTail, ε |
| Primario | Literal, IDENT, '(' | Literal, IDENT, ')' |
| Literal | NUMBER, Booleano | NUMBER, Booleano |
| Booleano | 'true', 'false' | 'true', 'false' |


