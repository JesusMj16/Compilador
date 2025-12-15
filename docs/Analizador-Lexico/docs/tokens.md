# Tokens del Lenguaje 
Lenguaje: Utilizando funciones, declaraciones, asignaciones, expresiones lógicas y aritméticas, e instrucciones condicionales `if` y `else`.

## Palabras reservadas
| Token | Definición |
|----------------------|----------------|
| `fn` | definición de función | 
| `let` | declaración de variable |
| `mut` | modificador de variable (variables mutables) |
| `if` | condición con rama verdadera |
| `else` | rama alternativa del `if` |
| `return` | retorno de función |
| `true` | valor lógico verdadero |
| `false` | valor lógico falso |

---

## Tipos de datos
| Token | Descripción |
|---------------------|----|
| `i32` | entero de 32 bits |
| `f64` | flotante de 64 bits |
| `bool` | booleano |

---

## Operadores aritméticos
| Token | Descripción |
|-----------------------------------|----|
| `+` | suma |
| `-` | resta o signo |
| `*` | multiplicación |
| `/` | división |
| `%` | módulo |

---

## Operadores de comparación
| Token | Descripción |
|-----------------------------------------|----|
| `==` | igual |
| `!=` | diferente |
| `<` | menor que |
| `>` | mayor que |
| `<=` | menor o igual que |
| `>=` | mayor o igual que |

---

## Operadores lógicos
| Token | Descripción |
|--------------------|----|
| `&&` | operador AND |
| `\|\|` | operador OR |
| `!` | operador NOT |

---

## Asignación
| Token | Descripción |
|--------------------|----|
| `=` | asignación simple |

---

## Delimitadores y separadores
| Token | Descripción |
|-----------------|----|
| `;` | fin de instrucción |
| `,` | separador |
| `(` `)` | paréntesis |
| `{` `}` | bloques |
| `:` | anotación de tipo |

---

## Identificadores
- Deben iniciar con una letra o `_`.
- Pueden contener letras, dígitos o `_`.
- No se permiten `__` consecutivos.
- Ejemplos válidos: `x`, `_contador`, `suma_total`
- Ejemplos inválidos: `12abc`, `a__b`

---

## Números
- Enteros decimales: `0`, `123`.
- Reales decimales (con exponente opcional): `3.14`, `0.5`, `2.5e10`.
- El signo `+`/`-` se tokeniza como operador y la negación/unario se resuelve en la fase sintáctica (por ejemplo: `-45` se tokeniza como `-` y `45`).
- Ejemplos inválidos: `1.2.3`, `12e`, `12e3` (el exponente solo se permite en reales con punto).

---

## Comentarios
- Una línea: `// comentario`
- Varias líneas: `/* comentario */`
