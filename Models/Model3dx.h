#ifndef __incl_Model3dx_h
#define __incl_Model3dx_h

#include "../Utils/HandleDst.h"
#include "lib3dx/xModel.h"

struct Model3dx : public HandleDst
{
public:
    char      *m_Name;  // for reconstruction
    xFile     *model;

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

    void Invalidate()  {
        model->texturesInited = false;
        for (xElement *elem = model->firstP; elem; elem = elem->nextP)
            InvalidateElementRenderData(elem);
    }
    bool IsValid()     { return model; }

    void FreeRenderData(bool listOnly = false)
    {
        for (xElement *elem = model->firstP; elem; elem = elem->nextP)
            FreeElementRenderData(elem, listOnly);
    }

private:
    void InvalidateElementRenderData(xElement *elem);
    void FreeElementRenderData(xElement *elem, bool listOnly);
};

#endif
