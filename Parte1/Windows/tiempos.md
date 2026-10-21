Medido el 21/10/2026 en Windows 11 (AMD64), con 5 corridas por combinación. El tiempo es el que imprime el proceso 0 con MPI_Wtime, desde que todos los procesos empiezan a buscar hasta que le llega la llave.

Velocidad con 1 proceso, 1,750,881 llaves por segundo.

#### Tiempo promedio y desviación estándar

| Llave | Valor | 1 proceso | 2 procesos | 4 procesos | 8 procesos |
|---|---|---|---|---|---|
| Chica | 1,000,000 | 0.571 ± 0.011 s | 0.603 ± 0.010 s | 0.648 ± 0.014 s | 0.760 ± 0.013 s |
| Mediana | 8,000,000 | 4.761 ± 0.147 s | 4.970 ± 0.036 s | 5.348 ± 0.064 s | 6.199 ± 0.084 s |
| Del grupo (2^54) | 18,014,398,509,481,984 | 326 años (estimado) | 326 años (estimado) | 101 ± 31 µs | 218 ± 180 µs |
| 2^55 + 4,000,000 | 36,028,797,022,963,968 | 652 años (estimado) | 2.479 ± 0.057 s | 2.645 ± 0.057 s | 3.130 ± 0.092 s |
| 3*2^53 + 4,000,000 | 27,021,597,768,222,976 | 489 años (estimado) | 489 años (estimado) | 163 años (estimado) | 3.053 ± 0.088 s |

#### Tiempo adecuado de cada llave con 1 proceso

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.584 s | 0.568 s | 0.580 s | 0.555 s | 0.569 s | 0.571 s | 0.011 s | 0.571 µs |
| Mediana | 4.651 s | 4.585 s | 4.954 s | 4.834 s | 4.781 s | 4.761 s | 0.147 s | 0.595 µs |
| Del grupo (2^54) | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 326 años (estimado) | - | - |
| 2^55 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 652 años (estimado) | - | - |
| 3*2^53 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 489 años (estimado) | - | - |
| Promedio con 1 proceso |  |  |  |  |  | 2.666 s | - | 0.583 µs |

#### Tiempo adecuado de cada llave con 2 procesos

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.617 s | 0.605 s | 0.589 s | 0.604 s | 0.597 s | 0.603 s | 0.010 s | 0.603 µs |
| Mediana | 4.974 s | 4.968 s | 4.989 s | 4.913 s | 5.008 s | 4.970 s | 0.036 s | 0.621 µs |
| Del grupo (2^54) | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 326 años (estimado) | - | - |
| 2^55 + 4,000,000 | 2.545 s | 2.501 s | 2.452 s | 2.503 s | 2.396 s | 2.479 s | 0.057 s | 0.620 µs |
| 3*2^53 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 489 años (estimado) | - | - |
| Promedio con 2 procesos |  |  |  |  |  | 2.684 s | - | 0.615 µs |

#### Tiempo adecuado de cada llave con 4 procesos

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.632 s | 0.640 s | 0.647 s | 0.651 s | 0.671 s | 0.648 s | 0.014 s | 0.648 µs |
| Mediana | 5.314 s | 5.373 s | 5.359 s | 5.433 s | 5.263 s | 5.348 s | 0.064 s | 0.669 µs |
| Del grupo (2^54) | 66 µs | 84 µs | 86 µs | 126 µs | 141 µs | 101 µs | 31 µs | - |
| 2^55 + 4,000,000 | 2.610 s | 2.564 s | 2.664 s | 2.687 s | 2.700 s | 2.645 s | 0.057 s | 0.661 µs |
| 3*2^53 + 4,000,000 | no se corrió | no se corrió | no se corrió | no se corrió | no se corrió | 163 años (estimado) | - | - |
| Promedio con 4 procesos |  |  |  |  |  | 2.160 s | - | 0.659 µs |

#### Tiempo adecuado de cada llave con 8 procesos

| Llave | Corrida 1 | Corrida 2 | Corrida 3 | Corrida 4 | Corrida 5 | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |
|---|---|---|---|---|---|---|---|---|
| Chica | 0.744 s | 0.764 s | 0.755 s | 0.780 s | 0.757 s | 0.760 s | 0.013 s | 0.760 µs |
| Mediana | 6.109 s | 6.137 s | 6.324 s | 6.207 s | 6.218 s | 6.199 s | 0.084 s | 0.775 µs |
| Del grupo (2^54) | 79 µs | 526 µs | 184 µs | 101 µs | 198 µs | 218 µs | 180 µs | - |
| 2^55 + 4,000,000 | 3.141 s | 3.029 s | 3.046 s | 3.241 s | 3.195 s | 3.130 s | 0.092 s | 0.783 µs |
| 3*2^53 + 4,000,000 | 3.053 s | 3.058 s | 2.930 s | 3.179 s | 3.046 s | 3.053 s | 0.088 s | 0.763 µs |
| Promedio con 8 procesos |  |  |  |  |  | 2.629 s | - | 0.770 µs |

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
| Chica | 1.00 | 0.95 | 0.88 | 0.75 |
| Mediana | 1.00 | 0.96 | 0.89 | 0.77 |
| Del grupo (2^54) | 1.00 | 1.00 (estimado) | 1.0e+14 (estimado) | 4.7e+13 (estimado) |
| 2^55 + 4,000,000 | 1.00 | 8.3e+09 (estimado) | 7.8e+09 (estimado) | 6.6e+09 (estimado) |
| 3*2^53 + 4,000,000 | 1.00 | 1.00 (estimado) | 3.00 (estimado) | 5.1e+09 (estimado) |
