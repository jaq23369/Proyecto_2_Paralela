# Capa opcional de Tailscale encima de mpi-cluster-image.
# Sirve para unir contenedores de computadoras que no estan en la misma red fisica.
# Primero se construye la imagen base y luego esta:
#   docker build -t mpi-cluster-image Parte2/Imagen
#   docker build -t mpi-cluster-ts Parte2/Tailscale
FROM mpi-cluster-image

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y curl ca-certificates iproute2 iptables \
 && curl -fsSL https://tailscale.com/install.sh | sh \
 && rm -rf /var/lib/apt/lists/*

COPY entrypoint-ts.sh /proyecto/entrypoint-ts.sh
RUN sed -i 's/\r$//' /proyecto/entrypoint-ts.sh && chmod +x /proyecto/entrypoint-ts.sh

ENTRYPOINT ["/proyecto/entrypoint-ts.sh"]
CMD ["/bin/bash"]
