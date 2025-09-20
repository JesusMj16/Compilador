### 📌 Cadenas y comentarios de bloques no cerrados 
**Descripición:** El analizador debe ser capaz de detectar que una cadena de texto iniciada con `" `no se cierra, y que un comentario de bloque iniciado con `/*` no finaliza con `*/` antes de que termine el archivo.

```c++
let texto_malo = "Esta es una cadena sin cerrar;

/* Este es un comentario
   de bloque que nunca
   se cierra.

```