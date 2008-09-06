#ifndef __incl_Manager_h
#define __incl_Manager_h

#include <map>
#include "Utils.h"
#include "HandleMgr.h"

template <class RESOURCE = Resource, class HANDLE = Handle<Resource> >
class Manager
{
protected:
// Handle manager data type.
    typedef HandleMgr <RESOURCE, HANDLE> HHandleMgr;

// Index by name into db.
    typedef std::map <std::string, HANDLE, istring_less > NameIndex;
    typedef std::pair <typename NameIndex::iterator, bool> NameIndexInsertRc;

// Private data.
    HHandleMgr m_HandleMgr;
    NameIndex  m_NameIndex;

    void Lock( HANDLE hnd )
    {
        RESOURCE *dat = m_HandleMgr.DereferenceNoValidation( hnd );
        if (dat) dat->Lock();
    }

public:

    void InvalidateItems();

    bool IsHandleValid( HANDLE hnd )
    {
        return m_HandleMgr.DereferenceNoValidation( hnd );
    }

    void Release( HANDLE hnd );

    ~Manager(void);
};

template <typename RESOURCE, class HANDLE>
void Manager <RESOURCE, HANDLE>
:: InvalidateItems()
{
    typename NameIndex::iterator i, begin = m_NameIndex.begin(), end = m_NameIndex.end();
    for ( i = begin ; i != end ; ++i )
        m_HandleMgr.DereferenceNoValidation( i->second )->Invalidate();
}

template <typename RESOURCE, class HANDLE>
void Manager <RESOURCE, HANDLE>
:: Release( HANDLE hnd )
{
    RESOURCE* dst = m_HandleMgr.DereferenceNoValidation( hnd );
    if ( dst )
    {
        dst->Release();
        if (!dst->IsLocked())
        {
            // delete from index
            m_NameIndex.erase( m_NameIndex.find( dst->Identifier() ) );
            // delete from db
            ((Resource*)dst)->Destroy();
            m_HandleMgr.Release( hnd );
        }
    }
}

template <typename RESOURCE, class HANDLE>
Manager <RESOURCE, HANDLE>
:: ~Manager( void )
{
    // release all our remaining items before we go
    typename NameIndex::iterator
        NI_curr = m_NameIndex.begin(),
        NI_last = m_NameIndex.end();
    for ( ; NI_curr != NI_last ; ++NI_curr )
        ((Resource*)m_HandleMgr.DereferenceNoValidation( NI_curr->second ))->Destroy();
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
