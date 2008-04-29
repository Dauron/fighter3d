#include "FontMgr.h"

HFont FontMgr :: GetFont( const char* name, const char size )
{
    std::string key( name );
    key.append("|", 1);
    key.append(&size, 1);

    // insert/find
    NameIndexInsertRc rc =
        m_NameIndex.insert( std::make_pair( key, HFont() ) );
    if ( rc.second )
    {
        // this is a new insertion
        GLFont* fnt = m_HandleMgr.Acquire( rc.first->second );
        if ( !fnt->Load( name, size ) )
        {
            IncReferences( rc.first->second );
            DeleteFont( rc.first->second );
            assert (false);
            return HFont();
        }
    }
    IncReferences(rc.first->second);

    if (IsHandleValid(rc.first->second))
        return ( rc.first->second );
    else
    {
        assert (false);
        return HFont();
    }
}

void FontMgr :: DeleteFont( HFont hfnt )
{
    GLFont* fnt = m_HandleMgr.DereferenceNoValidation( hfnt );
    if ( fnt != 0 )
    {
        fnt->DecReferences();
        if (!fnt->m_References)
        {
            std::string key( fnt->m_Name );
            key.append("|", 1);
            key.append(&(fnt->m_Size), 1);
            // delete from index
            m_NameIndex.erase( m_NameIndex.find( key ) );
            // delete from db
            fnt->Unload();
            m_HandleMgr.Release( hfnt );
        }
    }
}
