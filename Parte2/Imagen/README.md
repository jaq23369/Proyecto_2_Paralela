# Parte 2, cluster MPI con Docker

Esta carpeta lleva el programa de fuerza bruta al cluster. Cada integrante corre un contenedor Docker igual, los contenedores se conectan por un router y juntos buscan la llave del texto cifrado de otro equipo.

El programa es `bruteforce_fix.c`, la versión que cifra y descifra todo el texto. Todos los equipos modificaron el original así, para cifrar el texto completo y encontrar una frase en el texto ya descifrado.

El enunciado pide el Dockerfile en la raíz del proyecto. Aquí está en `Parte2/Imagen/` para tener junto todo el trabajo de la imagen, que usamos igual en Windows y en Mac. Si el profesor lo pide en la raíz, se mueve el Dockerfile y se cambia el contexto del build.

## Qué hay en la carpeta

| Archivo | Para qué sirve |
|---|---|
| `Dockerfile` | Construye la imagen `mpi-cluster-image` con Ubuntu 22.04, Open MPI, SSH y OpenSSL |
| `entrypoint.sh` | Arranca el servidor SSH al iniciar el contenedor |
| `bruteforce_fix.c` | El programa del cluster, con el nombre del nodo en la salida y la lectura de base64 |
| `cifrar.c` | Cifra un texto y saca base64, para publicarlo a otro equipo |
| `mpi_prueba.c` | Comprueba colectivas MPI y el numero de nodos distintos |
| `compat/` | Reemplazo de `rpc/des_crypt.h` con OpenSSL, Ubuntu 22.04 ya no lo trae |
| `cluster_hosts` | Ejemplo de hostfile con las IPs, slots y max_slots |
| `texto_openmpi.txt` | El texto del grupo |
| `cruzado_grupo9/` | El texto cifrado del Grupo 9 que se descifró en la prueba |
| `BITACORA.md` | Qué funcionó, qué falló y cómo se arregló |
| `NOTAS.md` | Apuntes de la versión fix de la Parte 1 |

## Requisitos

Docker Desktop 4.34 o superior, con "Enable host networking" en Settings, Resources, Network. Todas las computadoras del equipo en el mismo router.

Para la sesion Joel/Kevin en la misma red, ver `../cluster_hosts.lan`.
La imagen permanece activa por defecto (`sleep infinity`); para abrir una
terminal usar `docker exec -it mpi_cluster bash`. El inicio genera una identidad
SSH por contenedor y autoriza su propia publica para probar `ssh localhost`.

## 1. Construir la imagen

Cada integrante corre esto desde la carpeta `Proyecto_2`.

```
docker build -t mpi-cluster-image Parte2/Imagen
```

## 2. Validar la imagen

```
docker run --rm mpi-cluster-image mpicc --version
docker run --rm mpi-cluster-image mpirun -np 2 echo "Proceso MPI ejecutado"
docker run --rm mpi-cluster-image bash -lc 'ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa >/dev/null 2>&1; cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys; ssh -o StrictHostKeyChecking=no localhost echo "Conexion SSH sin contrasena exitosa"'
```

Los resultados esperados están en `BITACORA.md`.

## 3. Probar el programa en una sola máquina

```
docker run -it --rm mpi-cluster-image bash
# ya dentro del contenedor:
mpirun -np 4 ./bruteforce              # texto propio, llave 2^54
mpirun -np 4 ./bruteforce 8000000      # texto propio, otra llave
```

## 4. Descifrado cruzado con el texto de otro equipo

El programa trae dos modos. Sin archivo cifra su propio texto, para probar. Con `--b64` o `--file` carga el texto cifrado de otro equipo y busca su llave.

Para entregar el texto del grupo, se cifra con `cifrar`, que saca base64.

```
./cifrar texto_openmpi.txt 18014398509481984 > nuestro_cifrado.b64
```

Ese base64 se sube a Canvas con la palabra objetivo. Como se cifra todo el texto, la palabra puede estar en cualquier parte.

Para romper el texto de otro equipo, se guarda su base64 en un archivo y se corre.

```
mpirun -np 4 ./bruteforce --b64 el_cifrado_de_ellos.b64 --word "palabra objetivo"
```

### El armado de la llave

El programa arma la llave DES con el armado corregido por defecto, que es el de `bruteforce_fix.c`. Se comprobó que es el que usan los equipos al romper el texto del Grupo 9. Si un equipo dejara el armado original del código base, se agrega `--keyorig`.

```
mpirun -np 4 ./bruteforce --b64 el_cifrado_de_ellos.b64 --word "palabra objetivo" --keyorig
```

## 5. El cluster en la red física

Esta es la parte que el enunciado llama el reto real. Se arma entre todas las computadoras del equipo conectadas al router.

### Levantar el contenedor con la red del host

En cada computadora.

```
docker run -it --network host --name mpi_node mpi-cluster-image /bin/bash
```

Con `--network host` el contenedor usa la red de la computadora, si no, queda escondido detrás del NAT de Docker y los otros nodos no lo alcanzan.

### Paso A, llaves SSH entre nodos

Desde el contenedor del nodo maestro se genera una llave y se copia al `authorized_keys` de cada worker. Dentro del maestro.

```
ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa
cat ~/.ssh/id_rsa.pub          # se copia esta salida
```

En cada worker se pega esa llave.

```
echo "<la llave publica del maestro>" >> ~/.ssh/authorized_keys
```

Se prueba desde el maestro con `ssh <ip_del_worker> echo conectado`.

### Paso B, el ataque distribuido

Se arma el hostfile con las IPs que dio el router, por ejemplo en `cluster_hosts`, y desde el maestro se corre. Hay que cambiar `192.168.1.0/24` por la subred real, y el archivo cifrado tiene que estar en la misma ruta en todos los contenedores.

```
mpirun --hostfile cluster_hosts \
  --mca btl_tcp_if_include 192.168.1.0/24 \
  --mca oob_tcp_if_include 192.168.1.0/24 \
  -np <TOTAL_SLOTS> ./bruteforce --b64 el_cifrado.b64 --word "palabra objetivo"
```

Las banderas `--mca` obligan a MPI a usar solo la subred del router. Sin ellas, MPI intenta pasar el tráfico por las interfaces virtuales de Docker y se congela sin dar error.
