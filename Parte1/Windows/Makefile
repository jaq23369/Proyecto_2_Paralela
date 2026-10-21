# Compilacion en Windows con MS-MPI y MSYS2 (UCRT64) con OpenSSL
# Se corre desde una terminal de MSYS2 UCRT64, o con C:\msys64\ucrt64\bin y
# C:\Program Files\Microsoft MPI\Bin en el PATH
# Uso
#   make            compila bruteforce.exe
#   make run        corre bruteforce con 4 procesos
#   make run NP=8   cambia el numero de procesos
#   make tiempos    mide el tiempo de busqueda con varias llaves (tarda unos minutos)
#   make clean      borra el ejecutable (solo desde una terminal de MSYS2)
#   En PowerShell el make se llama mingw32-make

INC     := -I compat
LDFLAGS := -lcrypto
NP      ?= 4

all: bruteforce.exe

# Va sin optimizacion por lo mismo que en la Mac, el 0xFE << i*8 del original es comportamiento indefinido
bruteforce.exe: bruteforce_.c compat/des_crypt_openssl.c compat/rpc/des_crypt.h
	mpicc -O0 $(INC) -o $@ bruteforce_.c compat/des_crypt_openssl.c $(LDFLAGS)

run: bruteforce.exe
	mpiexec -n $(NP) bruteforce.exe

tiempos: bruteforce.exe
	python medir_tiempos.py

clean:
	rm -f bruteforce.exe

.PHONY: all run tiempos clean
