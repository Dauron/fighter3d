#ifndef __incl_TextureMgr_h
#define __incl_TextureMgr_h

#include "../../Utils/Singleton.h"
#include "../../Utils/Manager.h"
#include "../OGL/Texture.h"

#define g_TextureMgr TextureMgr::GetSingleton()

class TextureMgr;
typedef Handle <TextureMgr> HTexture;

class TextureMgr : public Singleton<TextureMgr>, public Manager<Graphics::OGL::Texture, HTexture>
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
    void BindTexture( HTexture htex )
        { m_HandleMgr.Dereference( htex )->Bind(); }
};

#endif
