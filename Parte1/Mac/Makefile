# Compilacion en macOS con Open MPI y OpenSSL de Homebrew
# Uso
#   make            compila bruteforce
#   make run        corre bruteforce con 4 procesos
#   make run NP=8   cambia el numero de procesos
#   make tiempos    mide el tiempo de busqueda con varias llaves (tarda unos minutos)
#   make clean      borra el ejecutable

# El mpicc de Homebrew busca un SDK de macOS que puede no existir, con el clang de Apple compila bien
export OMPI_CC := /usr/bin/clang

OPENSSL := $(shell brew --prefix openssl@3)
INC     := -I compat -I $(OPENSSL)/include
LDFLAGS := -L $(OPENSSL)/lib -lcrypto
NP      ?= 4

all: bruteforce

# Va sin optimizacion. Al armar la llave hace 0xFE << i*8 con un int, eso es
# comportamiento indefinido y con -O2 el cifrado y el descifrado arman llaves distintas
bruteforce: bruteforce_.c compat/des_crypt_openssl.c compat/rpc/des_crypt.h
	mpicc -O0 $(INC) -o $@ bruteforce_.c compat/des_crypt_openssl.c $(LDFLAGS)

run: bruteforce
	mpirun -np $(NP) ./bruteforce

tiempos: bruteforce
	python3 medir_tiempos.py

clean:
	rm -f bruteforce

.PHONY: all run tiempos clean
