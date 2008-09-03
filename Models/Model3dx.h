#ifndef __incl_Model3dx_h
#define __incl_Model3dx_h

#include "../Utils/HandleDst.h"
#include "lib3dx/xModel.h"

struct Model3dx : public HandleDst
{
public:
    std::string m_Name;  // for reconstruction
    xModel     *model;

    Model3dx() : model(NULL) {}

    virtual const std::string &GetId() { return m_Name; }

    bool Load ( const char *name );
    void Unload( void );
    bool ReLoad()
    {
        assert(m_Name.size());
        std::string name = m_Name;
        Unload();
        return Load(name.c_str());
    }

    void Invalidate()    { model->FL_textures_loaded = false; }
    bool IsValid() const { return model; }
};

#endif
