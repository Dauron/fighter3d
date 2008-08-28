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
        assert(Name);
        char *l_name = strdup(Name);
        Unload();
        bool res = Load(l_name);
        delete[] l_name;
        return res;
    }

    void Invalidate()  { I_bones = 0; }
    bool IsValid()     { return I_bones; }
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
        {  return ( m_HandleMgr.DereferenceNoValidation( hani )->Name );  }
    xAnimation* GetAnimation( HAnimation hani )
        {  return ( m_HandleMgr.Dereference( hani ) );  }
};

#endif
