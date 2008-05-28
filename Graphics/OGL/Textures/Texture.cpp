#include "Texture.h"
#include "ImageFile.h"
#include "../ogl.h"
#include "../../../Utils/Utils.h"
#include "../../../Utils/Debug.h"

bool Texture :: Load( const std::string& name, bool mipmap )
{
    assert (m_GLTexture == 0);

    ImageRec *image;
    const char *fname = name.c_str();
    if (strcasecmp(fname + name.size() - 4, ".bmp"))
        image = LoadTGA( fname );
    else
        image = LoadBMP( fname );
    if (!image) return false;

    m_Name   = name;
    m_MipMap = mipmap;
    m_Width  = image->sizeX;
    m_Height = image->sizeY;

    // Generate 1 texture ID
    glGenTextures(1, &m_GLTexture);
    // Set texture as current
    glBindTexture(GL_TEXTURE_2D, m_GLTexture);

    // Modulation mode
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    // Scaling filters (during the drawing)
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);                // MAG_FILTER = magnified filter : when the texture is enlarged
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST); // MIN_FILTER = minimized filter : when the texture is shrinked
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Wraping mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Generates the texture
    if(mipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, //target
                image->format,           //internal format : RGB 8 bits per color
                image->sizeX,            //image size
                image->sizeY,
                image->colororder,       //fomat : RGB
                image->type,
                image->data);            //picture represented as a byte array
    }
    else
    {
        // Set the properties of the texture : size, color type...
        glTexImage2D(GL_TEXTURE_2D,      //target
                0,                       //level : usually left to zero
                image->format,           //internal format : RGB 8 bits per color
                image->sizeX,            //image size
                image->sizeY,
                0,                       //0 : no border
                image->colororder,       //format : RGB
                image->type,
                image->data);            //picture represented as a byte array
    }
    delete[] image->data;
    delete image;

    return true;
}

void Texture :: Unload( void )
{
    if (m_GLTexture != 0)
    {
        m_Name.erase();
        glDeleteTextures(1, &m_GLTexture);
        m_GLTexture = 0;
    }
}
