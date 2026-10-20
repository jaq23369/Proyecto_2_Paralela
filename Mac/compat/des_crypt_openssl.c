/* Implementacion de ecb_crypt y des_setparity con OpenSSL para macOS */
#define OPENSSL_SUPPRESS_DEPRECATED
#include <openssl/des.h>

int ecb_crypt(char *key, char *buf, unsigned len, unsigned mode)
{
  DES_key_schedule ks;
  unsigned i;

  if (len % 8 != 0)
    return 3; /* DESERR_BADPARAM */
  DES_set_key_unchecked((const_DES_cblock *)key, &ks);
  for (i = 0; i < len; i += 8)
    DES_ecb_encrypt((const_DES_cblock *)(buf + i), (DES_cblock *)(buf + i), &ks,
                    (mode & 1) ? 0 : 1); /* bit 0 en 1 = descifrar */
  return 0; /* DESERR_NONE */
}

void des_setparity(char *key)
{
  DES_set_odd_parity((DES_cblock *)key);
}
