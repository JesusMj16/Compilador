### 📌 Identificadores Invalidos 
**Descripción:** Este caso prueba las reglas de formación de identificadores. El analizador debe rechazar `1numero` porque un identificador no puede comenzar con un dígito, y `mi__variable` porque la regla prohíbe dos guiones bajos consecutivos.

``` c++

let 1numero = 1; // Error: No puede empezar con un dígito.
let mi__variable = 2; // Error: No se permiten dos guiones bajos consecutivos.

```