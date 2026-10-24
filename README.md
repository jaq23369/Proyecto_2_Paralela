# Proyecto 2, fuerza bruta DES con OpenMPI

Proyecto del curso Computación Paralela y Distribuida, Universidad del Valle de Guatemala, sección 21. Equipo formado por Joel Jaquez (23369) y Kevin Villagrán (23584).

El proyecto busca la llave de un texto cifrado con DES en modo ECB probando todas las llaves posibles, o sea fuerza bruta. El trabajo se reparte entre varios procesos con Open MPI. En la Parte 2 esos procesos corren dentro de contenedores Docker en dos computadoras distintas, una con Windows y otra con Mac.

## De qué va

DES usa una llave de 56 bits, entonces hay 2^56 llaves posibles. El programa divide ese rango en bloques seguidos y le da uno a cada proceso. Cada proceso descifra el texto con cada llave de su bloque y revisa si aparece una palabra que ya se conoce. Cuando un proceso encuentra la llave, avisa a los demás para que paren.

Con 2168 bytes de texto, cada llave tarda unos 25 microsegundos en probarse en una de nuestras computadoras.

## Estructura del repositorio

| Carpeta | Contenido |
|---|---|
| `Parte1/` | Código de fuerza bruta, diagramas y mediciones en Windows y Mac |
| `Parte2/Imagen/` | Dockerfile, programas en C, texto de los grupos y bitácora de la imagen |
| `Parte2/Tailscale/` | Capa extra para conectar contenedores de computadoras distintas |
| `Parte2/foro/` | Entrada del foro con el texto cifrado de nuestro equipo |

## Requisitos

Docker Desktop en cada computadora. Kevin usa la versión 4.91.0 y Joel la 4.96.2. No hace falta activar Enable host networking porque la conexión entre nodos se hace con Tailscale.

Para la parte distribuida también se necesita una cuenta gratis en https://login.tailscale.com.

## Levantar el entorno

Todos los comandos se corren desde la carpeta `Proyecto 2`.

Primero se construye la imagen base. Trae Ubuntu 22.04, Open MPI, SSH y OpenSSL, y compila el programa como `/proyecto/bruteforce`.

```
docker build -t mpi-cluster-image Parte2/Imagen
```

Para comprobar que quedó bien.

```
docker run --rm mpi-cluster-image mpicc --version
docker run --rm mpi-cluster-image mpirun -np 2 echo "Proceso MPI ejecutado"
```

## Probar en una sola computadora

Esta corrida da el tiempo base (Ts). Usa el texto cifrado del Grupo 2 que ya viene en la imagen.

```
docker run -it --rm mpi-cluster-image bash
mpirun -np 4 /proyecto/bruteforce --b64 /proyecto/cruzado_grupo2/grupo2_cifrado.b64 --word "memoria distribuida"
```

La llave que debe salir es 4206649.

## Armar el clúster con dos computadoras

Los detalles están en `Parte2/Tailscale/README.md`. Estos son los pasos en resumen.

1. Una persona genera una clave de Tailscale en Settings, Keys, Generate auth key (reusable, expira en 1 día) y se la pasa a la otra por un chat privado. La clave empieza con `tskey-auth-` y no se sube al repositorio.
2. Cada quien construye la imagen de Tailscale encima de la base.

```
docker build -t mpi-cluster-ts Parte2/Tailscale
```

3. Cada quien levanta su contenedor con su nombre. Este es el de Kevin, el de Joel cambia el nombre a `joel-node`.

```
docker run -d --name mpi_ts --hostname kevin-node --cap-add NET_ADMIN --device /dev/net/tun -e TS_AUTHKEY=tskey-auth-XXXX -e TS_HOSTNAME=kevin-node mpi-cluster-ts sleep infinity
```

4. Cada quien ve su IP de Tailscale.

```
docker exec mpi_ts tailscale ip -4
```

5. El nodo maestro genera su llave SSH y la parte pública se agrega al archivo `authorized_keys` del otro contenedor. Nunca se comparte la llave privada.

```
docker exec mpi_ts sh -c 'ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa -q; cat ~/.ssh/id_rsa.pub'
```

6. El maestro crea el hostfile con las dos IPs. Los slots no deben pasar de los núcleos de cada computadora.

```
docker exec mpi_ts sh -c 'printf "<IP_KEVIN> slots=4 max_slots=4\n<IP_JOEL> slots=4 max_slots=4\n" > /proyecto/cluster_hosts'
```

7. Prueba de lanzamiento. Debe salir el nombre y la IP de los dos nodos.

```
docker exec mpi_ts mpirun --hostfile /proyecto/cluster_hosts --mca btl_tcp_if_include tailscale0 --mca oob_tcp_if_include tailscale0 --map-by ppr:1:node -np 2 sh -c 'echo $(hostname) $(tailscale ip -4)'
```

## Búsqueda distribuida

Se lanza desde el nodo maestro con 8 procesos, 4 en cada computadora.

```
docker exec mpi_ts mpirun --hostfile /proyecto/cluster_hosts --mca btl_tcp_if_include tailscale0 --mca oob_tcp_if_include tailscale0 -np 8 /proyecto/bruteforce --b64 /proyecto/cruzado_grupo2/grupo2_cifrado.b64 --word "memoria distribuida"
```

Se usa `tailscale0` en los dos parámetros porque Open MPI, si no, revisa todas las interfaces y se queda esperando una dirección de Docker (`docker0`) que existe igual en las dos computadoras y no se puede alcanzar.

## Resultados

Texto del Grupo 2, llave 4206649, nodo maestro en Windows y segundo nodo en Mac.

| Corrida | Procesos | Nodos | Tiempo | Speedup |
|---|---|---|---|---|
| Local (Ts) | 4 | 1 | 106.594 s | 1.00 |
| Distribuida (Tp) | 8 | 2 | 109.931 s | 0.97 |

No hubo mejora porque la llave está en el rango del proceso 0, que empieza en cero y tiene que probar 4.2 millones de llaves sin importar cuántos procesos haya. Repartir el trabajo ayudaría con una llave que caiga en el rango de otro nodo. La conexión entre las dos computadoras pasó por un relay de Tailscale en Miami (118 a 274 ms), nunca fue directa.

## Cuidados de seguridad

Este entorno es solo para pruebas. El acceso SSH es como root y sin contraseña, y `StrictHostKeyChecking` está apagado. Por eso se siguen estas reglas.

- La clave de Tailscale no se guarda en archivos del repositorio ni en documentos, y se revoca al terminar las pruebas.
- Solo se comparten llaves SSH públicas (`.pub`), nunca `id_rsa`.
- Al terminar se borra el contenedor con `docker rm -f mpi_ts`.

## Más información

- `Parte2/Imagen/README.md` explica los programas y el armado de la llave.
- `Parte2/Imagen/BITACORA.md` tiene lo que funcionó, lo que falló y cómo se arregló.
- `Parte2/Tailscale/README.md` tiene la guía completa del clúster y una tabla de problemas comunes.
