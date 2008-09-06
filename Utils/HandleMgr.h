/* Copyright (C) Scott Bilas, 2000.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Scott Bilas, 2000"
 */
#ifndef __incl_HandleMgr_h
#define __incl_HandleMgr_h

#include <vector>
#include <cassert>
#include "Handle.h"
#include "Resource.h"

template <class RESOURCE = Resource, class HANDLE = Handle<Resource> >
class HandleMgr
{
private:

    // private types
    typedef std::vector <RESOURCE>     UserVec;
    typedef std::vector <unsigned int> MagicVec;
    typedef std::vector <unsigned int> FreeVec;

    // private data
    UserVec  m_UserData;     // data we're going to get to
    MagicVec m_MagicNumbers; // corresponding magic numbers
    FreeVec  m_FreeSlots;    // keeps track of free slots in the db

public:

// Lifetime.

    HandleMgr( void )  {  }
   ~HandleMgr( void )  {  }

// Handle methods.

    // acquisition
    RESOURCE* Acquire( HANDLE &handle );
    void      Release( HANDLE handle );

    // dereferencing
    RESOURCE*       Dereference( HANDLE handle );
    const RESOURCE* Dereference( HANDLE handle ) const;
    RESOURCE*       DereferenceNoValidation( HANDLE handle );
    const RESOURCE* DereferenceNoValidation( HANDLE handle ) const;

    bool            IsHandleValid( HANDLE handle ) const
    {
        return ( handle.GetIndex() < m_UserData.size() )
            && ( m_MagicNumbers[ handle.GetIndex() ] == handle.GetMagic() );
    }

    // other query
    unsigned int GetUsedHandleCount( void ) const
        {  return ( m_MagicNumbers.size() - m_FreeSlots.size() );  }
    bool HasUsedHandles( void ) const
        {  return ( !!GetUsedHandleCount() );  }
};

template <typename RESOURCE, typename HANDLE>
RESOURCE* HandleMgr <RESOURCE, HANDLE>
:: Acquire( HANDLE &handle )
{
    // if free list is empty, add a new one otherwise use first one found
    unsigned int index;
    if ( m_FreeSlots.empty() )
    {
        index = (unsigned int)m_MagicNumbers.size();
        handle.Init( index );
        m_UserData.push_back( RESOURCE() );
        m_MagicNumbers.push_back( handle.GetMagic() );
    }
    else
    {
        index = m_FreeSlots.back();
        handle.Init( index );
        m_FreeSlots.pop_back();
        m_MagicNumbers[ index ] = handle.GetMagic();
    }
    return &*( m_UserData.begin() + index );
}

template <typename RESOURCE, typename HANDLE>
void HandleMgr <RESOURCE, HANDLE>
:: Release( HANDLE handle )
{
    // which one?
    unsigned int index = handle.GetIndex();

    // make sure it's valid
    assert( index < m_UserData.size() );
    assert( m_MagicNumbers[ index ] == handle.GetMagic() );

    // ok remove it - tag as unused and add to free list
    m_MagicNumbers[ index ] = 0;
    m_FreeSlots.push_back( index );
}

template <typename RESOURCE, typename HANDLE>
inline RESOURCE* HandleMgr <RESOURCE, HANDLE>
:: DereferenceNoValidation( HANDLE handle )
{
    if ( handle.IsNull() )  return ( 0 );

    // check handle validity - $ this check can be removed for speed
    // if you can assume all handle references are always valid.
    unsigned int index = handle.GetIndex();
    if (   ( index >= m_UserData.size() )
        || ( m_MagicNumbers[ index ] != handle.GetMagic() ) )
    {
        // no good! invalid handle == client programming error
        assert( 0 );
        return ( 0 );
    }
    RESOURCE* res = &*( m_UserData.begin() + index );
    return res;
}


template <typename RESOURCE, typename HANDLE>
inline const RESOURCE* HandleMgr <RESOURCE, HANDLE>
:: DereferenceNoValidation( HANDLE handle ) const
{
    // this lazy cast is ok - non-const version does not modify anything
    typedef HandleMgr <RESOURCE, HANDLE> ThisType;
    return ( const_cast <ThisType*> ( this )->DereferenceNoValidation( handle ) );
}

template <typename RESOURCE, typename HANDLE>
inline RESOURCE* HandleMgr <RESOURCE, HANDLE>
:: Dereference( HANDLE handle )
{
    if ( handle.IsNull() )  return ( 0 );

    // check handle validity - $ this check can be removed for speed
    // if you can assume all handle references are always valid.
    unsigned int index = handle.GetIndex();
    if (   ( index >= m_UserData.size() )
        || ( m_MagicNumbers[ index ] != handle.GetMagic() ) )
    {
        // no good! invalid handle == client programming error
        assert( 0 );
        return ( 0 );
    }
    RESOURCE* res = &*( m_UserData.begin() + index );
    if (res->IsDisposed())
        res->Recreate();
    return res;
}

template <typename RESOURCE, typename HANDLE>
inline const RESOURCE* HandleMgr <RESOURCE, HANDLE>
:: Dereference( HANDLE handle ) const
{
    // this lazy cast is ok - non-const version does not modify anything
    typedef HandleMgr <RESOURCE, HANDLE> ThisType;
    return ( const_cast <ThisType*> ( this )->Dereference( handle ) );
}

#endif
