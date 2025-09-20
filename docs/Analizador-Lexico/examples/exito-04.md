### 📌 Definición y retorno de funciones 
Se enfoca en la definición de funciones con la palabra clave fn.
Prueba el reconocimiento de parámetros con sus tipos `(a: i32)`, 
la sintaxis para el tipo de `retorno (-> i32)` y la palabra clave
`return`.
``` c++

fn sumar(a: i32, b: i32) -> i32 {
    return a + b;
}

fn principal() {
    let resultado_suma = sumar(10, 20);
}
```