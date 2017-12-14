#ifndef LIBRSAQRGEN_H__
#define LIBRSAQRGEN_H__


#ifdef __cplusplus
extern "C" {
#endif

void rsaQRInitCrypto(char params[256],char result[256]);
void rsaQREncryptCsvInvoice(char csv[256], char qrCont[512]);
void rsaQRGetError(char error[256]);

#ifdef __cplusplus
}
#endif

#endif