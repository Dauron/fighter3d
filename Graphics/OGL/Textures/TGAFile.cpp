#include "ImageFile.h"
#include <cstdio>
#include <cstring>
#include "../ogl.h"
#include "../../../Utils/Debug.h"
#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

inline void FreeData(FILE *file, ImageRec *texture)
{
    if(texture->data != NULL)       // Was Image Data Loaded
        delete[] texture->data;     // If So, Release The Image Data
    delete texture;
    if (file)
        fclose(file);               // Close The File
}

// Loads A TGA File Into Memory
ImageRec *LoadTGA(const char *filename)
{
    unsigned char  TGAheader[12] = {1,0,2,0,0,0,0,0,0,0,0,0};  // Uncompressed TGA Header
    unsigned char  TGAcompare[12];                             // Used To Compare TGA Header
    unsigned char  header[6];                                  // First 6 Useful Bytes From The Header
    ImageRec *texture = new ImageRec();
    texture->data = NULL;
    texture->colororder = GL_BGR_EXT; // GL_RGB;
    texture->type = GL_UNSIGNED_BYTE;

    FILE *file = fopen(filename, "rb");                        // Open The TGA File
    if( file == NULL ||                                        // Does File Even Exist?
        fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||  // Are There 12 Bytes To Read?
        memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0               ||  // Does The Header Match What We Want?
        fread(header,1,sizeof(header),file)!=sizeof(header))   // If So Read Next 6 Header Bytes
    {
        FreeData(file, texture);
        return NULL;                                           // Return False
    }

    texture->sizeX = (unsigned int)(header[1]) << 8 + header[0]; // Determine The TGA Width  (highbyte*256+lowbyte)
    texture->sizeY = (unsigned int)(header[3]) << 8 + header[2]; // Determine The TGA Height (highbyte*256+lowbyte)

     if( texture->sizeX == 0 ||                                // Is The Width Less Than Or Equal To Zero
         texture->sizeY == 0 ||                                // Is The Height Less Than Or Equal To Zero
         (header[4]!=24 && header[4]!=32))                     // Is The TGA 24 or 32 Bit?
    {
        FreeData(file, texture);
        return NULL;                                           // Return False
    }

    texture->bpp = header[4];                                  // Grab The TGA's Bits Per Pixel (24 or 32)
    texture->format = (header[4] == 24) ? GL_RGB8 : GL_RGBA8;  // Set The GL Mode To RGB (24 BPP) or RBGA (32 BPP)
    unsigned int Bpp = texture->bpp/8;                         // Holds Number Of Bytes Per Pixel Used In The TGA File
    unsigned int imageSize = texture->sizeX * texture->sizeY * Bpp;  // Calculate The Memory Required For The TGA Data
    texture->data = new unsigned char[imageSize];              // Reserve Memory To Hold The TGA Data

    if( texture->data == NULL ||                               // Does The Storage Memory Exist?
        fread(texture->data, 1, imageSize, file)!=imageSize)   // Does The Image Size Match The Memory Reserved?
    {
        FreeData(file, texture);
        return NULL;
    }

//    // BGR -> RGB
//    unsigned int temp;                                         // Temporary Variable
//    for(unsigned int i=0; i < imageSize; i += Bpp)             // Loop Through The Image Data
//    {                                                          // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
//        temp=texture->data[i];                                 // Temporarily Store The Value At Image Data 'i'
//        texture->data[i] = texture->data[i + 2];               // Set The 1st Byte To The Value Of The 3rd Byte
//        texture->data[i + 2] = temp;                           // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
//    }

    fclose (file);                                             // Close The File
    return texture;
}

