// ***************************************************************************
// bmp.c: BMP graphics file functions
// Author: Jeff Roberts <jeffroberts1@gmail.com>
// ***************************************************************************


// ***************************************************************************
// Include section

// system
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


// ***************************************************************************
// Definitions section

// **** ALL BMP HEADER FIELDS ARE LITTLE ENDIAN ****

struct bmpfile_header {
	uint32_t filesz;
	uint16_t creator1;
	uint16_t creator2;
	uint32_t bmp_offset;
};

typedef struct {
	uint32_t header_sz;
	int32_t width;
	int32_t height;
	uint16_t nplanes;
	uint16_t bitspp;
	uint32_t compress_type;
	uint32_t bmp_bytesz;
	int32_t hres;
	int32_t vres;
	uint32_t ncolors;
	uint32_t nimpcolors;
} BITMAPINFOHEADER;

typedef struct {
	uint8_t magic[2]; // 'BM'
	struct bmpfile_header header;
	BITMAPINFOHEADER iheader;
	uint8_t *pixels;
} BMPFile;

#define BMP_HEADER_LENGTH (2 + sizeof(struct bmpfile_header) + sizeof(BITMAPINFOHEADER))

// ***************************************************************************
// Global Variable section


// ***************************************************************************
// Prototypes section
static uint32_t DWORDPad(uint32_t value);
static uint8_t *readFile(char *filename, uint32_t *filesize);
static uint32_t realRowSize(uint32_t width, uint32_t bpp);
static uint32_t sanityCheck(uint8_t *buf, uint32_t bufsize);

// ***************************************************************************
// Extern section

static uint32_t DWORDPad(uint32_t value) {
	uint32_t pad = 0;

	if (value & 0x03) {
		pad = 4 - (value % 4);
	}

	return pad;
}

static uint8_t *readFile(char *filename, uint32_t *filesize) {
	uint8_t *buf = NULL;
	FILE *fp = NULL;
	struct stat statbuf;

	if (stat(filename, &statbuf)) return NULL;
	if (statbuf.st_size == 0) return NULL;

	buf = calloc(statbuf.st_size, 1);
	if (!buf) return NULL;

	fp = fopen(filename, "r");
	if (!fp) {
		free(buf);
		return NULL;
	}

	if (fread(buf, 1, statbuf.st_size, fp) != statbuf.st_size) {
		free(buf);
		fclose(fp);
		return NULL;
	}

	fclose(fp);

	if (filesize) *filesize = statbuf.st_size;

	return buf;
}

// ***************************************************************************
// @fn          realRowSize
// @brief       determines the real size (bytes) of a row including padding
// @param       width width of a row
// @param		bpp bits per pixel
// @return      none
// ***************************************************************************
static uint32_t realRowSize(uint32_t width, uint32_t bpp) {
	int len, pad;

	bpp = bpp >> 3; // Convert to bytes
	len = bpp * width;
	pad = DWORDPad(len);

	return len + pad;
}

// ***************************************************************************
// @fn          sanityCheck
// @brief       validates a buffer containing a BMP
// @param       buf buffer containing BMP
// @param		bufsize length of buffer
// @return      0 if sane, otherwise -1
// ***************************************************************************
static uint32_t sanityCheck(uint8_t *buf, uint32_t bufsize) {
	uint8_t *p = buf;
	struct bmpfile_header *bheader = (struct bmpfile_header *)(buf + 2);
	BITMAPINFOHEADER *iheader = (BITMAPINFOHEADER *)(buf + 2 + sizeof(struct bmpfile_header));
	uint32_t rowLength = 0;

	// Is bufsize even large enough to have the right headers?
	if ( bufsize < BMP_HEADER_LENGTH )
		return -1;

	// Magic
	if (p[0] != 'B' || p[1] != 'M') return -1;
	// Filesize in header
	if (bheader->filesz != bufsize) return -1;
	// BMP data offset within range?
	if (bheader->bmp_offset >= bufsize) return -1;

	// We only load 24 bit images
	if (iheader->bitspp != 24) return -1;
	// Size of pixel array + offset to array within bounds?
	if (iheader->bmp_bytesz + bheader->bmp_offset > bufsize) return -1;
	// We don't do compression
	if (iheader->compress_type != 0) return -1;
	// We only handle one color plane
	if (iheader->nplanes != 1) return -1;
	// Width and height make sense?
	rowLength = realRowSize(iheader->width, 24);
	if (rowLength * iheader->height != iheader->bmp_bytesz) return -1;

	return 0;
}

// ***************************************************************************
// @fn          BMP_Create
// @brief       Allocate and initialize a new BMPFile
// @param       width width in pixels
// @param		height height in pixels
// @return      pointer to new BMPFile
// ***************************************************************************
BMPFile *BMP_Create(uint16_t width, uint16_t height) {
	BMPFile *bmp = NULL;
	int headerlen = 0;

	bmp = calloc(1, sizeof(BMPFile));
	if (!bmp) return bmp;

	// Initialize header fields to correct values
	bmp->magic[0] = 'B';
	bmp->magic[1] = 'M';

	bmp->iheader.bitspp = 24; // 24 bits per pixel
	bmp->iheader.bmp_bytesz = realRowSize(width, 24) * height; // Size of pixel array
	bmp->iheader.compress_type = 0; // No compression is applied
	bmp->iheader.header_sz = 40; // size of header, fixed value
	bmp->iheader.height = height;
	bmp->iheader.hres = 2600; // Horizonal resolution, just made it up
	bmp->iheader.ncolors = 0; // Number of colors in the palette
	bmp->iheader.nimpcolors = 0; // Number of 'important' colors, unused
	bmp->iheader.nplanes = 1; // Number of color planes, always 1
	bmp->iheader.vres = 2600; // Vertical resolution, just made it up
	bmp->iheader.width = width;

	headerlen = 2 + sizeof(struct bmpfile_header) + bmp->iheader.header_sz;
	bmp->header.bmp_offset = headerlen;
	bmp->header.creator1 = 0xFECA;
	bmp->header.creator2 = 0xBEBA;
	bmp->header.filesz = headerlen + bmp->iheader.bmp_bytesz;

	// Allocate space for the pixels
	bmp->pixels = calloc(bmp->iheader.bmp_bytesz, 1);
	if (!bmp->pixels) {
		free(bmp);
		bmp = NULL;
	}

	return bmp;
}

// ***************************************************************************
// @fn          BMP_Destroy
// @brief       Releases a BMPFile
// @param       bmpfile pointer to a pointer to a BMPFile to release
// @return      none
// ***************************************************************************
void BMP_Destroy(BMPFile **bmpfile) {
	BMPFile *bmp = *bmpfile;

	if (!bmp) return;

	if (bmp->pixels) free(bmp->pixels);
	free(bmp);
	*bmpfile = NULL;

	return;
}

// ***************************************************************************
// @fn          BMP_GetPixel
// @brief       Get a pixel at a position
// @param       bmpfile pointer to BMPFile
// @param		x x position
// @param		y y position
// @param		rgb array of 3 bytes to place color in
// @return      nothing, rgb values in rgb, [0]=r, [1]=g, [2]=b
// ***************************************************************************
void BMP_GetPixel(BMPFile *bmpfile, uint32_t x, uint32_t y, uint8_t *rgb) {
	uint32_t ry, rx;
	uint32_t rowLength;

	if (!bmpfile || (x >= bmpfile->iheader.width) || (y >= bmpfile->iheader.height)) return;
	if (!rgb) return;

	rowLength = bmpfile->iheader.width * (bmpfile->iheader.bitspp >> 3);
	ry = bmpfile->iheader.height - y - 1;
	rx = x * 3;

	rgb[2] = bmpfile->pixels[rx + (ry * rowLength)];
	rgb[1] = bmpfile->pixels[rx + (ry * rowLength) + 1];
	rgb[0] = bmpfile->pixels[rx + (ry * rowLength) + 2];
}

uint8_t *BMP_GetPixelArray(BMPFile *bmpfile) {
	if (!bmpfile) return NULL;
	return bmpfile->pixels;
}

int BMP_GetHeight(BMPFile *bmpfile) {
	if (!bmpfile) return 0;
	return bmpfile->iheader.height;
}

int BMP_GetWidth(BMPFile *bmpfile) {
	if (!bmpfile) return 0;
	return bmpfile->iheader.width;
}

// ***************************************************************************
// @fn          BMP_Load
// @brief       Load a .BMP from a file
// @param       filename name of file to open
// @return      new BMPFile * or NULL on error
// ***************************************************************************
BMPFile *BMP_Load(char *filename) {
	BMPFile *bmp = NULL;
	uint8_t *buf = NULL;
	uint32_t filesize = 0;
	uint32_t rowLength = 0;
	uint32_t unpaddedLength = 0;
	uint32_t row = 0;
	uint8_t *pixelbuf = NULL;

	buf = readFile(filename, &filesize);
	if (!buf) return NULL;

	if (sanityCheck(buf, filesize)) {
		fprintf(stderr, "BMP file didn't pass sanity check!\n");
		free(buf);
		return NULL;
	}

	bmp = calloc(1, sizeof(BMPFile));
	if (!bmp) {
		free(buf);
		return NULL;
	}

	bmp->magic[0] = 'B';
	bmp->magic[1] = 'M';

	memcpy(&bmp->header, buf + 2, sizeof(struct bmpfile_header));
	memcpy(&bmp->iheader, buf + 2 + sizeof(struct bmpfile_header), sizeof(BITMAPINFOHEADER));

	bmp->pixels = calloc(bmp->iheader.bmp_bytesz, 1);
	if (!bmp->pixels) {
		free(buf);
		free(bmp);
		return NULL;
	}

	// Copy pixel array to pixel buffer, skipping row padding if needed
	pixelbuf = buf + bmp->header.bmp_offset;
	rowLength = realRowSize(bmp->iheader.width, bmp->iheader.bitspp);
	unpaddedLength = bmp->iheader.width * (bmp->iheader.bitspp >> 3);

	for (row = 0; row < bmp->iheader.height; row++) {
		memcpy(bmp->pixels + (row * unpaddedLength), pixelbuf + (row * rowLength), unpaddedLength);
	}

	free(buf);

	return bmp;
}

// ***************************************************************************
// @fn          BMP_SetPixel
// @brief       Set a pixel at a position to a value
// @param       bmpfile pointer to BMPFile
// @param		x x position
// @param		y y position
// @param		value value to set
// @return      none
// ***************************************************************************
void BMP_SetPixel(BMPFile *bmpfile, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
	uint32_t ry, rx;
	uint32_t rowLength;

	if (!bmpfile || (x >= bmpfile->iheader.width) || (y >= bmpfile->iheader.height)) return;

	rowLength = bmpfile->iheader.width * (bmpfile->iheader.bitspp >> 3);
	ry = bmpfile->iheader.height - y - 1;
	rx = x * 3;

	bmpfile->pixels[rx + (ry * rowLength)] = b;
	bmpfile->pixels[rx + (ry * rowLength) + 1] = g;
	bmpfile->pixels[rx + (ry * rowLength) + 2] = r;
}

// ***************************************************************************
// @fn          BMP_Write
// @brief       Writes a BMPFile to disk
// @param       bmpfile BMPFile to write
// @param		filename Name of file to write to
// @return      0 on success otherwise -1
// ***************************************************************************
int BMP_Write(BMPFile *bmpfile, char *filename) {
	size_t count = 0;
	FILE *fp = NULL;
	int rc = -1;
	int row = 0;
	int rowLength;
	int unpaddedLength;

	fp = fopen(filename, "w");
	if (!fp) goto error;

	// Write it out.
	count = fwrite(bmpfile->magic, 2, 1, fp);
	if (!count) goto error;

	count = fwrite(&bmpfile->header, sizeof(struct bmpfile_header), 1, fp);
	if (!count) goto error;

	count = fwrite(&bmpfile->iheader, bmpfile->iheader.header_sz, 1, fp);
	if (!count) goto error;


	// Write the actual pixel array
	rowLength = realRowSize(bmpfile->iheader.width, bmpfile->iheader.bitspp);
	unpaddedLength = bmpfile->iheader.width * (bmpfile->iheader.bitspp >> 3);
	if ( rowLength != unpaddedLength ) {
		// Requires padding
		for (row = 0; row < bmpfile->iheader.height; row++) {
			count = fwrite(bmpfile->pixels + (unpaddedLength * row), 1, unpaddedLength, fp);
			if (count != unpaddedLength) goto error;
			// pad w/ junk
			count = fwrite(bmpfile->pixels, 1, rowLength - unpaddedLength, fp);
			if (count != (rowLength - unpaddedLength)) goto error;
		}
	}
	else {
		count = fwrite(bmpfile->pixels,
						1,
						bmpfile->iheader.bmp_bytesz,
						fp);
		if (count != bmpfile->iheader.bmp_bytesz) goto error;
	}


	rc = 0;
error:
	if (fp) fclose(fp);
	return rc;
}
