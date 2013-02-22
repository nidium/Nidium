#ifndef __APE_BASE64_H
#define __APE_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

void base64_encode_b(unsigned char * src, char *dst, int len);
int base64_decode(unsigned char * out, const char *in, int out_length);
char *base64_encode(unsigned char * src, int len);

#ifdef __cplusplus
}
#endif

#endif

