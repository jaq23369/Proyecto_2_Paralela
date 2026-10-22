#!/bin/bash
# Arranca el servidor SSH que MPI necesita para hablar con los otros nodos y
# luego ejecuta el comando que se le pase al contenedor (por defecto, bash).
service ssh start >/dev/null 2>&1
exec "$@"
