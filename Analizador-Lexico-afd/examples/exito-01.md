### 📌 Declaración de variables y operaciones

**Descripción:** Este caso prueba la capacidad del analizador
para reconocer la declaración de variables (tanto mutables mut como 
inmutables let), la asignación de tipos (`i32`, `f64`), 
los literales numéricos, y los operadores aritméticos (`+, *`)
y relacionales (`>, ==, %`). Es una prueba fundamental para los
tokens más comunes.
``` c++
fn main() {
    // Variables inmutables 
    let cinco: i32 = 5; 
    let pi: f64 = 3;
    // Variable mutable
    let mut contador: i32 = 0;
    contador = contador + 1;

    let resultado = cinco * (2 + contador); // resultado = 5 * (2+1) = 15

    // Operadores lógicos y relacionales
    let es_mayor = resultado > 10; // true
    let es_igual = (resultado % 2) == 0; // false
}    
```

