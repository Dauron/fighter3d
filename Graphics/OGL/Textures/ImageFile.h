/*
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=24
 */

#ifndef __incl_ImageFile_h
#define __incl_ImageFile_h

struct ImageRec
{
    unsigned int sizeX, sizeY;
    unsigned int bpp;
    unsigned int format;
    unsigned int colororder;
    unsigned int type;
    unsigned char *data;
};

ImageRec *LoadTGA(const char *filename);
ImageRec *LoadBMP(const char *filename);

#endif

