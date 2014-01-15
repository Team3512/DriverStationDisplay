//=============================================================================
//File Name: Util.cpp
//Description: Contains miscellaneous utility functions
//Author: FRC Team 3512, Spartatroniks
//=============================================================================

#include "Util.hpp"

int npot( int num ) {
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;

    return num;
}

void BMPtoPXL( HDC dc , HBITMAP bmp , int width , int height , BYTE* pxlData ) {
    BITMAPINFOHEADER bmi = {0};
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = width;
    bmi.biHeight = -height;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY; for PNG or JPEG

    GetDIBits( dc , bmp , 0 , height , pxlData , (BITMAPINFO*)&bmi , DIB_RGB_COLORS );
}
