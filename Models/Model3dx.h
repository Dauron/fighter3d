#ifndef __incl_Model3dx_h
#define __incl_Model3dx_h

#include "lib3dx/xRenderGL.h"
#include "../Utils/HandleDst.h"

struct Model3dx : public HandleDst
{
public:
    char      *m_Name;  // for reconstruction
    xRenderGL  renderer;

    Model3dx() : m_Name(NULL) {}

    bool Load ( const char *name );
    void Unload( void )
    {
        renderer.Finalize();
        if (m_Name) { delete[] m_Name; m_Name = NULL; }
    }
    bool ReLoad()
    {
        assert(m_Name);
        char *name = strdup(m_Name);
        Unload();
        bool res = Load(name);
        delete[] name;
        return res;
    }

    void Invalidate()  { renderer.Invalidate(); }
    bool IsValid()     { return renderer.xModel; }
};

#endif
