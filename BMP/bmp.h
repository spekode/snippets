// ***************************************************************************
// bmp.h: BMP graphics file functions header
// Author: Jeff Roberts <jeffroberts1@gmail.com>
// ***************************************************************************
#ifndef BMP_H_
#define BMP_H_

// ***************************************************************************
// Include section
#include <stdint.h>

// ***************************************************************************
// Definitions section
typedef void BMPFile;

// ***************************************************************************
// Global Variable section


// ***************************************************************************
// Prototypes section
BMPFile *BMP_Create(uint16_t width, uint16_t height);
void BMP_Destroy(BMPFile **bmpfile);
int BMP_GetHeight(BMPFile *bmpfile);
void BMP_GetPixel(BMPFile *bmpfile, uint32_t x, uint32_t y, uint8_t *rgb);
uint8_t *BMP_GetPixelArray(BMPFile *bmpfile);
int BMP_GetWidth(BMPFile *bmpfile);
BMPFile *BMP_Load(char *filename);
void BMP_SetPixel(BMPFile *bmpfile, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);
int BMP_Write(BMPFile *bmpfile, char *filename);

// ***************************************************************************
// Extern section


#endif
