// ***************************************************************************
// glfont.c: OpenGL Texture-Mapped Font functions
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

// OpenGL
#include <GL/gl.h>

// BMP graphics files
#include "bmp.h"

// ***************************************************************************
// Definitions section
typedef struct {
	GLuint textureID;
	GLuint base;
} GLFont;

// ***************************************************************************
// Global Variable section
static GLFont *cFont = NULL;
static float rotateAngle = 0.0;
static float rotatex = 0.0;
static float rotatey = 0.0;
static float rotatez = 0.0;
static float scalex = 1.0;
static float scaley = 1.0;
static float scalez = 1.0;

// ***************************************************************************
// Prototypes section
static GLuint makeFontList(GLuint textureID);
static GLuint makeTexture(int width, int height, uint8_t *data);
static void _glPrint(float x, float y, float z, char *text, int textlen);

// ***************************************************************************
// Extern section

static GLuint makeFontList(GLuint textureID) {
	GLuint base;
	int count;
	float x, y;

	base = glGenLists(256);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	for (count = 0; count < 256; count++) {
		x = ( float )( count % 16 ) / 16.0f;
		y = 1 - ( float )( count / 16 ) / 16.0f;
		glNewList(base + (count), GL_COMPILE);
			glBegin( GL_QUADS );
				/* Bottom left */
				glTexCoord2f(x, y - 0.0625f);
				glVertex3f(-10, -10, 0);

				/* Bottom right */
				glTexCoord2f(x + 0.0625f, y - 0.0625f);
				glVertex3f(10, -10, 0);

				/* Top right */
				glTexCoord2f(x + 0.0625f, y);
				glVertex3f(10, 10, 0);

				/* Top left */
				glTexCoord2f(x, y);
				glVertex3f(-10, 10, 0);
			glEnd( );

			glTranslated( 12, 0, 0 );
		glEndList( );
	}

	glPopMatrix();

	return base;
}

static GLuint makeTexture(int width, int height, uint8_t *data) {
	GLuint id = 0;

	/* Create The Texture */
	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Generate The Texture */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width,
		height, 0, GL_BGRA,
		GL_UNSIGNED_BYTE, data);

	return id;
}

GLFont *glFontLoad(char *fontfile) {
	GLFont *font = NULL;
	BMPFile *bmp = NULL;

	bmp = BMP_Load(fontfile);
	if (!bmp) return NULL;

	font = calloc(1, sizeof(GLFont));
	if (!font) {
		BMP_Destroy(&bmp);
		return NULL;
	}

	font->textureID = makeTexture(BMP_GetWidth(bmp), BMP_GetHeight(bmp), BMP_GetPixelArray(bmp));
	BMP_Destroy(&bmp);

	font->base = makeFontList(font->textureID);

	return font;
}

void glFontDestroy(GLFont **font) {
	GLFont *fontp = *font;

	if (!fontp)	return;

	if (cFont == fontp) cFont = NULL;

	glDeleteLists(fontp->base, 256);
	glDeleteTextures(1, &fontp->textureID);
	free(fontp);
	*font = NULL;
}

void glFontReset(void) {
	rotateAngle = 0.0;
	rotatex = 0.0;
	rotatey = 0.0;
	rotatez = 0.0;
	scalex = 1.0;
	scaley = 1.0;
	scalez = 1.0;
}

void glFontRotate(float angle, float x, float y, float z) {
	rotateAngle = angle;
	rotatex = x;
	rotatey = y;
	rotatez = z;
}

void glFontScale(float x, float y, float z) {
	scalex = x;
	scaley = y;
	scalez = z;
}

void glFontSelect(GLFont *font) {
	cFont = font;
}

static void _glPrint(float x, float y, float z, char *text, int textlen) {
	if (!cFont) return;

	glBindTexture(GL_TEXTURE_2D, cFont->textureID);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef(x, y, z);
	glScalef(scalex, scaley, scalez);
	glRotatef(rotateAngle, rotatex, rotatey, rotatez);

	glListBase(cFont->base - 32);
	glCallLists(textlen, GL_BYTE, text);

	glPopMatrix();
}

void glPrint(float x, float y, float z, char *text) {
	_glPrint(x, y, z, text, strlen(text));
	//printf("%d\n", strlen(text));
}

void glPrintChar(float x, float y, float z, char c) {
	_glPrint(x, y, z, &c, 1);
}
