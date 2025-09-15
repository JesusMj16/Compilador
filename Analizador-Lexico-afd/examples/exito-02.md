###  📌Estructuras de control if-else

**Descripción:**  Se verifica el reconocimiento de las palabras
reservadas `if` y `else` para el control de flujo. También se prueba
el manejo de bloques de código delimitados por llaves `{}` y el 
operador lógico de negación `!`.

``` c++
fn verificar_edad(edad: i32) {
    if edad >= 18 {
        let mensaje = "Es mayor de edad";
    } else {
        let mensaje_alternativo = "Es menor de edad";
    }
    
    if !true {
        // Esto no se ejecuta
    }
}
``` 