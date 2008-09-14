#include "ImageFile.h"
#include <cstdio>
#include <cstring>

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

inline void FreeData(FILE *file, Image *texture)
{
    delete texture;
    if (file)
        fclose(file);               // Close The File
}

/*
BYTE    ITEM         SIZE    DESCRIPTION
****    ****         ****    ***********
 0      Offset         1     Usually 0,
                             add 18 to this value to find the start of the palette/image data.
 1      ColorType      1     Image type. 0 = RGB, 1 = Indexed.
 2      ImageType      1     0 = None, 1 = Indexed, 2 = RGB, 3 = Greyscale,
                            +8 = RLE encoded.
 3      PaletteStart   2     Start of palette.
 5      PaletteLen     2     Number of palette entries.
 7      PalBits        1     Bits per colour entry.
 8      X Origin       2     Image X Origin
10      Y Origin       2     Image Y Origin
12      Width          2     Image width (Pixels).
14      Height         2     Image height (Pixels)
16      BPP            1     Bits per pixel (8,16,24 or 32)
17      Orientation    1     If Bit 5 is set, the image will be upside down (like BMP)

+ optional 26 byte footer
*/

/*
 * Based on http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=24
 */
// Loads A TGA File Into Memory
Image *LoadTGA(const char *filename)
{
    unsigned char  TGAheader[12] = {1,0,2,0,0,0,0,0,0,0,0,0};  // Uncompressed TGA Header
    unsigned char  TGAcompare[12];                             // Used To Compare TGA Header
    unsigned char  header[6+256];                                  // First 6 Useful Bytes From The Header
    Image *texture = new Image();
    texture->type  = Image::TP_UNSIGNED_BYTE;

    FILE *file = fopen(filename, "rb");                         // Open The TGA File
    if( file == NULL ||                                         // Does File Even Exist?
        fread(TGAcompare,1,12,file)!=sizeof(TGAcompare) ||      // Are There 12 Bytes To Read?
        memcmp(TGAheader+1,TGAcompare+1,11) != 0        ||      // Does The Header Match What We Want?
        fread(header,1,6+TGAcompare[0],file)!=6+TGAcompare[0] ) // If So Read Next 6+offset Header Bytes
    {
        FreeData(file, texture);
        return NULL;                                           // Return False
    }

    texture->sizeX = (unsigned int)(header[1] << 8) + header[0]; // Determine The TGA Width  (highbyte*256+lowbyte)
    texture->sizeY = (unsigned int)(header[3] << 8) + header[2]; // Determine The TGA Height (highbyte*256+lowbyte)
    texture->bpp = header[4];                                    // Grab The TGA's Bits Per Pixel (24 or 32)

     if( texture->sizeX == 0 ||                                // Is The Width Less Than Or Equal To Zero
         texture->sizeY == 0 ||                                // Is The Height Less Than Or Equal To Zero
         (texture->bpp!=24 && texture->bpp!=32))               // Is The TGA 24 or 32 Bit?
    {
        FreeData(file, texture);
        return NULL;                                           // Return False
    }

    texture->format     = (texture->bpp == 24) ? Image::FT_RGB8 : Image::FT_RGBA8;
    texture->colorOrder = (texture->bpp == 24) ? Image::CO_BGR  : Image::CO_BGRA;
    unsigned int Bpp = texture->bpp >> 3;                      // Holds Number Of Bytes Per Pixel Used In The TGA File
    unsigned int imageSize = texture->sizeX * texture->sizeY * Bpp;  // Calculate The Memory Required For The TGA Data
    texture->data = new unsigned char[imageSize];              // Reserve Memory To Hold The TGA Data

    fseek(file, 0, SEEK_END);
    //long size = ftell(file);
    fseek(file, 18+TGAcompare[0], SEEK_SET);

    if( texture->data == NULL ||                               // Does The Storage Memory Exist?
        fread(texture->data, 1, imageSize, file)!=imageSize)   // Does The Image Size Match The Memory Reserved?
    {
        FreeData(file, texture);
        return NULL;
    }
/*
    // BGR -> RGB
    unsigned int temp;                                         // Temporary Variable
    for(unsigned int i=0; i < imageSize; i += Bpp)             // Loop Through The Image Data
    {                                                          // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
        temp=texture->data[i];                                 // Temporarily Store The Value At Image Data 'i'
        texture->data[i] = texture->data[i + 2];               // Set The 1st Byte To The Value Of The 3rd Byte
        texture->data[i + 2] = temp;                           // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
    }
*/
    fclose (file);                                             // Close The File

    return texture;
}

