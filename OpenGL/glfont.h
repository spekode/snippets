// ***************************************************************************
// glfont.h: OpenGL Texture-mapped Font header
// Author: Jeff Roberts <jeffroberts1@gmail.com>
// ***************************************************************************
#ifndef GLFONT_H_
#define GLFONT_H_

// ***************************************************************************
// Include section


// ***************************************************************************
// Definitions section
typedef void GLFont;

// ***************************************************************************
// Global Variable section


// ***************************************************************************
// Prototypes section
GLFont *glFontLoad(char *fontfile);
void glFontDestroy(GLFont **font);
void glFontRotate(float angle, float x, float y, float z);
void glFontScale(float x, float y, float z);
void glFontSelect(GLFont *font);
void glPrint(float x, float y, float z, char *text);
void glPrintChar(float x, float y, float z, char c);
void glFontReset(void);

// ***************************************************************************
// Extern section


#endif
