# Versión para Windows de bruteforce

Esta carpeta tiene el programa `bruteforce_.c` de la Parte 1 listo para compilar y correr en Windows, con el texto del grupo cifrado con una llave privada y la medición del tiempo de búsqueda. El original sigue intacto en `Parte1/bruteforce_.c`.

Se probó el 21 de octubre de 2026 en una PC con Windows 11 Home, procesador Intel Core i5-12400 (6 núcleos, 12 hilos), MS-MPI 10.1.12498, gcc 16.1.0 de MSYS2 (UCRT64) y OpenSSL 3.6.3.

## Qué hay en la carpeta

| Archivo | Para qué sirve |
|---|---|
| `bruteforce_.c` | El programa original con solo los cambios de adaptación |
| `compat/rpc/des_crypt.h` | Reemplazo de la librería `rpc/des_crypt.h`, que Windows no trae |
| `compat/des_crypt_openssl.c` | Las funciones `ecb_crypt` y `des_setparity` hechas con OpenSSL |
| `Makefile` | Compila, corre y mide con un solo comando |
| `texto_openmpi.txt` | El texto del grupo que se cifra, el mismo que está dentro del programa |
| `medir_tiempos.py` | Corre las mediciones de tiempo y arma las tablas |
| `resultados_tiempos.csv` | Todas las corridas de la medición |
| `tiempos.md` | Las tablas de la medición |

Los archivos `compat/` y `texto_openmpi.txt` son los mismos de la carpeta `Mac`.

## Requisitos

Hay que instalar MS-MPI (el programa y el SDK) y MSYS2 con gcc, OpenSSL y el paquete de MS-MPI.

```
winget install Microsoft.msmpi
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-openssl mingw-w64-ucrt-x86_64-msmpi
```

El segundo comando se corre en una terminal de MSYS2 UCRT64.

## Cómo compilar y correr

Desde PowerShell, con estos dos directorios en el PATH.

```
$env:Path = "C:\msys64\ucrt64\bin;C:\Program Files\Microsoft MPI\Bin;" + $env:Path
cd "Parte1\Windows"

mpicc -O0 -I compat -o bruteforce.exe bruteforce_.c compat/des_crypt_openssl.c -lcrypto
mpiexec -n 4 .\bruteforce.exe
mpiexec -n 4 .\bruteforce.exe 123456     # corre con otra llave
```

Con el Makefile.

```
mingw32-make                 # compila bruteforce.exe
mingw32-make run             # corre con 4 procesos
mingw32-make run NP=8        # cambia el número de procesos
mingw32-make tiempos         # mide el tiempo de búsqueda, tarda unos minutos
```

Para llaves grandes como la del grupo (2⁵⁴) con 1 o 2 procesos, el programa tardaría años, porque el proceso 0 tendría que probar todas las llaves anteriores. Con 4 procesos o más termina en microsegundos.

## Texto cifrado y llave privada

Es el mismo texto de la versión Mac, 352 palabras y 2131 bytes, con la llave 18014398509481984 (2⁵⁴). Los primeros 48 bytes cifrados salen idénticos a los de la Mac.

```
9b831394cf5e1faa6c9ca5bed8597f26696d706c656d656e74616369c3b36e206c6962726520792064652063c3b36469
```

Como `ecb_crypt` solo cifra los primeros 16 bytes, la frase de búsqueda `"Open MPI es una"` tiene que estar dentro de esos 16 bytes. Se explica con más detalle en el README de la carpeta `Mac`.

## Cambios de adaptación en bruteforce_.c

Estos cambios son para que el programa corra en Windows. La forma de cifrar queda igual que en el original.

### 1. En Windows un long mide 32 bits

En Linux y en la Mac un `long` mide 64 bits, pero en Windows mide 32. Las llaves llegan a 2⁵⁶, entonces con `long` se desbordan y `(char *)&k` solo cubriría 4 de los 8 bytes de la llave. Todos los `long` pasaron a `long long`, `1L << 56` pasó a `1LL << 56`, `MPI_LONG` a `MPI_LONG_LONG`, `%ld` y `%li` a `%lld` y `atol` a `atoll`.

### 2. Windows no trae rpc/des_crypt.h

Igual que en la Mac, se usa el reemplazo de `compat/` hecho con OpenSSL. Con `-I compat` el código del programa no se toca.

### 3. La frase de búsqueda

Igual que en la Mac, se cambió la frase por `"Open MPI es una"` porque la original nunca coincidía y quedaba fuera de los 16 bytes que se cifran.

### 4. Hay que compilarlo sin optimización

Se compila con `-O0`, igual que en la Mac. El `0xFE << i*8` del original usa un `int` de 32 bits y se desplaza hasta 56 posiciones, y en C eso no está definido. En esta compilación el desplazamiento da la vuelta a partir de i = 4, entonces los bytes de arriba de la llave no se arman como dice el diagrama. Como `encrypt` y `decrypt` arman la llave de la misma forma, el programa encuentra la llave igual, pero con una optimización más alta el compilador podría armarlas distinto.

### 5. Agregados para la documentación y la medición

Son los mismos de la versión Mac. El programa acepta la llave al correr, el proceso 0 imprime los primeros 48 bytes cifrados, hay un `MPI_Barrier` y `MPI_Wtime` para medir solo la búsqueda, y el proceso 0 imprime `Tiempo de busqueda`.

## Medición del tiempo de búsqueda

Se hizo con `mingw32-make tiempos`, que corre `medir_tiempos.py`, con las mismas 5 llaves, los mismos 1, 2, 4 y 8 procesos y 5 corridas por combinación que en la Mac. Las 70 corridas encontraron la llave correcta.

Con 1 proceso el programa prueba unas 1,750,881 llaves por segundo (0.57 µs por llave), más rápido que en la Mac, que hacía 1,279,650.

### Tiempo promedio y desviación estándar

| Llave | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|
| Chica (1,000,000) | 0.571 ± 0.011 s | 0.603 ± 0.010 s | 0.648 ± 0.014 s | 0.760 ± 0.013 s |
| Mediana (8,000,000) | 4.761 ± 0.147 s | 4.970 ± 0.036 s | 5.348 ± 0.064 s | 6.199 ± 0.084 s |
| Del grupo (2⁵⁴) | 326 años (estimado) | 326 años (estimado) | 101 ± 31 µs | 218 ± 180 µs |
| 2⁵⁵ + 4,000,000 | 652 años (estimado) | 2.479 ± 0.057 s | 2.645 ± 0.057 s | 3.130 ± 0.092 s |
| 3·2⁵³ + 4,000,000 | 489 años (estimado) | 489 años (estimado) | 163 años (estimado) | 3.053 ± 0.088 s |

Las corridas una por una y los speedups están en `tiempos.md`.

### Lo que muestran los datos

- Con las llaves chica y mediana, que siempre tiene el proceso 0, más procesos no bajan el tiempo. Sube de 0.571 s con 1 proceso a 0.760 s con 8.
- Cuando la llave queda 4,000,000 llaves después del inicio de un rango, tarda entre 2.5 y 3.1 s con 2, 4 u 8 procesos, o sea lo mismo que le toma al proceso que la tiene probar esas 4,000,000 llaves.
- Con 8 procesos cada llave probada cuesta 0.76 µs, contra 0.57 µs con 1 proceso. No se midió la causa, pero puede ser que cada proceso llama a `MPI_Test` en cada llave y con más procesos hay más trabajo de MPI y más procesos compartiendo los 6 núcleos del procesador.
- La llave del grupo queda justo al inicio del rango del proceso 1 con 4 procesos y del proceso 2 con 8, y se encuentra en cientos de microsegundos o menos.
