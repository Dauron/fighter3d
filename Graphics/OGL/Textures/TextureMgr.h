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
    typedef std::pair <NameIndex::iterator, bool> NameIndexInsertRc;

public:

// Lifetime.
    TextureMgr( void ) {  /* ... */  }
   ~TextureMgr( void ) {  /* ... */  }

// Texture management.
    HTexture GetTexture   ( const char* name );
    void     DeleteTexture( HTexture htex );

// Texture query.
    const std::string& GetName( HTexture htex ) const
        {  return ( m_HandleMgr.Dereference( htex )->m_Name );  }
    int GetWidth( HTexture htex ) const
        {  return ( m_HandleMgr.Dereference( htex )->m_Width );  }
    int GetHeight( HTexture htex ) const
        {  return ( m_HandleMgr.Dereference( htex )->m_Height );  }
    GLuint GetTexture( HTexture htex ) const
        {  return ( m_HandleMgr.Dereference( htex )->m_GLTexture );  }
    void BindTexture( HTexture htex ) const
        { glBindTexture(GL_TEXTURE_2D, m_HandleMgr.Dereference( htex )->m_GLTexture ); }
};

#endif
