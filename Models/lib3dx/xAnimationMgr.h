#ifndef __incl_lib3dx_AnimationMgr_h
#define __incl_lib3dx_AnimationMgr_h

#include "../../Utils/Singleton.h"
#include "../../Utils/Manager.h"
#include "../../Utils/Resource.h"
#include "xAnimation.h"

#define g_AnimationMgr xAnimationMgr::GetSingleton()

class xAnimationMgr;
typedef Handle<xAnimationMgr> HAnimation;

struct xAnimationH : public xAnimation, public Resource
{
private:
    std::string Id;

public:
    xAnimationH() { Clear(); }

    virtual void Clear() {
        Resource::Clear();
        xAnimation::Clear();
        Id.clear();
    }

    virtual bool Create()
    {
        assert( Name.size() );
        return Load( Name.c_str() );
    }
    virtual bool Create( const std::string& name )
    {
        Id = Name = name;
        return Create();
    }

    virtual void Dispose()
    {
        xAnimation::Destroy();
    }
    virtual bool IsDisposed()
    { return !I_bones; }

    virtual const std::string &Identifier() { return Id; }
};

class xAnimationMgr : public Singleton<xAnimationMgr>, public Manager<xAnimationH, HAnimation>
{
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
