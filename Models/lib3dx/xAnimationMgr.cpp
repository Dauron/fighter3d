#include "xAnimationMgr.h"

HAnimation xAnimationMgr :: GetAnimation    ( const char* name )
{
    // insert/find
    NameIndexInsertRc rc =
        m_NameIndex.insert( std::make_pair( name, HAnimation() ) );
    if ( rc.second )
    {
        // this is a new insertion
        xAnimationH* ani = m_HandleMgr.Acquire( rc.first->second );
        if ( !ani->Load( rc.first->first.data() ) )
        {
            m_HandleMgr.Release( rc.first->second );
            m_NameIndex.erase( rc.first );
            //IncReferences( rc.first->second );
            //DeleteAnimation( rc.first->second );
            assert (false);
            return HAnimation();
        }
    }
    IncReferences(rc.first->second);

    if (IsHandleValid(rc.first->second))
        return ( rc.first->second );
    else
    {
        assert (false);
        return HAnimation();
    }
}

void xAnimationMgr :: DeleteAnimation( HAnimation hani )
{
    xAnimationH* ani = m_HandleMgr.Dereference( hani );
    if ( ani != 0 )
    {
        ani->DecReferences();
        if (!ani->m_References)
        {
            // delete from index
            m_NameIndex.erase( m_NameIndex.find( ani->name ) );
            // delete from db
            ani->Unload();
            m_HandleMgr.Release( hani );
        }
    }
}
