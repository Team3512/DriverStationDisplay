#ifndef _JPEG_DECODE_H
#define _JPEG_DECODE_H

#define XMD_H
#include <jpeglib.h>

struct jpeg_decompress_struct *
jpeg_readheader(char *inbuf, int inbufsize);

int jpeg_decode(struct jpeg_decompress_struct *, char *outbuf, int outbufsize);

#endif
