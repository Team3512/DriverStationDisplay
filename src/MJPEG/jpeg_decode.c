#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

struct jpeg_decompress_struct *
jpeg_readheader(char *inbuf, int inbufsize)
{
	struct jpeg_error_mgr *jerr;
	struct jpeg_decompress_struct *cinfo;

	cinfo = malloc(sizeof(struct jpeg_decompress_struct));
	jerr = malloc(sizeof(struct jpeg_error_mgr));

	/* set up libjpeg-turbo error handling */
	cinfo->err = jpeg_std_error(jerr);

	/* set up the cinfo struct */
	jpeg_create_decompress(cinfo);

	/* load the jpeg from a memory buffer */
	jpeg_mem_src(cinfo, (unsigned char *)inbuf, (unsigned long)inbufsize);
	/* read header */
	jpeg_read_header(cinfo, TRUE);

	return cinfo;
}

int
jpeg_decode(struct jpeg_decompress_struct *cinfo, char *outbuf, int outbufsize)
{
	JSAMPARRAY buffer;
	int row_stride;
	int outpos;
	int i;

	/* decompress jpeg */
	jpeg_start_decompress(cinfo);

	/* read in raw image data */
	row_stride = cinfo->output_width * cinfo->output_components;
	buffer = (*cinfo->mem->alloc_sarray)
		((j_common_ptr) cinfo, JPOOL_IMAGE, row_stride, 1);

	i = 0;
	while(cinfo->output_scanline < cinfo->output_height){
		jpeg_read_scanlines(cinfo, buffer, 1);
		outpos = (row_stride*i);
		if(outpos+row_stride > outbufsize) break;
		memcpy(outbuf+outpos, buffer[0], row_stride);
		i++;
	}

	return 0;
}
