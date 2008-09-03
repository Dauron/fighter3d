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
    virtual const std::string &GetId() { return Name; }

    bool ReLoad()
    {
        assert(Name.size());
        std::string l_name = Name;
        Unload();
        return Load(l_name.c_str());
    }

    void Invalidate()    { I_bones = 0; }
    bool IsValid() const { return I_bones; }
};

class xAnimationMgr : public Singleton<xAnimationMgr>, public Manager<xAnimationH, HAnimation>
{
    typedef std::pair <NameIndex::iterator, bool> NameIndexInsertRc;

public:

// Lifetime.
    xAnimationMgr ( void ) {  /* ... */  }
   ~xAnimationMgr ( void ) {  /* ... */  }

// Animation management.
    HAnimation GetAnimation    ( const char* name );

// Animation query.
    const std::string &GetName( HAnimation hani ) const
        {  return ( m_HandleMgr.DereferenceNoValidation( hani )->Name );  }
    xAnimation        *GetAnimation( HAnimation hani )
        {  return ( m_HandleMgr.Dereference( hani ) );  }
};

#endif
