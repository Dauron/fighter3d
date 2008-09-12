#include "FontMgr.h"

HFont FontMgr :: GetFont( const char* name, int size )
{
    std::string key( name );
    key.append("|", 1);
    key += itos(size);

    // insert/find
    NameIndexInsertRc rc =
        m_NameIndex.insert( std::make_pair( key, HFont() ) );
    if ( rc.second )
    {
        // this is a new insertion
        Graphics::OGL::Font* fnt = m_HandleMgr.Acquire( rc.first->second );
        if ( !fnt->Create( name, size ) )
        {
            m_HandleMgr.Release( rc.first->second );
            m_NameIndex.erase( rc.first );
            //IncReferences( rc.first->second );
            //DeleteFont( rc.first->second );
            //assert (false);
            return HFont();
        }
    }
    Lock(rc.first->second);

    if (IsHandleValid(rc.first->second))
        return ( rc.first->second );
    else
    {
        assert (false);
        return HFont();
    }
}
