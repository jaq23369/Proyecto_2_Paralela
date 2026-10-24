# Imagen del cluster MPI para la Parte 2
# Construir con:  docker build -t mpi-cluster-image Parte2/Imagen
FROM ubuntu:22.04

# Que apt no haga preguntas durante la construccion
ENV DEBIAN_FRONTEND=noninteractive

# Dependencias que pide el enunciado, mas libssl-dev.
# Ubuntu 22.04 (glibc 2.35) ya no trae rpc/des_crypt.h, entonces ecb_crypt y
# des_setparity se toman de OpenSSL a traves de la carpeta compat.
RUN apt-get update && apt-get install -y \
    build-essential \
    openmpi-bin \
    libopenmpi-dev \
    openssh-server \
    iproute2 \
    libssl-dev \
 && rm -rf /var/lib/apt/lists/*

# OpenMPI prohibe correr como root. Dentro del contenedor somos root, entonces
# se habilita con estas dos variables, tal como indica el enunciado.
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

# SSH sin trabas para que MPI pueda levantar procesos en los otros nodos.
# Se permite el acceso root y se apaga la verificacion manual de huellas.
RUN mkdir -p /var/run/sshd \
 && sed -i 's/#\?PermitRootLogin.*/PermitRootLogin yes/' /etc/ssh/sshd_config \
 && printf '\nStrictHostKeyChecking no\nUserKnownHostsFile /dev/null\n' >> /etc/ssh/ssh_config \
 && echo 'root:mpi' | chpasswd

WORKDIR /proyecto

# Codigo, texto, reemplazo de DES y archivos de apoyo
COPY compat/ ./compat/
COPY bruteforce_fix.c cifrar.c mpi_prueba.c texto_openmpi.txt cluster_hosts entrypoint.sh ./
COPY cruzado_grupo9/ ./cruzado_grupo9/
COPY cruzado_grupo2/ ./cruzado_grupo2/

# Compilar con el reemplazo de OpenSSL. El binario se llama bruteforce.
# Va sin optimizacion por el mismo motivo que en la Parte 1, el armado original
# de la llave es comportamiento indefinido.
RUN mpicc -O0 -I compat -o bruteforce bruteforce_fix.c compat/des_crypt_openssl.c -lcrypto \
 && gcc  -O0 -I compat -o cifrar     cifrar.c         compat/des_crypt_openssl.c -lcrypto \
 && sed -i 's/\r$//' entrypoint.sh \
 && mpicc -O2 -Wall -Wextra -o mpi_prueba mpi_prueba.c \
 && chmod +x entrypoint.sh

# El entrypoint arranca el servidor SSH y luego ejecuta lo que se le pida.
ENTRYPOINT ["/proyecto/entrypoint.sh"]
CMD ["sleep", "infinity"]
