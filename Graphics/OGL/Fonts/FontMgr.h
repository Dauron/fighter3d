#ifndef __incl_FontMgr_h
#define __incl_FontMgr_h

#include "../../../Utils/Singleton.h"
#include "../../../Utils/Manager.h"
#include "GLFont.h"

#define g_FontMgr FontMgr::GetSingleton()

class FontMgr;
typedef Handle <FontMgr> HFont;

class FontMgr : public Singleton<FontMgr>, public Manager<GLFont, HFont>
{
public:
// Font management.
    HFont GetFont   ( const char* name, int size );

// Font query.
    const std::string& GetName( HFont hfnt ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( hfnt )->Name );  }
    int GetSize( HFont hfnt ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( hfnt )->Size );  }
    const GLFont* GetFont( HFont hfnt ) const
        {  return ( m_HandleMgr.Dereference( hfnt ) );  }
};

#endif
