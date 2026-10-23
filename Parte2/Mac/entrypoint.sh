#!/bin/bash
set -eu
# La identidad del cliente SSH se genera al crear el nodo, fuera de la imagen.
mkdir -p /run/sshd /root/.ssh
chmod 700 /root/.ssh
ssh-keygen -A
if [ ! -f /root/.ssh/id_rsa ]; then
    ssh-keygen -q -t rsa -b 3072 -N '' -f /root/.ssh/id_rsa
fi
touch /root/.ssh/authorized_keys
if ! grep -qxF "$(cat /root/.ssh/id_rsa.pub)" /root/.ssh/authorized_keys; then
    cat /root/.ssh/id_rsa.pub >> /root/.ssh/authorized_keys
fi
chmod 600 /root/.ssh/authorized_keys
service ssh start >/dev/null
exec "$@"
