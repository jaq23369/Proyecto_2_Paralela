Medido el 20/10/2026 en una Mac arm64 (macOS 27.0.1), con 5 corridas por combinación. El tiempo es el que imprime el proceso 0 con MPI_Wtime, desde que todos los procesos empiezan a buscar hasta que le llega la llave.

Velocidad con 1 proceso, 1,279,650 llaves por segundo.

#### Tiempo promedio y desviación estándar

| Llave | Valor | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|---|
| Chica | 1,000,000 | 0.781 ± 0.016 s | 0.794 ± 0.004 s | 0.845 ± 0.003 s | 0.862 ± 0.001 s |
| Mediana | 8,000,000 | 6.178 ± 0.008 s | 6.568 ± 0.077 s | 6.814 ± 0.010 s | 6.894 ± 0.011 s |
| Del grupo (2^54) | 18,014,398,509,481,984 | 446 años (estimado) | 446 años (estimado) | 16 ± 8 µs | 13 ± 1 µs |
| 2^55 + 4,000,000 | 36,028,797,022,963,968 | 892 años (estimado) | 3.357 ± 0.012 s | 3.423 ± 0.019 s | 3.456 ± 0.013 s |
| 3*2^53 + 4,000,000 | 27,021,597,768,222,976 | 669 años (estimado) | 669 años (estimado) | 223 años (estimado) | 3.482 ± 0.053 s |

#### Tiempo adecuado de cada llave con 1 proceso

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.772 s | 0.768 s | 0.808 s | 0.786 s | 0.774 s | 0.781 s | 0.016 s | 0.781 µs |
| Mediana | 6.179 s | 6.167 s | 6.188 s | 6.171 s | 6.183 s | 6.178 s | 0.008 s | 0.772 µs |
| Del grupo (2^54) | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 446 años (estimado) | - | - |
| 2^55 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 892 años (estimado) | - | - |
| 3*2^53 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 669 años (estimado) | - | - |
| Promedio con 1 proceso |  |  |  |  |  | 3.479 s | - | 0.777 µs |

#### Tiempo adecuado de cada llave con 2 procesos

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.796 s | 0.797 s | 0.793 s | 0.797 s | 0.789 s | 0.794 s | 0.004 s | 0.794 µs |
| Mediana | 6.461 s | 6.536 s | 6.560 s | 6.628 s | 6.654 s | 6.568 s | 0.077 s | 0.821 µs |
| Del grupo (2^54) | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 446 años (estimado) | - | - |
| 2^55 + 4,000,000 | 3.356 s | 3.353 s | 3.347 s | 3.352 s | 3.379 s | 3.357 s | 0.012 s | 0.839 µs |
| 3*2^53 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 669 años (estimado) | - | - |
| Promedio con 2 procesos |  |  |  |  |  | 3.573 s | - | 0.818 µs |

#### Tiempo adecuado de cada llave con 4 procesos

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.847 s | 0.850 s | 0.845 s | 0.841 s | 0.845 s | 0.845 s | 0.003 s | 0.845 µs |
| Mediana | 6.800 s | 6.822 s | 6.806 s | 6.823 s | 6.818 s | 6.814 s | 0.010 s | 0.852 µs |
| Del grupo (2^54) | 13 µs | 31 µs | 13 µs | 12 µs | 13 µs | 16 µs | 8 µs | - |
| 2^55 + 4,000,000 | 3.417 s | 3.412 s | 3.419 s | 3.409 s | 3.455 s | 3.423 s | 0.019 s | 0.856 µs |
| 3*2^53 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 223 años (estimado) | - | - |
| Promedio con 4 procesos |  |  |  |  |  | 2.770 s | - | 0.851 µs |

#### Tiempo adecuado de cada llave con 8 procesos

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.862 s | 0.863 s | 0.863 s | 0.861 s | 0.862 s | 0.862 s | 0.001 s | 0.862 µs |
| Mediana | 6.889 s | 6.888 s | 6.893 s | 6.887 s | 6.914 s | 6.894 s | 0.011 s | 0.862 µs |
| Del grupo (2^54) | 12 µs | 12 µs | 14 µs | 14 µs | 12 µs | 13 µs | 1 µs | - |
| 2^55 + 4,000,000 | 3.448 s | 3.444 s | 3.457 s | 3.477 s | 3.455 s | 3.456 s | 0.013 s | 0.864 µs |
| 3*2^53 + 4,000,000 | 3.577 s | 3.460 s | 3.458 s | 3.458 s | 3.457 s | 3.482 s | 0.053 s | 0.871 µs |
| Promedio con 8 procesos |  |  |  |  |  | 2.939 s | - | 0.865 µs |

#### Proceso que tiene la llave y llaves que prueba antes de llegar a ella

| Llave | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|
| Chica | P0, 1,000,000 | P0, 1,000,000 | P0, 1,000,000 | P0, 1,000,000 |
| Mediana | P0, 8,000,000 | P0, 8,000,000 | P0, 8,000,000 | P0, 8,000,000 |
| Del grupo (2^54) | P0, 18,014,398,509,481,984 | P0, 18,014,398,509,481,984 | P1, 0 | P2, 0 |
| 2^55 + 4,000,000 | P0, 36,028,797,022,963,968 | P1, 4,000,000 | P2, 4,000,000 | P4, 4,000,000 |
| 3*2^53 + 4,000,000 | P0, 27,021,597,768,222,976 | P0, 27,021,597,768,222,976 | P1, 9,007,199,258,740,992 | P3, 4,000,000 |

#### Speedup con respecto a 1 proceso

Es el tiempo con 1 proceso dividido entre el tiempo con n procesos. Cuando uno de los dos tiempos es estimado, el speedup también lo es.

| Llave | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|
| Chica | 1.00 | 0.98 | 0.92 | 0.91 |
| Mediana | 1.00 | 0.94 | 0.91 | 0.90 |
| Del grupo (2^54) | 1.00 | 1.00 (estimado) | 8.6e+14 (estimado) | 1.1e+15 (estimado) |
| 2^55 + 4,000,000 | 1.00 | 8.4e+09 (estimado) | 8.2e+09 (estimado) | 8.1e+09 (estimado) |
| 3*2^53 + 4,000,000 | 1.00 | 1.00 (estimado) | 3.00 (estimado) | 6.1e+09 (estimado) |
