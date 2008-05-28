#ifndef __incl_Texture_h
#define __incl_Texture_h

#include <string>
#include "../../../Utils/HandleDst.h"

struct Texture : public HandleDst
{
    std::string  m_Name;        // for reconstruction
    bool         m_MipMap;      // generate mip maps?
    unsigned int m_Width;       // width
    unsigned int m_Height;      // height
    unsigned int m_GLTexture;   // GL Texture identifier

    Texture()
    {
        Invalidate();
    }

    bool Load  ( const std::string& name, bool mipmap );
    void Unload( void );

    bool ReLoad  ()
    {
        std::string name = m_Name;
        Unload();
        return Load (name, m_MipMap);
    }

    void Invalidate()
    {
        m_GLTexture = 0;
    }
    bool IsValid() const
    {
        return m_GLTexture != 0; // && glIsTexture( m_GLTexture ); // glIsTexture doesn't work to good :/
    }
};

#endif
