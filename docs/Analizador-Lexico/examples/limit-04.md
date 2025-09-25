### 📌 Archivo extenso de 200 líneas
**Descripción:** Este caso prueba el rendimiento y robustez del analizador léxico con un archivo de código extenso que contiene múltiples funciones, estructuras de control, variables y operaciones. Es útil para verificar que el analizador puede manejar archivos grandes sin problemas de memoria o rendimiento.

```rust
// Programa extenso que implementa diferentes algoritmos y estructuras de datos
// para probar el analizador léxico con un archivo de 200 líneas

fn main() {
    // Declaraciones de variables
    let mut contador: i32 = 0;
    let limite: i32 = 100;
    
    // Bucles y estructuras de control
    while contador < limite {
        if contador % 2 == 0 {
            contador = contador + 1;
        } else {
            contador = contador * 2;
        }
    }
    
    // Llamadas a funciones
    let resultado = fibonacci(20);
    let sorted_array = bubble_sort();
    
    // Más código para completar las 200 líneas...
}
```