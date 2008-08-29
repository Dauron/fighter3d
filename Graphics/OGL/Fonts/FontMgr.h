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
    typedef std::pair <NameIndex::iterator, bool> NameIndexInsertRc;

public:

// Lifetime.
    FontMgr( void ) {  /* ... */  }
   ~FontMgr( void ) {  /* ... */  }

// Texture management.
    HFont GetFont   ( const char* name, int size );
    void  DeleteFont( HFont hfnt );

// Texture query.
    const std::string& GetName( HFont hfnt ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( hfnt )->m_Name );  }
    int GetSize( HFont hfnt ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( hfnt )->m_Size );  }
    const GLFont* GetFont( HFont hfnt ) const
        {  return ( m_HandleMgr.Dereference( hfnt ) );  }
};

#endif
