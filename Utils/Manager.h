#ifndef __incl_Manager_h
#define __incl_Manager_h

#include <map>
#include "Utils.h"
#include "HandleMgr.h"

template <typename HANDLE_DST, class HANDLE>
class Manager
{
protected:
// Handle manager data type.
    typedef HandleMgr <HANDLE_DST, HANDLE> HHandleMgr;

// Index by name into db.
    typedef std::map <std::string, HANDLE, istring_less > NameIndex;

// Private data.
    HHandleMgr m_HandleMgr;
    NameIndex  m_NameIndex;

    void IncReferences( HANDLE hnd )
    {
        HANDLE_DST *dat = m_HandleMgr.DereferenceNoValidation( hnd );
        if (dat) dat->IncReferences();
    }
    void DecReferences( HANDLE hnd )
    {
        HANDLE_DST *dat = m_HandleMgr.DereferenceNoValidation( hnd );
        if (dat) dat->DecReferences();
    }

public:

    void InvalidateItems();

    bool IsHandleValid( HANDLE hnd )
    {
        return m_HandleMgr.DereferenceNoValidation( hnd );
    }

    void DeleteReference( HANDLE hnd );

    ~Manager(void);
};

template <typename HANDLE_DST, class HANDLE>
void Manager <HANDLE_DST, HANDLE>
:: InvalidateItems()
{
    typename NameIndex::iterator i, begin = m_NameIndex.begin(), end = m_NameIndex.end();
    for ( i = begin ; i != end ; ++i )
        m_HandleMgr.DereferenceNoValidation( i->second )->Invalidate();
}

template <typename HANDLE_DST, class HANDLE>
void Manager <HANDLE_DST, HANDLE>
:: DeleteReference( HANDLE hnd )
{
    HANDLE_DST* dst = m_HandleMgr.DereferenceNoValidation( hnd );
    if ( dst != 0 )
    {
        dst->DecReferences();
        if (!dst->m_References)
        {
            // delete from index
            m_NameIndex.erase( m_NameIndex.find( dst->GetId() ) );
            // delete from db
            dst->Unload();
            m_HandleMgr.Release( hnd );
        }
    }
}

template <typename HANDLE_DST, class HANDLE>
Manager <HANDLE_DST, HANDLE>
:: ~Manager( void )
{
    // release all our remaining items before we go
    typename NameIndex::iterator i, begin = m_NameIndex.begin(), end = m_NameIndex.end();
    for ( i = begin ; i != end ; ++i )
        m_HandleMgr.DereferenceNoValidation( i->second )->Unload();
}

// Sample usage:
//

/*
struct Texture : public HandleDst
{
  // ...
}

class TextureMgr;
typedef Handle <TextureMgr> HTexture;

class TextureMgr : public Manager<Texture, HTexture>
{
  public:
    HTexture* GetTexture( const char* name );
    void BindTexture( HTexture htex );
    // ...
};

*/

#endif
