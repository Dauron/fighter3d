#ifndef __incl_TextureMgr_h
#define __incl_TextureMgr_h

#include "../../../Utils/Singleton.h"
#include "../../../Utils/Manager.h"
#include "../GLShader.h"
#include "Texture.h"

#define g_TextureMgr TextureMgr::GetSingleton()

class TextureMgr;
typedef Handle <TextureMgr> HTexture;

class TextureMgr : public Singleton<TextureMgr>, public Manager<Texture, HTexture>
{
public:
// Texture management.
    HTexture GetTexture   ( const char* name );

// Texture query.
    const std::string& GetName( HTexture htex ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( htex )->Name );  }
    int GetWidth( HTexture htex ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( htex )->Width );  }
    int GetHeight( HTexture htex ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( htex )->Height );  }
    GLuint GetTexture( HTexture htex )
        {  return ( m_HandleMgr.Dereference( htex )->ID_GLTexture );  }
    void BindTexture( HTexture htex )
        { glBindTexture(GL_TEXTURE_2D, m_HandleMgr.Dereference( htex )->ID_GLTexture ); }
};

#endif
