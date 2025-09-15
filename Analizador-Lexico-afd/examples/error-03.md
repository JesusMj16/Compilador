### 📌 Numeros Invalidos
**Descripición:** Se verifica que el analizador pueda detectar literales numéricos mal formados. `1.2.3` es inválido por tener múltiples puntos decimales, `0xFG` contiene un dígito (`G`) no válido en hexadecimal, y `0b102` contiene un dígito (`2`) no válido en binario.

``` c++
let numero_invalido = 1.2.3; // Error: Múltiples puntos decimales.
let hexadecimal_malo = 0xFG; // Error: 'G' no es un dígito hexadecimal válido.
let binario_malo = 0b102;   // Error: '2' no es un dígito binario válido.

```

