#include "TextureMgr.h"
#include "../../Utils/Filesystem.h"

HTexture TextureMgr :: GetTexture( const char* name )
{
    // insert/find
    NameIndexInsertRc rc =
        m_NameIndex.insert( std::make_pair( Filesystem::GetFullPath(name), HTexture() ) );
    if ( rc.second )
    {
        // this is a new insertion
        Texture* tex = m_HandleMgr.Acquire( rc.first->second );
        if ( !tex->Load( rc.first->first, true ) )
        {
            m_HandleMgr.Release( rc.first->second );
            m_NameIndex.erase( rc.first );
            //IncReferences( rc.first->second );
            //DeleteTexture( rc.first->second );
            //assert (false);
            return HTexture();
        }
    }
    IncReferences(rc.first->second);

    if (IsHandleValid(rc.first->second))
        return ( rc.first->second );
    else
    {
        assert (false);
        return HTexture();
    }
}

void TextureMgr :: DeleteTexture( HTexture htex )
{
    Texture* tex = m_HandleMgr.DereferenceNoValidation( htex );
    if ( tex != 0 )
    {
        tex->DecReferences();
        if (!tex->m_References)
        {
            // delete from index
            m_NameIndex.erase( m_NameIndex.find( tex->m_Name ) );
            // delete from db
            tex->Unload();
            m_HandleMgr.Release( htex );
        }
    }
}
