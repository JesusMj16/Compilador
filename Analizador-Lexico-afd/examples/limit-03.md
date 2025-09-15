### 📌 Codigo valios pero sin espacios entre tokens.
**Descripción:** Una prueba de estrés importante. El analizador debe ser capaz de separar correctamente los tokens aunque no haya espacios entre ellos. Por ejemplo, `a=10` debe ser reconocido como tres tokens (`a, =, 10`), no como uno solo. Esto valida la lógica de transición de estados del autómata.

``` c++
let a=10;let b=20;if a>b{return a;}else{return b;}
```