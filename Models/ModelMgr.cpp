#include "ModelMgr.h"

HModel ModelMgr :: GetModel( const char* name )
{
    // insert/find
    NameIndexInsertRc rc =
        m_NameIndex.insert( std::make_pair( name, HModel() ) );
    if ( rc.second )
    {
        // this is a new insertion
        Model3dx* mod = m_HandleMgr.Acquire( rc.first->second );
        if ( !mod->Load( rc.first->first.data() ) )
        {
            IncReferences( rc.first->second );
            DeleteModel( rc.first->second );
            assert (false);
            return HModel();
        }
    }
    IncReferences(rc.first->second);

    if (IsHandleValid(rc.first->second))
        return ( rc.first->second );
    else
    {
        assert (false);
        return HModel();
    }
}

void ModelMgr :: DeleteModel( HModel hmod )
{
    Model3dx* mod = m_HandleMgr.DereferenceNoValidation( hmod );
    if ( mod != 0 )
    {
        mod->DecReferences();
        if (!mod->m_References)
        {
            // delete from index
            m_NameIndex.erase( m_NameIndex.find( mod->m_Name ) );
            // delete from db
            mod->Unload();
            m_HandleMgr.Release( hmod );
        }
    }
}
