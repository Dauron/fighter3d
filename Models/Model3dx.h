#ifndef __incl_Model3dx_h
#define __incl_Model3dx_h

#include "../Utils/HandleDst.h"
#include "lib3dx/xModel.h"

struct Model3dx : public HandleDst
{
public:
    char      *m_Name;  // for reconstruction
    xModel     *model;

    Model3dx() : m_Name(NULL), model(NULL) {}

    bool Load ( const char *name );
    void Unload( void );
    bool ReLoad()
    {
        assert(m_Name);
        char *name = strdup(m_Name);
        Unload();
        bool res = Load(name);
        delete[] name;
        return res;
    }

    void Invalidate()  { model->texturesInited = false; }
    bool IsValid()     { return model; }
};

#endif
