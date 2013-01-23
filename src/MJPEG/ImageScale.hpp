#ifndef _IMAGESCALE_HPP
#define _IMAGESCALE_HPP

#include <stdint.h>

int
imageScale(
        uint8_t *image_in,
        int width_in,
        int height_in,
        uint8_t *image_out,
        int width_out,
        int height_out,
        int bpp);

#endif /* _IMAGESCALE_HPP */
