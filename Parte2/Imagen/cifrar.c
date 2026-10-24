/* cifrar: cifra un texto con DES igual que bruteforce_fix (texto completo,
 * armado corregido) y saca el resultado en base64, que es el formato en que los
 * equipos publican su texto para el descifrado cruzado.
 *
 * Como se cifra todo el texto, la palabra objetivo puede estar en cualquier
 * parte del texto.
 *
 * Uso
 *   ./cifrar texto.txt 18014398509481984 > nuestro_cifrado.b64
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <rpc/des_crypt.h>

static void make_key(long key, char *k8){
  unsigned long k = 0, kk = (unsigned long)key;
  for(int i=0; i<8; ++i){
    kk <<= 1;
    k += (kk & (0xFEUL << i*8));   // armado corregido, igual que bruteforce_fix
  }
  memcpy(k8, &k, 8);
  des_setparity(k8);
}

static void b64_print(const unsigned char *in, int n){
  static const char *T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int i;
  for(i = 0; i + 2 < n; i += 3){
    putchar(T[in[i]>>2]);
    putchar(T[((in[i]&3)<<4) | (in[i+1]>>4)]);
    putchar(T[((in[i+1]&15)<<2) | (in[i+2]>>6)]);
    putchar(T[in[i+2]&63]);
  }
  if(n - i == 1){
    putchar(T[in[i]>>2]); putchar(T[(in[i]&3)<<4]); putchar('='); putchar('=');
  } else if(n - i == 2){
    putchar(T[in[i]>>2]); putchar(T[((in[i]&3)<<4)|(in[i+1]>>4)]);
    putchar(T[(in[i+1]&15)<<2]); putchar('=');
  }
  putchar('\n');
}

int main(int argc, char *argv[]){
  if(argc != 3){
    fprintf(stderr, "Uso: %s texto.txt llave > cifrado.b64\n", argv[0]);
    return 1;
  }
  long key = atol(argv[2]);

  FILE *f = fopen(argv[1], "rb");
  if(!f){ fprintf(stderr, "No se pudo abrir %s\n", argv[1]); return 1; }
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  if(n < 8){ fprintf(stderr, "El texto debe tener al menos 8 bytes\n"); fclose(f); return 1; }
  char *buf = malloc(n + 1);
  if(fread(buf, 1, n, f) != (size_t)n){ fprintf(stderr, "Error al leer\n"); fclose(f); free(buf); return 1; }
  fclose(f);

  char k8[8];
  make_key(key, k8);
  ecb_crypt(k8, buf, n - (n % 8), DES_ENCRYPT);  // cifra todos los bloques completos

  b64_print((unsigned char*)buf, (int)n);
  fprintf(stderr, "Cifrado %ld bytes (%ld bloques completos) con la llave %ld\n", n, n/8, key);
  free(buf);
  return 0;
}
