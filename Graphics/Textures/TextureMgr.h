#ifndef __incl_TextureMgr_h
#define __incl_TextureMgr_h

#include "../../Utils/Singleton.h"
#include "../../Utils/Manager.h"
#include "TexResource.h"

#define g_TextureMgr TextureMgr::GetSingleton()

class TextureMgr;
typedef Handle <TextureMgr> HTexture;

class TextureMgr : public Singleton<TextureMgr>, public Manager<Graphics::TexResource, HTexture>
{
public:
// Texture management.
    HTexture GetTexture   ( const char* name );
    HTexture SetTexture   ( const char* name, Image image );

// Texture query.
    const std::string& GetName( HTexture htex ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( htex )->texture->Name );  }
    int GetWidth( HTexture htex ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( htex )->texture->Width );  }
    int GetHeight( HTexture htex ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( htex )->texture->Height );  }
    void BindTexture( HTexture htex )
        { m_HandleMgr.Dereference( htex )->Bind(); }

    Graphics::TexResource &GetResource( HTexture htex )
        { return *m_HandleMgr.Dereference( htex ); }
};

#endif
