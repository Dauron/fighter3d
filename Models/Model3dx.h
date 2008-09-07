#ifndef __incl_Model3dx_h
#define __incl_Model3dx_h

#include "../Utils/Resource.h"
#include "lib3dx/xModel.h"

struct Model3dx : public Resource
{
public:
    std::string Name;  // for reconstruction
    xModel     *model;

    Model3dx() { Clear(); }

    virtual void Clear() {
        Resource::Clear();
        Name.clear();
        model = 0;
    }

    virtual bool Create();
    virtual bool Create( const std::string& name )
    {
        Name = name;
        return Create();
    }

    virtual void Dispose();
    virtual void Invalidate()
    { /*model->FL_textures_loaded = 0;*/ }
    virtual bool IsDisposed()
    { return !model; }

    virtual const std::string &Identifier() { return Name; }
};

#endif
