/* bruteforce para el cluster de la Parte 2.
 *
 * Parte de bruteforce_fix.c de la Parte 1, que cifra y descifra todo el texto y
 * corrige el armado de la llave. Es la versión que usa el equipo, porque todos
 * modificaron el original para cifrar el texto completo y buscar una frase en el
 * texto descifrado por entero.
 *
 * Agregados de la Parte 2:
 *   - cada proceso imprime el nombre de su nodo (MPI_Get_processor_name), para
 *     ver en consola que participaron varias máquinas.
 *   - lee el texto cifrado de otro equipo desde un archivo en base64 (--b64) o
 *     binario (--file), y busca la palabra objetivo con --word.
 *   - por defecto arma la llave con el armado corregido. Con --keyorig usa el
 *     armado del código original, por si un equipo no lo cambió.
 *
 * Uso
 *   mpirun -np 4 ./bruteforce                                 prueba con texto propio
 *   mpirun -np 4 ./bruteforce --b64 grupo9.b64 --word "paralela y distribuida"
 *   mpirun -np 4 ./bruteforce --file cifrado.bin --word "palabra objetivo"
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <rpc/des_crypt.h>

#ifndef KEY_BITS
#define KEY_BITS 56
#endif
#ifndef THE_KEY
#define THE_KEY 18014398509481984L
#endif

int usar_orig = 0;   // 0 = armado corregido (por defecto), 1 = armado original

/* Armado corregido: 7 bits del numero por byte, usando unsigned long para que
 * los corrimientos de mas de 32 bits esten bien definidos. */
static void make_key_fix(long key, char *k8){
  unsigned long k = 0, kk = (unsigned long)key;
  for(int i=0; i<8; ++i){
    kk <<= 1;
    k += (kk & (0xFEUL << i*8));
  }
  memcpy(k8, &k, 8);
  des_setparity(k8);
}

/* Armado original, tal cual el codigo base (0xFE << i*8 con int). Se deja para
 * poder romper el texto de un equipo que no haya corregido este detalle. */
static void make_key_orig(long key, char *k8){
  long k = 0;
  for(int i=0; i<8; ++i){
    key <<= 1;
    k += (key & (0xFE << i*8));
  }
  memcpy(k8, &k, 8);
  des_setparity(k8);
}

static void make_key(long key, char *k8){
  if(usar_orig) make_key_orig(key, k8);
  else          make_key_fix(key, k8);
}

void decrypt(long key, char *ciph, int len){
  char k[8];
  make_key(key, k);
  ecb_crypt(k, ciph, len - (len % 8), DES_DECRYPT);
}

void encrypt(long key, char *ciph, int len){
  char k[8];
  make_key(key, k);
  ecb_crypt(k, ciph, len - (len % 8), DES_ENCRYPT);
}


char search[256] = "Open MPI es una";
int tryKey(long key, char *ciph, int len){
  char temp[len+1];
  memcpy(temp, ciph, len);
  temp[len]=0;
  decrypt(key, temp, len);
  return strstr((char *)temp, search) != NULL;
}

char eltexto[] =
  "Open MPI es una implementación libre y de código abierto del estándar MPI, que son las siglas de Message Passing Interface. MPI define un conjunto de funciones para que varios procesos trabajen juntos en un mismo problema mandándose mensajes entre ellos. Cada proceso tiene su propia memoria y no puede ver los datos de los demás, entonces la única forma de compartir información es enviarla y recibirla de forma explícita.\n"
  "El proyecto empezó en 2004, cuando se juntaron los equipos de varias implementaciones que ya existían, como LAM/MPI, LA-MPI y FT-MPI. La idea era unir lo mejor de cada una en un solo programa y que lo mantuviera una comunidad de universidades, laboratorios y empresas. Hoy Open MPI se usa en muchas supercomputadoras, pero también se puede instalar en una laptop para aprender.";
long the_key = THE_KEY;


/* Decodifica base64 en memoria. Devuelve el largo o -1 si falla. */
static int b64_decode(const char *in, int inlen, unsigned char *out){
  static const char *T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int tab[256];
  for(int i=0;i<256;i++) tab[i] = -1;
  for(int i=0;i<64;i++) tab[(unsigned char)T[i]] = i;
  int val=0, bits=0, n=0;
  for(int i=0;i<inlen;i++){
    int c = (unsigned char)in[i];
    if(c=='=') break;
    if(tab[c] < 0) continue;   // salta espacios y saltos de linea
    val = (val<<6) | tab[c];
    bits += 6;
    if(bits >= 8){ bits -= 8; out[n++] = (val >> bits) & 0xFF; }
  }
  return n;
}

/* Lee un archivo. Si es_b64, lo decodifica. Devuelve el largo o -1. */
static int leer_texto(const char *ruta, int es_b64, char **datos){
  FILE *f = fopen(ruta, "rb");
  if(!f) return -1;
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buf = malloc(n + 1);
  if(fread(buf, 1, n, f) != (size_t)n){ fclose(f); free(buf); return -1; }
  fclose(f);
  buf[n] = 0;
  if(es_b64){
    unsigned char *dec = malloc(n);
    int m = b64_decode(buf, (int)n, dec);
    free(buf);
    *datos = (char*)dec;
    return m;
  }
  *datos = buf;
  return (int)n;
}


int main(int argc, char *argv[]){
  int N, id;
  long upper = (1L << KEY_BITS);
  long mylower, myupper;
  MPI_Request req;

  char *archivo = NULL;
  int es_b64 = 0;
  for(int a = 1; a < argc; a++){
    if(strcmp(argv[a], "--b64") == 0 && a+1 < argc){ archivo = argv[++a]; es_b64 = 1; }
    else if(strcmp(argv[a], "--file") == 0 && a+1 < argc){ archivo = argv[++a]; es_b64 = 0; }
    else if(strcmp(argv[a], "--word") == 0 && a+1 < argc){
      strncpy(search, argv[++a], sizeof(search)-1);
      search[sizeof(search)-1] = 0;
    }
    else if(strcmp(argv[a], "--keyorig") == 0) usar_orig = 1;
    else the_key = atol(argv[a]);
  }

  MPI_Init(NULL, NULL);
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &N);
  MPI_Comm_rank(comm, &id);

  char nodo[MPI_MAX_PROCESSOR_NAME];
  int nodolen;
  MPI_Get_processor_name(nodo, &nodolen);

  /* El texto cifrado. Con un archivo se usa el del otro equipo. Sin archivo se
   * cifra el texto propio con the_key, para probar. */
  char *cipher;
  int ciphlen;
  if(archivo){
    ciphlen = leer_texto(archivo, es_b64, &cipher);
    if(ciphlen < 0){
      if(id == 0) printf("No se pudo leer el archivo %s\n", archivo);
      MPI_Finalize();
      return 1;
    }
  } else {
    int textlen = strlen(eltexto);
    ciphlen = ((textlen + 7) / 8) * 8;
    cipher = malloc(ciphlen + 1);
    memset(cipher, 0, ciphlen + 1);
    memcpy(cipher, eltexto, textlen);
    encrypt(the_key, cipher, ciphlen);
  }

  if(id == 0){
    printf("Buscando \"%s\" en %d bytes cifrados, armado %s\n",
           search, ciphlen, usar_orig ? "original" : "corregido");
    printf("Primeros 48 bytes en hex: ");
    for(int b=0; b<48 && b<ciphlen; b++) printf("%02x", (unsigned char)cipher[b]);
    printf("\n\n");
  }

  long found = -1;
  long received = -1;
  int ready = 0;

  long range_per_node = upper / N;
  mylower = range_per_node * id;
  myupper = range_per_node * (id+1);
  if(id == N-1)
    myupper = upper;
  printf("Process %d en el nodo %s, rango lower %ld upper %ld\n", id, nodo, mylower, myupper - 1);

  MPI_Barrier(comm);
  double t0 = MPI_Wtime();

  MPI_Irecv(&received, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);

  for(long i = mylower; i<myupper; ++i){
    MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
    if(ready)
      break;

    if(tryKey(i, cipher, ciphlen)){
      found = i;
      printf("Process %d en el nodo %s encontro la llave\n", id, nodo);
      for(int node=0; node<N; node++)
        MPI_Send(&found, 1, MPI_LONG, node, 0, comm);
      break;
    }
  }
  if(ready)
    found = received;

  long key_found;
  MPI_Allreduce(&found, &key_found, 1, MPI_LONG, MPI_MAX, comm);
  double t1 = MPI_Wtime();

  if(key_found >= 0)
    MPI_Wait(&req, MPI_STATUS_IGNORE);
  else{
    MPI_Cancel(&req);
    MPI_Wait(&req, MPI_STATUS_IGNORE);
  }

  if(id==0){
    printf("Tiempo de busqueda = %.6f s\n", t1 - t0);
    if(key_found >= 0){
      decrypt(key_found, cipher, ciphlen);
      printf("Key = %li\n\n", key_found);
      printf("%s\n", cipher);
    }else
      printf("No se encontro la llave entre 0 y %ld\n", upper - 1);
  }

  MPI_Finalize();
  return 0;
}
