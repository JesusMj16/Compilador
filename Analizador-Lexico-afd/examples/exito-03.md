### 📌 Ciclos while y for
**Descripción:** Este caso prueba las diferentes estructuras de 
ciclo del lenguaje: while, for y loop. Se asegura que el analizador
reconozca estas palabras clave, así como las instrucciones de 
control de ciclo break y continue. 
```c++
fn ciclos_ejemplo() {
    let mut i: i32 = 0;
    while i < 5 {
        i = i + 1;
        if i == 3 {
            continue; // Salta a la siguiente iteración
        }
    }

    for j in 0..10 {
        if j == 8 {
            break; // Termina el ciclo for
        }
    }

    loop {
        // Ciclo infinito que necesita un break
        break;
    }
}

```