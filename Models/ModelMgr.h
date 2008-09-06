#ifndef __incl_ModelMgr_h
#define __incl_ModelMgr_h

#include "../Utils/Singleton.h"
#include "../Utils/Manager.h"
#include "Model3dx.h"

#define g_ModelMgr ModelMgr::GetSingleton()

class ModelMgr;
typedef Handle<ModelMgr> HModel;

class ModelMgr : public Singleton<ModelMgr>, public Manager<Model3dx, HModel>
{
public:
// Model management.
    HModel GetModel   ( const char* name );

// Model query.
    const std::string &GetName( HModel hmod ) const
        {  return ( m_HandleMgr.Dereference( hmod )->Name );  }
    Model3dx          *GetModel( HModel hmod )
        {  return ( m_HandleMgr.Dereference( hmod ) );  }
};

#endif
