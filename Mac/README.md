# Versión para Mac de bruteforce

Esta carpeta tiene el programa `bruteforce_.c` de la Parte 1 listo para compilar y correr en una Mac, con el texto del grupo cifrado con una llave privada y la medición del tiempo de búsqueda. El original sigue intacto en `Parte1/bruteforce_.c`.

Se probó el 20 de octubre de 2026 en una Mac con chip Apple M3 Max (14 núcleos, arm64), macOS 27.0.1, Open MPI 5.0.11 y OpenSSL 3.6.5.

## Qué hay en la carpeta

| Archivo | Para qué sirve |
|---|---|
| `bruteforce_.c` | El programa original con solo los cambios de adaptación |
| `compat/rpc/des_crypt.h` | Reemplazo de la librería `rpc/des_crypt.h`, que la Mac no trae |
| `compat/des_crypt_openssl.c` | Las funciones `ecb_crypt` y `des_setparity` hechas con OpenSSL |
| `Makefile` | Compila, corre y mide con un solo comando |
| `texto_openmpi.txt` | El texto del grupo que se cifra, el mismo que está dentro del programa |
| `medir_tiempos.py` | Corre las mediciones de tiempo y arma las tablas |
| `resultados_tiempos.csv` | Todas las corridas de la medición |
| `tiempos.md` | Las tablas de la medición |

## Requisitos

Hay que tener Open MPI y OpenSSL instalados con Homebrew.

```
brew install open-mpi openssl@3
```

## Cómo compilar y correr

```
make                              # compila bruteforce
make run                          # corre bruteforce con 4 procesos
make run NP=8                     # cambia el número de procesos
mpirun -np 4 ./bruteforce 123456  # corre con otra llave
make tiempos                      # mide el tiempo de búsqueda, tarda unos minutos
make clean                        # borra el ejecutable
```

Sin el Makefile, los comandos son estos.

```
export OMPI_CC=/usr/bin/clang
OPENSSL=$(brew --prefix openssl@3)

mpicc -O0 -I compat -I $OPENSSL/include -o bruteforce bruteforce_.c compat/des_crypt_openssl.c -L $OPENSSL/lib -lcrypto
mpirun -np 4 ./bruteforce
```

Al compilar salen dos avisos del linker que dicen que las librerías se hicieron para una versión más nueva de macOS. No afectan en nada.

## Texto cifrado y llave privada

El texto del grupo explica qué es Open MPI, de dónde salió y cómo se usa. Está en `texto_openmpi.txt` y también dentro del programa, en la variable `eltexto`.

| Dato | Valor |
|---|---|
| Palabras | 352, el rango pedido es de 335 a 365 |
| Tamaño | 2131 bytes |
| Primeros 16 bytes | `Open MPI es una ` (con un espacio al final) |
| Frase de búsqueda | `"Open MPI es una"` |
| Llave privada | 18014398509481984, o sea 2⁵⁴, la misma del código original |

El programa original cifra con DES en modo ECB, pero solo le pasa a `ecb_crypt` los primeros 16 bytes del texto, o sea dos bloques de 64 bits. El resto del texto queda sin cifrar. Por eso el texto empieza con `Open MPI es una` a propósito, la frase de búsqueda tiene que estar dentro de esos 16 bytes para que funcione.

Al correr el programa, el proceso 0 muestra los primeros 48 bytes del texto ya cifrado, en hexadecimal.

```
9b831394cf5e1faa6c9ca5bed8597f26696d706c656d656e74616369c3b36e206c6962726520792064652063c3b36469
```

Los primeros 16 bytes, que son los primeros 32 caracteres en hexadecimal, quedan revueltos. Desde `696d706c656d656e` ya es el texto sin cifrar, que en letras dice "implementación libre y de códi". Con 4 procesos el programa encuentra la llave y el texto descifrado sale idéntico a `texto_openmpi.txt`.

## Cambios de adaptación en bruteforce_.c

Estos cambios son para que el programa corra en la Mac. La forma de cifrar queda igual que en el original, entonces lo que se cifre aquí se puede descifrar con el código original en Linux y al revés.

### 1. El compilador de Homebrew no encuentra el SDK

El `mpicc` de Homebrew usa un clang que busca un SDK de macOS que no existe en la máquina, y falla desde la primera línea.

```
clang: warning: no such sysroot directory: '/Library/Developer/CommandLineTools/SDKs/MacOSX27.sdk'
bruteforce_.c:1:10: fatal error: 'string.h' file not found
```

Se arregla diciéndole a `mpicc` que use el clang de Apple con `OMPI_CC=/usr/bin/clang`. El Makefile ya lo hace.

### 2. La Mac no trae rpc/des_crypt.h

El programa usa `ecb_crypt` y `des_setparity` de la librería `rpc/des_crypt.h`, que solo existe en Linux.

```
bruteforce_.c:6:10: fatal error: 'rpc/des_crypt.h' file not found
```

En `compat/` hay un reemplazo con las mismas funciones y las mismas constantes (`DES_ENCRYPT`, `DES_DECRYPT`), hechas con la librería DES de OpenSSL. El código del programa no se toca, solo se le dice al compilador que busque ese archivo en `compat/`. Se comprobó con un caso de prueba conocido de DES (llave `133457799BBCDFF1`, texto `0123456789ABCDEF`) y da el resultado esperado `85E813540F0AB405`, o sea el mismo DES estándar que usa Linux.

### 3. La función encrypt choca con la de macOS

En macOS el archivo `<unistd.h>` ya declara una función `encrypt(char *, int)`, y la del programa se llama igual pero recibe otras cosas.

```
bruteforce_.c:24:6: error: conflicting types for 'encrypt'
```

El programa no usa nada de `<unistd.h>`, entonces se quitó esa línea.

### 4. La frase de búsqueda nunca coincide

Este problema no es de la Mac, también pasa en Linux. Sin arreglarlo el programa nunca encuentra la llave.

- El programa busca `" los mismos del proyecto 1 "`, con un espacio antes y uno después. En el texto la frase aparece como `"(los mismos del proyecto 1),"`, entonces nunca coincide. Al correrlo con 4 procesos, el proceso 1 empieza justo en la llave correcta (2⁵⁴), la prueba y no la reconoce. El programa se queda buscando y el proceso 0 espera para siempre.
- Aunque se corrija la frase, queda otro problema. `ecb_crypt` solo cifra los primeros 16 bytes del texto y esa frase está después del byte 30, en la parte que nunca se cifró. Con eso cualquier llave parece correcta, y en la prueba el proceso 3 "encontró" una llave equivocada en su primer intento.

Se cambió la frase por `"Open MPI es una"`, que está dentro de los primeros 16 bytes del texto del grupo. Esto no cambia cómo se cifra, solo lo que se busca después de descifrar. Para descifrar el texto de otra persona hay que buscar una frase que esté dentro de sus primeros 16 bytes.

### 5. Hay que compilarlo sin optimización

Para armar la llave, el original hace `0xFE << i*8`. El `0xFE` es un `int` de 32 bits y se desplaza hasta 56 posiciones. En C eso es comportamiento indefinido, o sea que el resultado depende del compilador.

| Optimización | Resultado con 4 procesos |
|---|---|
| `-O0` | Encuentra la llave 18014398509481984 |
| `-O1` | Encuentra la llave 18014398509481984 |
| `-O2` | No la encuentra en 20 segundos |

Con `-O2` el compilador arma la llave de cifrar de una forma y la de descifrar de otra, entonces nunca coinciden. Por eso el Makefile compila con `-O0`, que es lo mismo que pasa si se compila con un `mpicc` simple sin opciones. Esto también quiere decir que el mismo código podría comportarse distinto en otra computadora, y no se ha comparado todavía contra una compu Linux con Intel.

### 6. Agregados para la documentación y la medición

Estos agregados no cambian cómo se cifra ni cómo se busca la llave.

| Agregado | Para qué sirve |
|---|---|
| El texto del grupo en `eltexto` | Cifrar el texto de 352 palabras en lugar del texto de ejemplo |
| El proceso 0 imprime los primeros 48 bytes cifrados en hexadecimal | Mostrar en la captura que el texto sí se cifró |
| La llave se puede pasar al correr, `mpirun -np 4 ./bruteforce 123456` | Probar varias llaves sin recompilar. Si no se pasa nada se usa 2⁵⁴ |
| `MPI_Barrier` y `MPI_Wtime` antes de empezar a buscar | Que todos los procesos empiecen juntos y medir solo la búsqueda, sin lo que tarda MPI en arrancar |
| El proceso 0 imprime `Tiempo de busqueda` | El tiempo desde que empieza la búsqueda hasta que le llega la llave |

## Medición del tiempo de búsqueda

Las mediciones se hicieron con `make tiempos`, que corre `medir_tiempos.py`. Para cada llave y cada número de procesos, el programa se corrió 5 veces y se sacó el promedio y la desviación estándar del tiempo que imprime el proceso 0.

Con 1 proceso el programa prueba unas 1,279,650 llaves por segundo. Recorrer las 2⁵⁶ llaves completas tomaría unos 1,784 años, entonces se eligieron llaves que caen en distintas partes de los rangos de cada proceso.

| Llave | Valor | Por qué se eligió |
|---|---|---|
| Chica | 1,000,000 | Está al inicio del rango del proceso 0 con cualquier número de procesos |
| Mediana | 8,000,000 | Igual que la chica, pero el proceso 0 tarda 8 veces más en llegar |
| Del grupo (2⁵⁴) | 18,014,398,509,481,984 | La llave del código original y del texto del grupo |
| 2⁵⁵ + 4,000,000 | 36,028,797,022,963,968 | Queda cerca del inicio de un rango con 2, 4 y 8 procesos |
| 3·2⁵³ + 4,000,000 | 27,021,597,768,222,976 | Queda cerca del inicio de un rango solo con 8 procesos |

Cuando una combinación tardaría años, no se corrió. Su tiempo se estimó dividiendo las llaves que tendría que probar el proceso entre la velocidad medida con 1 proceso, y en la tabla dice "estimado".

### Tiempo adecuado de cada llave según el número de procesos

Cada valor es el tiempo adecuado de esa llave, o sea el promedio de sus 5 corridas, y después del ± va la desviación estándar. Los valores que dicen "estimado" no se corrieron porque tardarían años.

| Llave | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|
| Chica (1,000,000) | 0.781 ± 0.016 s | 0.794 ± 0.004 s | 0.845 ± 0.003 s | 0.862 ± 0.001 s |
| Mediana (8,000,000) | 6.178 ± 0.008 s | 6.568 ± 0.077 s | 6.814 ± 0.010 s | 6.894 ± 0.011 s |
| Del grupo (2⁵⁴) | 446 años (estimado) | 446 años (estimado) | 16 ± 8 µs | 13 ± 1 µs |
| 2⁵⁵ + 4,000,000 | 892 años (estimado) | 3.357 ± 0.012 s | 3.423 ± 0.019 s | 3.456 ± 0.013 s |
| 3·2⁵³ + 4,000,000 | 669 años (estimado) | 669 años (estimado) | 223 años (estimado) | 3.482 ± 0.053 s |

En todas las combinaciones que se midieron, cada llave probada costó entre 0.77 y 0.87 microsegundos. Con ese valor se calcularon los tiempos estimados. Las 5 corridas de cada combinación, una por una, están en `tiempos.md`.

### Proceso que tiene la llave y llaves que prueba antes de llegar a ella

| Llave | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|
| Chica | P0, 1,000,000 | P0, 1,000,000 | P0, 1,000,000 | P0, 1,000,000 |
| Mediana | P0, 8,000,000 | P0, 8,000,000 | P0, 8,000,000 | P0, 8,000,000 |
| Del grupo (2⁵⁴) | P0, 18,014,398,509,481,984 | P0, 18,014,398,509,481,984 | P1, 0 | P2, 0 |
| 2⁵⁵ + 4,000,000 | P0, 36,028,797,022,963,968 | P1, 4,000,000 | P2, 4,000,000 | P4, 4,000,000 |
| 3·2⁵³ + 4,000,000 | P0, 27,021,597,768,222,976 | P0, 27,021,597,768,222,976 | P1, 9,007,199,258,740,992 | P3, 4,000,000 |

### Speedup con respecto a 1 proceso

Es el tiempo con 1 proceso dividido entre el tiempo con n procesos. Cuando uno de los dos tiempos es estimado, el speedup también lo es.

| Llave | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|
| Chica | 1.00 | 0.98 | 0.92 | 0.91 |
| Mediana | 1.00 | 0.94 | 0.91 | 0.90 |
| Del grupo (2⁵⁴) | 1.00 | 1.00 (estimado) | 8.6 × 10¹⁴ (estimado) | 1.1 × 10¹⁵ (estimado) |
| 2⁵⁵ + 4,000,000 | 1.00 | 8.4 × 10⁹ (estimado) | 8.2 × 10⁹ (estimado) | 8.1 × 10⁹ (estimado) |
| 3·2⁵³ + 4,000,000 | 1.00 | 1.00 (estimado) | 3.00 (estimado) | 6.1 × 10⁹ (estimado) |

### Lo que muestran los datos

- Las 5 corridas de cada combinación dieron casi el mismo tiempo. En las combinaciones que tardan más de medio segundo, la desviación estándar no pasa del 2.1% del promedio.
- Con las llaves chica y mediana, que siempre tiene el proceso 0, más procesos no bajan el tiempo. Sube un poco, de 0.78 s con 1 proceso a 0.86 s con 8.
- Cuando la llave queda cerca del inicio del rango de un proceso, el tiempo depende de cuántas llaves prueba ese proceso antes de llegar a ella. Con 4,000,000 llaves antes, tarda unos 3.4 s con 2, 4 u 8 procesos.
- La llave del grupo queda justo al inicio del rango del proceso 1 con 4 procesos y del proceso 2 con 8, y se encuentra en unos 15 microsegundos.

## Pruebas que se hicieron

| Prueba | Resultado |
|---|---|
| `make run` con la llave 2⁵⁴ | Encuentra la llave y el texto descifrado sale idéntico a `texto_openmpi.txt` |
| `make tiempos` | 70 corridas, todas encontraron la llave correcta |
| Compilado con `-O0`, `-O1` y `-O2` | Con `-O0` y `-O1` funciona, con `-O2` no encuentra la llave |
