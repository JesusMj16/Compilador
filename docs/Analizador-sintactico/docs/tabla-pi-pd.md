# Tabla de PI y PD

Esta tabla resume los conjuntos de Primera Izquierda (PI) y Primera Derecha (PD) asociados a cada no terminal de la gramatica descrita en `gramatica.md`. Los conjuntos PI son equivalentes a FIRST y los conjuntos PD coinciden con LAST. El simbolo `epsilon` indica que la produccion puede derivar la cadena vacia.

## No terminales estructurales
| No terminal           | PI                                                   | PD                                               |
|-----------------------|-------------------------------------------------------|--------------------------------------------------|
| Programa              | {'fn','let','if','while','for','loop','match','{','return','break','continue','!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'EOF'} |
| ListaItems            | {'fn','let','if','while','for','loop','match','{','return','break','continue','!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','[','epsilon'} | {';','}'} |
| Item                  | {'fn','let','if','while','for','loop','match','{','return','break','continue','!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {';','}'} |
| Funcion               | {'fn'}                                               | {'}'} |
| ListaParametrosOpt    | {'IDENT','epsilon'}                                  | {'i32','f64','bool','char','IDENT','epsilon'} |
| ListaParametros       | {'IDENT'}                                            | {'i32','f64','bool','char','IDENT'} |
| ListaParametrosTail   | {',','epsilon'}                                      | {'i32','f64','bool','char','IDENT','epsilon'} |
| Parametro             | {'IDENT'}                                            | {'i32','f64','bool','char','IDENT'} |
| Tipo                  | {'i32','f64','bool','char','IDENT'}                  | {'i32','f64','bool','char','IDENT'} |
| Bloque                | {'{'}                                                | {'}'} |
| ListaSentencias       | {'let','if','while','for','loop','match','{','return','break','continue','!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','[','epsilon'} | {';','}'} |
| Sentencia             | {'let','if','while','for','loop','match','{','return','break','continue','!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {';','}'} |
| LetSentencia          | {'let'}                                              | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','i32','f64','bool','char'} |
| IfSentencia           | {'if'}                                               | {'}'} |
| ElseOpt               | {'else','epsilon'}                                   | {'}','epsilon'} |
| LoopSentencia         | {'while','for','loop'}                               | {'}'} |
| WhileSentencia        | {'while'}                                            | {'}'} |
| ForSentencia          | {'for'}                                              | {'}'} |
| LoopForever           | {'loop'}                                             | {'}'} |
| ReturnSentencia       | {'return'}                                           | {'return','NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| BreakSentencia        | {'break'}                                            | {'break'} |
| ContinueSentencia     | {'continue'}                                         | {'continue'} |
| ExprSentencia         | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| ExpresionOpt          | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','[','epsilon'} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| MutOpt                | {'mut','epsilon'}                                    | {'mut','epsilon'} |
| AnotacionTipoOpt      | {':','epsilon'}                                      | {'i32','f64','bool','char','IDENT','epsilon'} |
| InicializacionOpt     | {'=','epsilon'}                                      | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| MatchSentencia        | {'match'}                                            | {'}'} |
| ListaMatchBrazos      | {'NUMBER','STRING','CHAR','true','false','IDENT'}    | {';'} |
| ListaMatchBrazosTail  | {'NUMBER','STRING','CHAR','true','false','IDENT','epsilon'} | {';','epsilon'} |
| MatchBrazo            | {'NUMBER','STRING','CHAR','true','false','IDENT'}    | {';'} |
| MatchPatron           | {'NUMBER','STRING','CHAR','true','false','IDENT'}    | {'NUMBER','STRING','CHAR','true','false','IDENT'} |
| MatchResultado        | {'{','!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'}','NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |

## No terminales de expresiones
| No terminal         | PI                                                   | PD                                               |
|---------------------|-------------------------------------------------------|--------------------------------------------------|
| Expresion           | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| Asignacion          | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| AsignacionTail      | {'=','+=','-=','*=','/=','%=','epsilon'}              | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| OperadorAsignacion  | {'=','+=','-=','*=','/=','%='}                        | {'=','+=','-=','*=','/=','%='} |
| LogicoOR            | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| LogicoORTail        | {'||','epsilon'}                                      | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| LogicoAND           | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| LogicoANDTail       | {'&&','epsilon'}                                      | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Igualdad            | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| IgualdadTail        | {'==','!=','epsilon'}                                 | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Comparacion         | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| TermCompTail        | {'<','>','<=','>=','epsilon'}                         | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Term                | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| TermTail            | {'+','-','epsilon'}                                   | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Factor              | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| FactorTail          | {'*','/','%','epsilon'}                               | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Unario              | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| OperadorUnario      | {'!','-','+'}                                         | {'!','-','+'} |
| Postfijo            | {'NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| PostfijoTail        | {'.','(','epsilon'}                                   | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Llamada             | {'('}                                                | {')'} |
| ListaArgumentosOpt  | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','[','epsilon'} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| ListaArgumentos     | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| ListaArgumentosTail | {',','epsilon'}                                      | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Primario            | {'NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| ArregloLiteral      | {'['}                                                | {']'} |
| ListaExpresionesOpt | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','[','epsilon'} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| ListaExpresiones    | {'!','-','+','NUMBER','STRING','CHAR','true','false','IDENT','(','['} | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']'} |
| ListaExpresionesTail| {',','epsilon'}                                      | {'NUMBER','STRING','CHAR','true','false','IDENT',')',']','epsilon'} |
| Literal             | {'NUMBER','STRING','CHAR','true','false'}            | {'NUMBER','STRING','CHAR','true','false'} |
| Booleano            | {'true','false'}                                     | {'true','false'} |

Los conjuntos PI y PD fueron calculados de forma manual asegurando su consistencia con la gramatica propuesta. Sirven como insumo directo para construir tablas de precedencia o verificar condiciones de analizadores LR basados en dicha gramatica.
