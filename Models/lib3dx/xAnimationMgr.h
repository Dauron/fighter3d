#ifndef __incl_lib3dx_AnimationMgr_h
#define __incl_lib3dx_AnimationMgr_h

#include "../../Utils/Singleton.h"
#include "../../Utils/Manager.h"
#include "../../Utils/HandleDst.h"
#include "xAnimation.h"

#define g_AnimationMgr xAnimationMgr::GetSingleton()

class xAnimationMgr;
typedef Handle<xAnimationMgr> HAnimation;

struct xAnimationH : public xAnimation, public HandleDst
{
    bool ReLoad()
    {
        assert(name);
        char *l_name = strdup(name);
        Unload();
        bool res = Load(l_name);
        delete[] l_name;
        return res;
    }

    void Invalidate()  { boneC = 0; }
    bool IsValid()     { return boneC; }
};

class xAnimationMgr : public Singleton<xAnimationMgr>, public Manager<xAnimationH, HAnimation>
{
    typedef std::pair <NameIndex::iterator, bool> NameIndexInsertRc;

public:

// Lifetime.
    xAnimationMgr ( void ) {  /* ... */  }
   ~xAnimationMgr ( void ) {  /* ... */  }

// Model management.
    HAnimation GetAnimation    ( const char* name );
    void       DeleteAnimation ( HAnimation hani );

// Model query.
    const char* GetName( HAnimation hani ) const
        {  return ( m_HandleMgr.Dereference( hani )->name );  }
    xAnimation* GetAnimation( HAnimation hani )
        {  return ( m_HandleMgr.Dereference( hani ) );  }
};

#endif
