# Bitácora de la Parte 2

Registro de qué funcionó, qué falló y cómo se arregló mientras armamos el cluster MPI con Docker.

Equipo de prueba local, una Mac con Apple M3 Max (arm64), macOS 27.0.1, Docker 29.7.2. Dentro de los contenedores, Ubuntu 22.04, Open MPI 4.1.2, gcc 11.4.0, OpenSSL 3.0.2.

## Construcción de la imagen

| Qué | Resultado |
|---|---|
| `docker build -t mpi-cluster-image Parte2/Imagen` | Funcionó. La imagen quedó armada con los paquetes del enunciado más libssl-dev |

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

## Preparacion Joel / Kevin del 21 de octubre de 2026

Se aplico la preparacion indicada en el README_JOEL recibido en Downloads.
El repositorio aun no incluia `mpi_prueba.c`, inicio con llaves automaticas ni
hostfile LAN; se agregaron esos componentes y se reconstruyo la imagen.
Tambien se incluyo el cifrado del Grupo 9 en la imagen, `iproute2` para
diagnostico y el terminador de la lectura Base64. El acumulador del decodificador
paso a unsigned para evitar desbordamiento de un entero con signo.

Estado observado en esta sesion:

| Prueba | Resultado real |
|---|---|
| Version de Docker Desktop | 4.96.2; Engine 29.9.2, arm64, 14 CPU disponibles |
| IP y mascara de en0 | 10.100.5.109, 255.255.224.0; subred 10.100.0.0/19 |
| Contenedor anterior `mpi_node` | Existia detenido; no se elimino ni modifico |
| Nuevo `mpi_cluster` | Creado con `--network host --hostname joel-cluster`, en ejecucion |
| SSH local dentro del nuevo nodo | `ssh -o BatchMode=yes localhost hostname` devuelve `joel-cluster` |
| Colectivas MPI local con 2 procesos | `MPI_OK: 2 procesos, 1 nodos; token 123456789; suma 3` |
| Exigir 2 nodos en esa misma prueba local | `MPI_ERROR`, salida 1: no confunde procesos con maquinas distintas |
| DES con archivo Base64 y llave 1234 | Encuentra `Key = 1234`; el texto recuperado coincide exactamente con `texto_openmpi.txt` |
| Publica para Kevin | Exportada a `Parte2/.local/joel_cluster.pub`; privada conservada dentro del contenedor |
| Puerto 22 desde la Mac | Conexion rechazada tanto a 127.0.0.1 como a 10.100.5.109 |
| Interfaces del nuevo nodo | eth0 192.168.65.3/24, services1 192.168.65.6/32, loopback y bridges 172.x; no aparece la LAN 10.100.0.0/19 |

No se pudo leer la preferencia de host networking por permisos de macOS,
y el control de aplicaciones no estuvo disponible. Se solicito al usuario
activar/verificar la opcion en Settings > Resources > Network y aplicar el
reinicio antes de repetir el acceso desde el host. La ausencia de interfaz LAN
es ademas la limitacion de Docker Desktop advertida en el README recibido;
no se ejecuto MPI con un filtro LAN inexistente.

Pendiente: nueva publica `kevin_cluster.pub`, intercambio manual de publicas,
SSH por puerto 22 entre ambos nodos y comunicacion MPI distribuida. No se
declara logrado el cluster fisico ni se calcula speedup con estas pruebas.
Comandos de continuacion en `../PRUEBA_JOEL.md`; logs locales en `../.local/`.


## Clúster entre Windows y Mac con Tailscale (23 de octubre de 2026)

En clase las dos computadoras estaban en la misma Wi-Fi (10.100.0.0/19) y Kevin entró por SSH al contenedor de Joel por el puerto 2222. MPI entre las dos máquinas no se llegó a probar. Después, con red host, el contenedor de Joel solo veía la máquina virtual de Docker y no la LAN (tabla de arriba). Esta parte se terminó cada uno desde su casa, con Tailscale dentro de los contenedores. La imagen es la misma de esta carpeta y la capa de Tailscale está en `Parte2/Tailscale/`.

Equipos de esta prueba.

| Nodo | Equipo | Sistema | IP de Tailscale | slots y max_slots |
|---|---|---|---|---|
| kevin-node (maestro) | PC con Intel Core i5-12400, 6 núcleos y 12 hilos | Windows 11, Docker Desktop | 100.88.93.44 | 4 y 4 |
| joel-node | MacBook Pro con Apple M3 Max | macOS, Docker Desktop | 100.68.142.84 | 4 y 4 |

### Problema 4, Radmin VPN y la red host de Docker Desktop

Qué falló. Se intentó primero Radmin VPN para simular la misma subred. El contenedor no ve el adaptador de Radmin y, además, no hay cliente para Mac. Con la red host de Docker Desktop en Windows el contenedor solo ve la máquina virtual (192.168.65.3), no la tarjeta Wi-Fi.

Cómo se arregló. Se instaló Tailscale dentro de cada contenedor, con `--cap-add NET_ADMIN` y `--device /dev/net/tun`. Cada contenedor recibe una IP 100.x y la interfaz `tailscale0`, que se usa en `--mca btl_tcp_if_include` y `--mca oob_tcp_if_include` en lugar de una subred.

### Problema 5, el entrypoint.sh con fin de línea de Windows

Qué falló. Al construir la imagen en Windows, el `entrypoint.sh` quedó con CRLF y el contenedor no arrancaba, con el error `exec /proyecto/entrypoint.sh: no such file or directory`. Las validaciones del enunciado fallaban por eso.

Cómo se arregló. Se agregó `sed -i 's/$//'` al Dockerfile, tanto el de la imagen como el de Tailscale, antes del `chmod +x`.

### Problema 6, detalles al unir los nodos

| Qué pasó | Cómo se resolvió |
|---|---|
| `ping` no existe en la imagen | Se usó `tailscale ping`, que ya viene con Tailscale |
| El contenedor de Kevin no tenía llave SSH, porque se creó con una versión anterior de la imagen | Se generó con `ssh-keygen -t rsa -N "" -f /root/.ssh/id_rsa` y se pegó la pública de cada uno en el `authorized_keys` del otro |
| `mpirun` dio `No se pudo leer el archivo` con el texto del Grupo 9 | Al nodo de Kevin le faltaba `cruzado_grupo9/`. Se copió el archivo y se comprobó con `md5sum` que fuera el mismo en los dos nodos. Después se agregó la carpeta al Dockerfile |
| El archivo copiado desde Windows traía fin de línea CRLF y no era idéntico al de Joel | Se quitó el `` y los `md5sum` quedaron iguales |

### Pruebas del clúster

| Prueba | Resultado |
|---|---|
| `tailscale ping` entre los nodos | Responde, pero sin conexión directa. Pasa por el relay de Miami y tarda entre 118 y 274 ms |
| `ssh -o BatchMode=yes root@<IP del otro> hostname` en los dos sentidos | Funcionó. Imprime `joel-node` y `kevin-node` sin contraseña |
| `mpirun --hostfile cluster_hosts --map-by ppr:1:node -np 2 hostname` | Funcionó. Imprime `kevin-node` y `joel-node` |
| Demo interna con la llave 2⁵⁵ y 2 procesos | Funcionó. El proceso 1, en `joel-node`, encontró la llave y avisó al maestro. Tiempo 0.125944 s |

### Resultados con textos de otros equipos

Todas las corridas con `bruteforce_fix` (armado corregido), el texto en base64 con `--b64` y el mismo comando de la Parte 2. El Ts es una corrida con 4 procesos en un solo nodo (Kevin, Windows) y el Tp es una corrida con 8 procesos, 4 en cada nodo.

| Grupo | Palabra objetivo | Llave | Ts (4 procesos, 1 nodo) | Tp (8 procesos, 2 nodos) | Speedup |
|---|---|---|---|---|---|
| 9 (prueba) | paralela y distribuida | 3141592 | 82.013272 s | 85.837373 s | 0.96 |
| 2 (el real) | memoria distribuida | 4206649 | 106.593799 s | 109.930869 s | 0.97 |

La eficiencia del Grupo 2 con 8 procesos es 0.97 / 8 = 0.12. El Ts de 54.9 s de arriba es de la Mac y no se compara con estos, porque son equipos distintos.

Por qué no hubo speedup. Las dos llaves están en el rango del proceso 0, que empieza en 0 y tiene que probar 4.2 millones de llaves (3.1 millones en el Grupo 9) sin importar cuántos procesos haya. Los otros procesos buscan en rangos donde la llave no está. Cada llave costó 25.3 µs en local y 26.1 µs en el clúster para el Grupo 2. Los 3.3 s de más pueden venir del arranque por SSH en el otro nodo y de la latencia del relay, pero no se midió por separado.

Lo que quedaba pendiente en "Lo que falta probar en la red física" se hizo con Tailscale y no en la red del router. Queda sin probar la red física del mismo router y una llave que caiga en el rango de otro nodo, para ver un caso donde el clúster sí ayude.
