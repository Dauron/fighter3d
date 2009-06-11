#include "Texture.h"
#include "../../Utils/Utils.h" // string case compare

using namespace Graphics::OGL;

bool Texture :: Create()
{
    assert (ID_GLTexture == 0);
    //assert (Name.size());

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
    }

    if (image != &this->image)
    {
        image->FreeData();
        delete image;
    }
    return true;
}

void Texture :: Dispose( void )
{
    if (ID_GLTexture != 0)
    {
        glDeleteTextures(1, &ID_GLTexture);
        ID_GLTexture = 0;
    }
    image.FreeData();
}
