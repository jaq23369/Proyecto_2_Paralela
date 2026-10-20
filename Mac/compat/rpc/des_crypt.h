/* Reemplazo de <rpc/des_crypt.h> para macOS (la Mac no trae esta libreria).
   Mismas funciones y constantes que la version de Linux, implementadas con OpenSSL
   en des_crypt_openssl.c */
#ifndef DES_CRYPT_COMPAT_H
#define DES_CRYPT_COMPAT_H

#define DES_DIRMASK (1 << 0)
#define DES_ENCRYPT (0 * DES_DIRMASK)
#define DES_DECRYPT (1 * DES_DIRMASK)
#define DES_DEVMASK (1 << 1)
#define DES_HW      (0 * DES_DEVMASK)
#define DES_SW      (1 * DES_DEVMASK)

#define DESERR_NONE       0
#define DESERR_NOHWDEVICE 1
#define DESERR_HWERROR    2
#define DESERR_BADPARAM   3

int ecb_crypt(char *key, char *buf, unsigned len, unsigned mode);
void des_setparity(char *key);

#endif
