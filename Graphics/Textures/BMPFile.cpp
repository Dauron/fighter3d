#include "ImageFile.h"
#include <cstdio>
#include <cstring>

#ifndef _WINGDI_

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L

typedef struct tagRGBTRIPLE {
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved; // (Alpha)
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;         // Signature ID 'BM'
    DWORD   bfSize;         // Bytes Per Pixel in Bitmapdata (?)
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;      // Offset to Bitmap data
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    DWORD      biSize;          // Size of the Info Header Structure
    LONG       biWidth;         // Width of the Image
    LONG       biHeight;        // Height of the Image
    WORD       biPlanes;        // Number of Planes
    WORD       biBitCount;      // Bits Per Pixel
    DWORD      biCompression;   // Compression Type RGB/RLE
    DWORD      biSizeImage;     // Image Size
    LONG       biXPelsPerMeter; // ???
    LONG       biYPelsPerMeter; // ?WTF?
    DWORD      biClrUsed;       // Number of Used Colors In Palette (Size of Palette)
    DWORD      biClrImportant;  // ??
} BITMAPINFOHEADER;

#endif

inline void FreeData(FILE *file, Image *texture)
{
    texture->FreeData();
    delete texture;
    if (file)
        fclose(file);               // Close The File
}

// Loads A BMP File Into Memory
Image *LoadBMP(const char *filename)
{
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    //ZeroMemory( &bfh, sizeof(BITMAPFILEHEADER) );
    //ZeroMemory( &bih, sizeof(BITMAPINFOHEADER) );

    Image *texture      = new Image();
    texture->colorOrder = Image::CO_BGR;
    texture->type       = Image::TP_UNSIGNED_BYTE;

    FILE *file = fopen(filename, "rb");                        // Open The BMP File
    if( file == NULL ||                                        // Does File Even Exist?
        fread(&bfh,1,sizeof(BITMAPFILEHEADER),file)!=sizeof(BITMAPFILEHEADER) || // Are There 12 Bytes To Read?
        fseek(file,14,SEEK_SET)!=0 ||
        fread(&bih,1,sizeof(BITMAPINFOHEADER),file)!=sizeof(BITMAPINFOHEADER) || // If So Read Next 6 Header Bytes
        fseek(file,54,SEEK_SET)!=0 ||
        (bih.biPlanes != 1) || (bih.biCompression != BI_RGB) || (bih.biBitCount < 8) ||
        (bfh.bfType != 0x4d42)  )
    {
        FreeData(file, texture);
        return NULL;
    }

    if (bih.biHeight < 0) bih.biHeight = -bih.biHeight;
    texture->sizeX  = (unsigned int)bih.biWidth;                 // Determine The BMP Width
    texture->sizeY  = (unsigned int)bih.biHeight;                // Determine The BMP Height
    texture->bpp    = bih.biBitCount == 8 ? 24 : bih.biBitCount; // Grab The BMP's Bits Per Pixel (24 or 32)
    texture->format = (texture->bpp == 24) ? Image::FT_RGB8 : Image::FT_RGBA8;
    unsigned int BppIn  = bih.biBitCount >> 3;
    unsigned int BppOut = texture->bpp   >> 3;                         // Holds Number Of Bytes Per Pixel Used In The TGA File
    unsigned int imageSize = texture->sizeX * texture->sizeY * BppOut; // Calculate The Memory Required For The TGA Data
    texture->data = new unsigned char[imageSize];                      // Reserve Memory To Hold The BMP Data

    if( texture->sizeX == 0 || texture->sizeY == 0 || texture->data == NULL )
    {
        FreeData(file, texture);
        return NULL;
    }

    // 256 colors bitmap palete
    if ( bih.biClrUsed == 0 && bih.biBitCount == 8)
        bih.biClrUsed = 256;
    RGBQUAD pal[256];
    if ( bih.biClrUsed>0 ) {
        int fp = 54;
        if (fread( &pal, sizeof(RGBQUAD), bih.biClrUsed, file ) != bih.biClrUsed)
        {
            FreeData(file, texture);
            return NULL;
        }
        fp += bih.biClrUsed*sizeof(RGBQUAD);
        fseek( file, fp, 0 );
    }

    unsigned int dataPerLine  = texture->sizeX * BppIn;
    unsigned int bytesPerLine = ((bih.biWidth * bih.biBitCount + 31) >> 5) << 2;
    unsigned int spamPerLine  = bytesPerLine - dataPerLine;
    unsigned char *dataptr    = texture->data;
    unsigned char *dataend    = texture->data + imageSize;

    if (bih.biClrUsed == 0) // true color
    {
        while (dataptr != dataend)
        {
            if (fread(dataptr, 1, dataPerLine, file) != dataPerLine ||
                fseek(file, spamPerLine, SEEK_CUR) ) // Does The Image Size Match The Memory Reserved?
            {
                FreeData(file, texture);
                return NULL;
            }
            /* // BGR -> RGB
            unsigned char tmp;
            for(unsigned int j=0; j < bih.biWidth; ++j)
            {
                tmp = *dataptr;
                *dataptr = *(dataptr+2);
                *(dataptr+2) = tmp;
                dataptr += BppOut;
            }
            */
            dataptr += dataPerLine;
        }
    }
    else // 256 colors
    {
        unsigned char* indices = new unsigned char[dataPerLine];
        int nLine = 0;
        while (dataptr != dataend)
        {
            if (fread(indices, 1, dataPerLine, file) != dataPerLine ||
                fseek(file, spamPerLine, SEEK_CUR) ) // Does The Image Size Match The Memory Reserved?
            {
                delete[] indices;
                FreeData(file, texture);
                return NULL;
            }
            unsigned char *index = indices;
            for(LONG j=0; j < bih.biWidth; ++j)
            {
                RGBQUAD *color = pal + *(index++);
                // BGR - Swap Blue with Red for RGB
                *(dataptr++) = color->rgbBlue;
                *(dataptr++) = color->rgbGreen;
                *(dataptr++) = color->rgbRed;
            }
            ++nLine;
        }
        delete[] indices;
    }

    fclose (file);                      // Close The File
    return texture;                     // Texture Building Went Ok, Return True
}

