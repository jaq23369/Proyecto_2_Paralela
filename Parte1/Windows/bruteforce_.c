#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
// Cambio para Windows: en Windows un long mide 32 bits y las llaves llegan a 2^56, por eso todos los long pasaron a long long
#include <rpc/des_crypt.h>




void decrypt(long long key, char *ciph, int len){
  long long k = 0;
  for(int i=0; i<8; ++i){
    key <<= 1;
    k += (key & (0xFE << i*8));
  }
  des_setparity((char *)&k);
  ecb_crypt((char *)&k, (char *) ciph, 16, DES_DECRYPT);
}




void encrypt(long long key, char *ciph){
  long long k = 0;
  for(int i=0; i<8; ++i){
    key <<= 1;
    k += (key & (0xFE << i*8));
  }
  des_setparity((char *)&k); 
  ecb_crypt((char *)&k, (char *) ciph, 16, DES_ENCRYPT);
}




// Cambio: la frase original " los mismos del proyecto 1 " nunca coincidia con el texto y quedaba fuera de los 16 bytes que se cifran.
// La frase tiene que estar dentro de los primeros 16 bytes del texto
char search[] = "Open MPI es una";
int tryKey(long long key, char *ciph, int len){
  char temp[len+1]; 
  memcpy(temp, ciph, len);
  temp[len]=0;
  decrypt(key, temp, len);
  return strstr((char *)temp, search) != NULL;
}

// Texto del grupo sobre Open MPI (352 palabras, ver texto_openmpi.txt)
char eltexto[] =
  "Open MPI es una implementación libre y de código abierto del estándar MPI, que son las siglas de Message Passing Interface. MPI define un conjunto de funciones para que varios procesos trabajen juntos en un mismo problema mandándose mensajes entre ellos. Cada proceso tiene su propia memoria y no puede ver los datos de los demás, entonces la única forma de compartir información es enviarla y recibirla de forma explícita.\n"
  "El proyecto empezó en 2004, cuando se juntaron los equipos de varias implementaciones que ya existían, como LAM/MPI, LA-MPI y FT-MPI. La idea era unir lo mejor de cada una en un solo programa y que lo mantuviera una comunidad de universidades, laboratorios y empresas. Hoy Open MPI se usa en muchas supercomputadoras, pero también se puede instalar en una laptop para aprender.\n"
  "Para usarlo se escribe un programa normal en C, C++ o Fortran y se agregan las llamadas de MPI. El programa empieza con MPI_Init y termina con MPI_Finalize. En medio, cada proceso pregunta cuántos procesos hay en total con MPI_Comm_size y cuál es su número con MPI_Comm_rank. Con ese número, llamado rank, cada proceso sabe qué parte del trabajo le toca.\n"
  "Los mensajes se mandan con funciones como MPI_Send y MPI_Recv, que esperan hasta que la comunicación termina. También hay versiones que no se quedan esperando, como MPI_Isend y MPI_Irecv, y que permiten seguir trabajando mientras el mensaje viaja. Además existen operaciones colectivas, como MPI_Bcast para mandar un dato a todos los procesos o MPI_Reduce para juntar los resultados de todos en uno solo.\n"
  "Para compilar se usa mpicc, que es el compilador de C con las opciones de MPI ya agregadas, y para correr el programa se usa mpirun, indicando cuántos procesos se quieren con la opción -np. Los procesos pueden estar en la misma computadora o repartidos en varias máquinas conectadas por una red, y el código no tiene que cambiar.\n"
  "Gracias a esto, un problema muy grande se puede dividir en partes más pequeñas y resolverse en mucho menos tiempo, siempre que el trabajo se reparta bien y los procesos no pasen demasiado tiempo esperando mensajes.";
long long the_key = 18014398509481984;


int main(int argc, char *argv[]){ 
  int N, id;
  long long upper = (1LL << 56); //upper bound DES keys 2^56
  long long mylower, myupper;
  MPI_Status st;
  MPI_Request req;

  // Agregado para medir tiempos, la llave se puede pasar al correr (mpirun -np 4 ./programa 123456)
  if(argc > 1)
    the_key = atoll(argv[1]);

  int ciphlen = strlen(eltexto);
  MPI_Comm comm = MPI_COMM_WORLD;

  char cipher[ciphlen+1];
  memcpy(cipher, eltexto, ciphlen);
  cipher[ciphlen]=0;
  encrypt(the_key, cipher);

  MPI_Init(NULL, NULL);
  MPI_Comm_size(comm, &N);
  MPI_Comm_rank(comm, &id);

  // Agregado para la documentacion, el proceso 0 muestra los primeros 48 bytes del texto cifrado
  if(id == 0){
    printf("Texto cifrado (primeros 48 bytes en hex)\n");
    for(int b=0; b<48 && b<ciphlen; b++)
      printf("%02x", (unsigned char)cipher[b]);
    printf("\n\n");
  }

  long long found = 0;
  int ready = 0;

  long long range_per_node = upper / N;
  mylower = range_per_node * id;
  myupper = range_per_node * (id+1) -1;
  if(id == N-1){
    myupper = upper;
  }
  printf("Process %d lower %lld upper %lld\n", id, mylower, myupper);

  // Agregado para medir tiempos, todos los procesos empiezan a buscar al mismo tiempo
  MPI_Barrier(comm);
  double t0 = MPI_Wtime();

  MPI_Irecv(&found, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);

  for(long long i = mylower; i<myupper; ++i){
    MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
    if(ready)
      break;  

    if(tryKey(i, cipher, ciphlen)){
      found = i;
      printf("Process %d found the key\n", id);
      for(int node=0; node<N; node++){
        MPI_Send(&found, 1, MPI_LONG_LONG, node, 0, comm);
      }
      break;
    }
  }

  if(id==0){
    MPI_Wait(&req, &st);
    printf("Tiempo de busqueda = %.6f s\n", MPI_Wtime() - t0);
    decrypt(found, cipher, ciphlen);
    printf("Key = %lld\n\n", found);
    printf("%s\n", cipher);
  }

  MPI_Finalize();
}
