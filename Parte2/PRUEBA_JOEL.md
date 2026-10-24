# Prueba Joel / Kevin

Preparacion basada en `/Users/joel/Downloads/README_JOEL.md`, recibido el
21/10/2026. Los resultados reales se registran en `Imagen/BITACORA.md`.
Las nuevas funciones se implementaron en este repositorio porque el paquete
adicional mencionado por ese README no estaba disponible.

## Preparacion en Mac

Activar Docker Desktop > Settings > Resources > Network > Enable host networking.
Desde `Proyecto_2` (conservar `mpi_node`, que tiene las llaves anteriores):

```bash
docker build -t mpi-cluster-image Parte2/Imagen
docker run -d --network host --name mpi_cluster --hostname joel-cluster mpi-cluster-image
docker exec mpi_cluster ssh -o BatchMode=yes localhost hostname
docker exec mpi_cluster timeout 20 mpirun -np 2 /proyecto/mpi_prueba 1
docker cp Parte2/cluster_hosts.lan mpi_cluster:/proyecto/cluster_hosts
mkdir -p Parte2/.local
docker cp mpi_cluster:/root/.ssh/id_rsa.pub Parte2/.local/joel_cluster.pub
```

Esperado: `joel-cluster` y `MPI_OK: 2 procesos, 1 nodos`, token 123456789,
suma 3. Si el contenedor ya existe, inspeccionarlo; no borrarlo ni crear otro
con el mismo nombre. La prueba con argumento `2` debe fallar localmente.

La publica de Joel esta en `Parte2/.local/joel_cluster.pub` para compartir
manualmente con Kevin. Las privadas permanecen en el contenedor. `.local/`
esta excluido de Git.

## Autorizar la nueva publica de Kevin

Usar `kevin_cluster.pub` del nuevo contenedor, sin asumir que `kevin.pub`
del contenedor anterior sea la misma. Con la ruta correcta del archivo:

```bash
docker cp /ruta/kevin_cluster.pub mpi_cluster:/tmp/kevin_cluster.pub
docker exec mpi_cluster sh -c 'grep -qxF "$(cat /tmp/kevin_cluster.pub)" /root/.ssh/authorized_keys || cat /tmp/kevin_cluster.pub >> /root/.ssh/authorized_keys; chmod 600 /root/.ssh/authorized_keys'
docker exec mpi_cluster ssh -o BatchMode=yes -o ConnectTimeout=5 root@10.100.1.201 hostname
```

Kevin debe autorizar la publica de Joel y probar el sentido contrario:

```powershell
docker exec mpi_cluster ssh -o BatchMode=yes -o ConnectTimeout=5 root@10.100.5.109 hostname
```

Esperado: `kevin-cluster` y `joel-cluster`, respectivamente. El nodo nuevo
usa puerto 22; el anterior sigue configurado para 2222.

## MPI entre computadoras

Kevin debe tener los mismos fuentes y reconstruir su imagen para Windows/x86_64.
En ambos nodos, revisar `docker exec mpi_cluster ip -4 addr` y `ip -4 route`.
Las IP de la sesion son `10.100.1.201` y `10.100.5.109`; la LAN es
`10.100.0.0/19`. El hostfile limita esta primera prueba a un proceso por equipo.

Kevin ejecuta primero (valida lanzamiento, no comunicacion MPI):

```powershell
docker exec mpi_cluster timeout 30 mpirun --hostfile /proyecto/cluster_hosts --map-by ppr:1:node -np 2 hostname
```

Solo si la LAN aparece como interfaz disponible dentro de ambos contenedores:

```powershell
docker exec mpi_cluster timeout 40 mpirun --hostfile /proyecto/cluster_hosts --mca btl_tcp_if_include 10.100.0.0/19 --mca oob_tcp_if_include 10.100.0.0/19 --map-by ppr:1:node -np 2 /proyecto/mpi_prueba 2
```

Exigir `MPI_OK: 2 procesos, 2 nodos`, token 123456789, suma 3 y ambos nombres.
Un timeout (124) es fallo. Host networking de Docker Desktop funciona a nivel
TCP/UDP y no expone directamente las interfaces fisicas: si la LAN no aparece,
registrar el resultado y resolver la topologia; no forzar una subred inexistente
ni reemplazarla a ciegas por interfaces 172.x de la VM.
Fuente: https://docs.docker.com/engine/network/drivers/host/

## DES, despues de validar MPI

En cada equipo:

```bash
docker exec mpi_cluster sh -c './cifrar texto_openmpi.txt 1234 > /proyecto/prueba.b64'
```

Kevin ejecuta con el filtro ya validado:

```powershell
docker exec mpi_cluster timeout 60 mpirun --hostfile /proyecto/cluster_hosts --mca btl_tcp_if_include 10.100.0.0/19 --mca oob_tcp_if_include 10.100.0.0/19 --map-by ppr:1:node -np 2 /proyecto/bruteforce --b64 /proyecto/prueba.b64 --word "Open MPI es una"
```

Esperado: `Key = 1234`. Para el Grupo 9, cambiar la entrada a
`--b64 /proyecto/cruzado_grupo9/grupo9_cifrado.b64 --word "paralela y distribuida"`.
Comparar tiempos solo con la misma version, entrada y configuracion; no
presentar diferencias entre el equipo Windows y la Mac como speedup.
