# Bitácora de la Parte 2

Registro de qué funcionó, qué falló y cómo se arregló mientras armamos el cluster MPI con Docker.

Equipo de prueba local, una Mac con Apple M3 Max (arm64), macOS 27.0.1, Docker 29.7.2. Dentro de los contenedores, Ubuntu 22.04, Open MPI 4.1.2, gcc 11.4.0, OpenSSL 3.0.2.

## Construcción de la imagen

| Qué | Resultado |
|---|---|
| `docker build -t mpi-cluster-image Parte2/Mac` | Funcionó. La imagen quedó armada con los paquetes del enunciado más libssl-dev |

### Problema 1, Ubuntu 22.04 no trae rpc/des_crypt.h

Qué falló. El programa usa `ecb_crypt` y `des_setparity` de `rpc/des_crypt.h`. Se comprobó dentro de `ubuntu:22.04` que ese archivo ya no existe, porque glibc 2.35 quitó las funciones de Sun RPC. Con solo los paquetes que pide el enunciado (`build-essential`, `openmpi-bin`, `libopenmpi-dev`, `openssh-server`) el programa no compila.

Cómo se arregló. Se agregó `libssl-dev` al `apt-get` y se usa el mismo reemplazo de la carpeta `compat` que ya se había hecho para Mac y Windows, con `-I compat` y `-lcrypto`. Así el código del programa no se toca y DES sigue siendo el mismo.

### Problema 2, correr OpenMPI como root

Qué falló. Dentro del contenedor el usuario es root, y OpenMPI se niega a correr como root por seguridad.

Cómo se arregló. Se pusieron en el Dockerfile las dos variables que indica el enunciado, `OMPI_ALLOW_RUN_AS_ROOT=1` y `OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1`.

## Tabla de validación del enunciado

Todas las pruebas se corrieron sobre la imagen `mpi-cluster-image`.

| Validar | Esperado | ¿Funcionó? | Qué se tuvo que modificar o hacer |
|---|---|---|---|
| `docker run --rm mpi-cluster-image mpicc --version` | gcc (Ubuntu 11.4.0) | Sí, imprime `gcc (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0` | Nada |
| `docker run --rm mpi-cluster-image mpirun -np 2 echo "Proceso MPI ejecutado"` | El mensaje 2 veces | Sí, lo imprime 2 veces | Las variables OMPI_ALLOW_RUN_AS_ROOT en el Dockerfile, si no, OpenMPI se niega a correr como root |
| `docker run -it --rm mpi-cluster-image /bin/bash` | Entra a la terminal | Sí | El entrypoint arranca SSH y luego deja la bash |
| `service ssh start` | Arranca SSH | Sí | El entrypoint ya lo hace al iniciar el contenedor |
| `ssh-keygen` y copiar a `authorized_keys` | Genera la llave | Sí | Nada |
| `ssh localhost echo "..."` | Imprime el mensaje sin contraseña | Sí, imprime "Conexion SSH sin contrasena exitosa" | `PermitRootLogin yes` y `StrictHostKeyChecking no` en el Dockerfile |

## Prueba del programa dentro del contenedor

| Prueba | Resultado |
|---|---|
| `mpirun -np 4 ./bruteforce` (texto propio, llave 2⁵⁴) | Encuentra la llave y recupera el texto completo. Cada línea muestra el nodo del proceso |
| `./cifrar t.txt 2500000 > t.b64` y luego `mpirun -np 4 ./bruteforce --b64 t.b64 --word "..."` | Encuentra la llave 2500000 y recupera el texto. Así se hará el descifrado cruzado con el texto de otro equipo |

## Prueba del cluster entre dos contenedores (en una sola Mac)

Antes de la red física, se simuló el cluster con dos contenedores en una red Docker de tipo bridge llamada `mpi-lan`, para validar el camino de SSH y MPI entre nodos.

| Paso | Resultado |
|---|---|
| Crear la red y los contenedores `mpi_master` (172.26.0.2) y `mpi_worker` (172.26.0.3) | Funcionó |
| Paso A, el maestro genera su llave SSH y se copia al `authorized_keys` del worker | Funcionó |
| Paso B, `mpirun --hostfile ... --mca btl_tcp_if_include ... -np 4 ./bruteforce 36028797018963968` | Funcionó. Los procesos 0 y 1 corrieron en el nodo master y los procesos 2 y 3 en el nodo worker. El worker encontró la llave |

### Problema 3, parecía que el SSH maestro a worker no respondía

Qué pasó. En el primer intento el comando solo mostró el aviso de "Permanently added worker" y no el mensaje de éxito.

Cómo se revisó. Se confirmó que `sshd` estaba corriendo en el worker con `service ssh status` y se repitió la conexión. La segunda vez sí imprimió el mensaje, entonces fue solo el orden en que salió el texto la primera vez, no un fallo real.

## Descifrado cruzado, formato de los otros equipos

Qué se descubrió. Los otros equipos no cifran solo 16 bytes como el código original, sino todo el texto, y lo publican en base64. El texto cifrado del Grupo 9 decodificado da 2168 bytes, que son 271 bloques completos de 8 bytes.

Cómo se arregló. Se usa `bruteforce_fix.c`, la versión de texto completo. `decrypt` descifra todos los bloques. `bruteforce` lee el archivo en base64 con `--b64` o binario con `--file`, y la palabra objetivo puede estar en cualquier parte del texto. `cifrar` saca el resultado en base64 para publicarlo.

### Descifrado del texto del Grupo 9

Se decodificó el base64 del Grupo 9 (2168 bytes, 271 bloques). Primero se probó con el armado original y no apareció en las primeras 300 millones de llaves. Luego se probó con el armado corregido y sí apareció.

| Dato | Valor |
|---|---|
| Llave del Grupo 9 | 3141592 |
| Armado que usaron | corregido (`0xFEUL`), el mismo de `bruteforce_fix.c` |
| Tiempo base local (Ts) | 54.9 s con 4 procesos en un contenedor en la Mac |
| Texto recuperado | "La computación paralela dejó de ser un tema exclusivo de los centros de investigación..." |

Esto confirma que los equipos cifran con el armado corregido, no con el del código base. Por eso el cluster usa el corregido por defecto. La opción `--keyorig` queda por si algún equipo usara el armado original.

Este Ts de 54.9 s es el punto de referencia para comparar contra el tiempo del cluster físico, como pide la sección III del enunciado.

## Lo que falta probar en la red física (no se puede en una sola Mac)

Estos puntos dependen de tener las computadoras del equipo conectadas al router, y es donde el enunciado dice que está el reto real.

- Correr los contenedores con `--network host` y habilitar "Enable host networking" en Docker Desktop 4.34 o superior.
- Abrir el firewall de cada sistema para las conexiones entrantes en la red privada.
- El bloqueo indefinido de MPI cuando detecta las interfaces virtuales de Docker en varias máquinas. Se evita limitando el tráfico a la subred del router con `--mca btl_tcp_if_include` y `--mca oob_tcp_if_include`, como ya se probó local.
- Para el descifrado cruzado con `--b64` o `--file`, el archivo tiene que estar en la misma ruta en todos los nodos.
