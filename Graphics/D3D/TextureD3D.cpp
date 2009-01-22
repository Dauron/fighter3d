#ifdef USE_D3D

#include "Texture.h"
#include "../../Utils/Utils.h" // string case compare
#include <D3dx9tex.h>

using namespace Graphics::D3D;

bool Texture :: Create()
{
    assert (ID_DXTexture == 0);
    assert (Name.size());
/*
    Image *image = NULL;
    if (this->image.data == NULL)
    {
        const char *fname = Name.c_str();
        if (strcasecmp(fname + Name.size() - 4, ".bmp"))
            image = LoadTGA( fname );
        else
            image = LoadBMP( fname );
        if (!image) return false;
    }
    else
        image = &this->image;

    Width  = image->sizeX;
    Height = image->sizeY;

    int  components = 3;
    D3DFORMAT format = 0;
    switch (image->format)
    {
        case Image::FT_RGB8:
            format     = D3DFMT_R8G8B8;
            components = 3;
            break;
        case Image::FT_RGBA8:
            format     = D3DFMT_A8R8G8B8;
            components = 4;
            break;
    }
    int size = components * image->sizeX * image->sizeY;
*/
    if (D3D_OK !=
        D3DXCreateTextureFromFile(Graphics::D3D::d3ddev, Name.c_str(), &ID_DXTexture)
/*
        D3DXCreateTextureFromFileInMemoryEx(
            Graphics::D3D::d3ddev,
            image->data,
            size,
            image->sizeX,
            image->sizeY,
            D3DX_DEFAULT, // generate mipmaps
            0,
            format,
            D3DPOOL_MANAGED,
            D3DX_FILTER_LINEAR,
            D3DX_FILTER_LINEAR,
            0,
            NULL,
            NULL,
            &ID_DXTexture)
*/
    )
    {
        /*if (image != &this->image)
        {
            image->FreeData();
            delete image;
        }*/
        return false;
    }
/*
    // Generate 1 texture ID
    glGenTextures(1, &ID_GLTexture);
    // Set texture as current
    glBindTexture(GL_TEXTURE_2D, ID_GLTexture);

    // Modulation mode
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    // Scaling filters (during the drawing)
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);                // MAG_FILTER = magnified filter : when the texture is enlarged
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); // MIN_FILTER = minimized filter : when the texture is shrinked
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Wraping mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, image->repeatX ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, image->repeatY ? GL_REPEAT : GL_CLAMP);

    GLint  components = 3, format = 0;
    GLenum type = 0, colorOrder = 0;
    switch (image->type)
    {
        case Image::TP_UNSIGNED_BYTE:
            type = GL_UNSIGNED_BYTE; break;
    }
    switch (image->format)
    {
        case Image::FT_RGB8:
            format     = GL_RGB8;
            components = 3;
            break;
        case Image::FT_RGBA8:
            format = GL_RGBA8;
            components = 4;
            break;
    }
    switch (image->colorOrder)
    {
        case Image::CO_RGB:
            colorOrder = GL_RGB; break;
        case Image::CO_RGBA:
            colorOrder = GL_RGBA; break;
        case Image::CO_BGR:
            colorOrder = GL_BGR_EXT; break;
        case Image::CO_BGRA:
            colorOrder = GL_BGRA_EXT; break;
    }

    // Generates the texture
    if(FL_MipMap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, //target
                components,              //number of combonents 1 - 4
                image->sizeX,            //image size
                image->sizeY,
                colorOrder,              //format (color order)
                type,
                image->data);            //picture represented as a byte array
    }
    else
    {
        // Set the properties of the texture : size, color type...
        glTexImage2D(GL_TEXTURE_2D,      //target
                0,                       //level : usually left to zero
                format,                  //internal format
                image->sizeX,            //image size
                image->sizeY,
                0,                       //0 : no border
                colorOrder,              //format (color order)
                type,
                image->data);            //picture represented as a byte array
    }*/
    /*
    if (image != &this->image)
    {
        image->FreeData();
        delete image;
    }
    */
    return true;
}

void Texture :: Dispose( void )
{
    if (ID_DXTexture != 0)
    {
        ID_DXTexture->Release();
        ID_DXTexture = 0;
    }
    image.FreeData();
}

#endif
