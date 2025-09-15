### 📌 Simbolos Invalidos 
**Descripción:** El analizador debe identificar que los caracteres `$` y `¿`no son parte del alfabeto del lenguaje (no son operadores, delimitadores ni caracteres válidos para identificadores o literales). Debe reportarlos como errores léxicos.

``` c++
let mi_variable$ = 10; // Error: '$' no es un operador ni parte de un identificador.
let otra_variable = ¿5;  // Error: '¿' es inválido.
```