/* Copyright (C) Scott Bilas, 2000.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Scott Bilas, 2000"
 */
#ifndef __incl_Singleton_h
#define __incl_Singleton_h

#include "../Config.h"

#ifdef WIN32
#pragma warning(disable : 4311) // pointer trunc
#pragma warning(disable : 4312) // pointer trunc
#endif

#include <cassert>

template <typename T> class Singleton
{
    static T* ms_Singleton;

protected:
    Singleton( void )
    {
        assert( !ms_Singleton );
        int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
        ms_Singleton = (T*)((int)this + offset);
    }
   ~Singleton( void )
        {  assert( ms_Singleton );  ms_Singleton = 0;  }

public:
    static T& GetSingleton( void )
        {  assert( ms_Singleton );  return ( *ms_Singleton );  }
    static T* GetSingletonPtr( void )
        {  return ( ms_Singleton );  }

    static void CreateS()
       { new T(); }
    static void DestroyS()
       { assert( ms_Singleton ); delete ms_Singleton; }
};

template <typename T> T* Singleton <T>::ms_Singleton = 0;


// Sample usage:
//
/*
class TextureMgr : public Singleton <TextureMgr>
{
public:
    Texture* GetTexture( const char* name );
    // ...
};

#define g_TextureMgr TextureMgr::GetSingleton()

void SomeFunction( void )
{
    Texture* stone1 = TextureMgr::GetSingleton().GetTexture( "stone1" );
    Texture* wood6 = g_TextureMgr.GetTexture( "wood6" );
    // ...
}
*/

#endif
