# 📌 Tokens del Lenguaje

## 🔑 Palabras reservadas
| Token | Definición |
|----------------------|----------------|
| `fn` | definición de función | 
| `let` | declaración de variable |
| `mut` | modificador de variable | 
| `if` | condicional |
| `else` | alternativa condicional |
| `match` | selección múltiple |
| `while` | ciclo condicional |
| `loop` | ciclo infinito |
| `for` | ciclo sobre rango o colección |
| `in` | usado para contador en `for` |
| `break` | instrucción para salir de un ciclo |
| `continue` | continuar al siguiente ciclo |
| `return` | retorno de función |
| `true` | valor lógico verdadero |
| `false` | valor lógico falso |

---

## 📦 Tipos de datos
| Token | Descripción |
|---------------------|----|
| `i32` | entero de 32 bits |
| `f64` | flotante de 64 bits |
| `bool` | booleano |
| `char` | carácter Unicode |

---

## ➕ Operadores aritméticos
| Token | Descripción |
|-----------------------------------|----|
| `+` | suma |
| `-` | resta o signo |
| `*` | multiplicación |
| `/` | división |
| `%` | módulo |

---

## 🔍 Operadores de comparación
| Token | Descripción |
|-----------------------------------------|----|
| `==` | igual |
| `!=` | diferente |
| `<` | menor que |
| `>` | mayor que |
| `<=` | menor o igual que |
| `>=` | mayor o igual que |

---

## ⚡ Operadores lógicos
| Token | Descripción |
|--------------------|----|
| `&&` | operador AND |
| `\|\|` | operador OR |
| `!` | operador NOT |

---

## 📌 Delimitadores y separadores
| Token | Descripción |
|-----------------|----|
| `;` | fin de instrucción |
| `,` | separador |
| `(` `)` | paréntesis |
| `{` `}` | bloques |
| `[` `]` | arreglos / slices |

---

## 🆔 Identificadores
- Deben iniciar con una letra o `_`.  
- Pueden contener letras, dígitos o `_`.  
- ❌ No se permiten `__` consecutivos.  
- Ejemplos válidos: `x`, `_contador`, `suma_total`  
- Ejemplos inválidos: `12abc`, `a__b`  

---

## 🔢 Números
- **Enteros**: `123`, `-45`, `0xff` (hexadecimal), `0b1010` (binario).  
- **Reales**: `3.14`, `-0.5`, `2.5e10`.  
- Ejemplos inválidos: `1.2.3`, `12e`.  

---

## 📝 Cadenas y caracteres
- **Cadenas**: `"Hola mundo"`, `"Linea\nNueva"`.  
- **Caracteres**: `'a'`, `'\n'`.  
- Ejemplos inválidos: `"Texto sin cierre`.  

---

## 💬 Comentarios
- Una línea: `// comentario`  
- Varias líneas: `/* comentario */`  
