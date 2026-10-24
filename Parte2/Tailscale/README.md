# Cluster MPI con Tailscale (para cuando no estamos en la misma red)

Esta carpeta agrega una capa opcional encima de `mpi-cluster-image`. Si el contenedor recibe una clave de Tailscale, entra a una VPN y tiene su propia IP (empieza con 100.). Así dos contenedores en computadoras distintas, Windows y Mac, se ven entre ellos sin depender de la red host de Docker.

Si no se pasa la clave, el contenedor arranca igual que la imagen base. Cuando estemos en el mismo router se puede seguir como en `Parte2/Imagen/README.md`, aunque en Windows la red host de Docker Desktop no ve la tarjeta de red real, así que Tailscale puede seguir siendo la opción más segura.

Radmin VPN no sirve para esto porque el contenedor no ve ese adaptador, y no tiene cliente para Mac.

## 1. Clave de Tailscale (solo la hace una persona)

1. Crear una cuenta gratis en https://login.tailscale.com
2. Ir a Settings, Keys, Generate auth key. Marcar Reusable y dejar que expire en 1 día.
3. Pasarle la clave (empieza con `tskey-auth-`) a la otra persona por un chat privado. No subirla al repo ni al docx.
4. Al terminar la prueba, revocar la clave en la misma pantalla.

## 2. Construir las imágenes (cada quien)

Desde la carpeta `Proyecto 2`.

```
docker build -t mpi-cluster-image Parte2/Imagen
docker build -t mpi-cluster-ts Parte2/Tailscale
```

## 3. Levantar el contenedor (cada quien, con su nombre)

Kevin, en PowerShell.

```
docker run -d --name mpi_ts --hostname kevin-node --cap-add NET_ADMIN --device /dev/net/tun -e TS_AUTHKEY=tskey-auth-XXXX -e TS_HOSTNAME=kevin-node mpi-cluster-ts sleep infinity
```

Joel, en su terminal. Es el mismo comando cambiando el nombre a `joel-node`.

Para ver la IP de cada contenedor.

```
docker exec mpi_ts tailscale ip -4
```

Cada quien debe poder hacer ping a la IP del otro.

```
docker exec mpi_ts ping -c 3 <IP_DEL_OTRO>
```

## 4. Llaves SSH (Paso A del PDF)

Kevin es el maestro. Genera su llave dentro del contenedor.

```
docker exec mpi_ts sh -c 'ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa -q; cat ~/.ssh/id_rsa.pub'
```

Joel pega esa llave en su contenedor.

```
docker exec mpi_ts sh -c 'echo "<LLAVE_PUBLICA_DE_KEVIN>" >> ~/.ssh/authorized_keys'
```

Prueba desde Kevin. Debe imprimir `joel-node` sin pedir contraseña.

```
docker exec mpi_ts ssh -o BatchMode=yes root@<IP_DE_JOEL> hostname
```

## 5. Hostfile y primera prueba MPI

Kevin crea el hostfile con las IPs de Tailscale. Los slots no deben pasar de los núcleos de cada computadora.

```
docker exec mpi_ts sh -c 'printf "<IP_DE_KEVIN> slots=4 max_slots=4\n<IP_DE_JOEL> slots=4 max_slots=4\n" > /proyecto/cluster_hosts'
```

Prueba de lanzamiento. Debe imprimir dos nombres distintos.

```
docker exec mpi_ts mpirun --hostfile /proyecto/cluster_hosts --mca btl_tcp_if_include tailscale0 --mca oob_tcp_if_include tailscale0 --map-by ppr:1:node -np 2 hostname
```

Se usa `tailscale0` en lugar de una subred porque es el nombre de la interfaz que crea Tailscale y es el mismo en las dos computadoras.

## 6. Búsqueda distribuida con el texto del Grupo 2

El archivo `cruzado_grupo2/grupo2_cifrado.b64` viene en la imagen de los dos. Si algún nodo se construyó antes de que existiera, hay que copiarlo con `docker cp` y comprobar que el `md5sum` sea el mismo en los dos nodos.

Primero el tiempo base local (Ts), con 4 procesos y sin hostfile.

```
docker exec mpi_ts mpirun -np 4 /proyecto/bruteforce --b64 /proyecto/cruzado_grupo2/grupo2_cifrado.b64 --word "memoria distribuida"
```

Después la búsqueda distribuida (Tp). Kevin corre.

```
docker exec mpi_ts mpirun --hostfile /proyecto/cluster_hosts --mca btl_tcp_if_include tailscale0 --mca oob_tcp_if_include tailscale0 -np 8 /proyecto/bruteforce --b64 /proyecto/cruzado_grupo2/grupo2_cifrado.b64 --word "memoria distribuida"
```

Las dos deben salir con `Key = 4206649`. Con `-np 8` y los slots anteriores, MPI reparte 4 procesos por computadora.

Resultados de la prueba del 23 de octubre de 2026, con el nodo de Kevin (Windows) como maestro.

| Corrida | Procesos | Nodos | Tiempo | Speedup |
|---|---|---|---|---|
| Local (Ts) | 4 | 1 | 106.594 s | 1.00 |
| Distribuida (Tp) | 8 | 2 | 109.931 s | 0.97 |

La llave está en el rango del proceso 0, que tiene que probar 4.2 millones de llaves sin importar cuántos procesos haya, por eso repartir la búsqueda no baja el tiempo. Con el texto del Grupo 9 (`--b64 /proyecto/cruzado_grupo9/grupo9_cifrado.b64 --word "paralela y distribuida"`, llave 3141592) pasó lo mismo, 82.013 s en local y 85.837 s distribuido. Para comparar hay que usar el mismo equipo y el mismo número de procesos en el Ts, no el tiempo de la Mac.

## Si algo falla

| Síntoma | Qué revisar |
|---|---|
| `tailscale ip -4` no imprime nada | La clave expiró o está mal. `docker logs mpi_ts` y `docker exec mpi_ts cat /var/log/tailscaled.log` |
| No existe `/dev/net/tun` | Falta `--device /dev/net/tun` o `--cap-add NET_ADMIN` en el `docker run` |
| El ping funciona pero SSH no | Falta pegar la llave pública, o el sshd del contenedor no arrancó (`docker exec mpi_ts service ssh status`) |
| `mpirun` se queda congelado | Revisar que las dos IPs del hostfile sean las de Tailscale y que se use `tailscale0` en los dos `--mca` |
| `/proyecto/bruteforce` no existe en el otro nodo | El otro nodo tiene que haber construido la misma imagen |
