//=============================================================================
//File Name: Util.hpp
//Description: Contains miscellaneous utility functions
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Bit-twiddling hack: Return the next power of two
int npot( int num );

// Converts HBITMAP of device context to pixel array
void BMPtoPXL( HDC dc , HBITMAP bmp , int width , int height , BYTE* pxlData );
