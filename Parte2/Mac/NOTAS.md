# Notas de bruteforce_fix

`bruteforce_fix.c` es la versión de `bruteforce_.c` con los errores de lógica corregidos. En la Parte 2 es el programa que usa el equipo, porque cifra y descifra todo el texto y corrige el armado de la llave. Aquí lleva además los agregados del cluster (nombre del nodo, lectura de base64 y la opción `--keyorig`).

## Arreglos de lógica

Cada arreglo está marcado en el código con un comentario.

| Problema en el original | Qué pasaba | Qué hace el fix |
|---|---|---|
| La llave se arma con `0xFE << i*8` | Comportamiento indefinido, los 56 bits de la llave quedan en posiciones distintas a las que se buscaba y con `-O2` deja de funcionar | `make_key_fix` usa `unsigned long` y `0xFEUL`, cada byte lleva 7 bits del número y el bit de paridad |
| `ecb_crypt` con 16 fijo | Solo se cifraban los primeros 16 bytes, el resto del texto quedaba legible | Se cifra todo el texto, hasta el último bloque completo de 8 bytes |
| `decrypt` ignora `len` | Siempre descifraba 16 bytes | `encrypt` y `decrypt` usan `len` |
| Frase con espacios | Nunca coincidía con el texto | Como se cifra todo, la frase puede estar en cualquier parte del texto |
| `myupper = rango * (id+1) - 1` con `i < myupper` | Cada proceso se saltaba la última llave de su rango | `myupper` es el primer valor que ya no le toca al proceso |
| `found` servía para dos cosas | La llave propia y la que llega de otro proceso se guardaban en la misma variable | `found` es la propia y `received` la que llega |
| Si nadie encuentra la llave | El proceso 0 se quedaba esperando para siempre en `MPI_Wait` | Al final todos usan `MPI_Allreduce` para juntar lo que encontraron, y si nadie la encontró se cancela la recepción pendiente y se avisa |

## Armado de la llave en la Parte 2

Por defecto `bruteforce_fix.c` usa el armado corregido. Se comprobó que es el que usan los equipos, porque con él se descifró el texto del Grupo 9 (llave 3141592). Para un equipo que hubiera dejado el armado original se agrega `--keyorig`, que reproduce el armado del código base.
