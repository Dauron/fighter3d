#include "lib3dx/xImport.h"
#include "../Utils/Utils.h"
#include "../OpenGL/Textures/TextureMgr.h"

#include "Model3dx.h"

bool Model3dx :: Load ( const char *name )
{
    assert (model == NULL);

    if (m_Name != name) m_Name = strdup(name); // set name if not reloading

    int size = strlen(name);
    if (!strcasecmp(name + size - 4, ".3dx"))
        model = xFileLoad(name);
    else
    {
        if (!strcasecmp(name + size - 4, ".3dm"))
            model = xLoadFrom3dmFile (name);
        else
        {
            Lib3dsFile *file3ds = lib3ds_file_load (name);
            if (file3ds)
            {
                model = xImportFileFrom3ds(file3ds);
                lib3ds_file_free(file3ds);
            }
        }
        // save
        char *fname = strdup (name);
        fname[size-1] = 'x';
        model->fileName = fname;
        xFileSave(model);
    }

    return model;
}

void Model3dx :: Unload( void )
{
    FreeRenderData();
    if (model)
    {
        if (model->texturesInited)
            for (xMaterial *mat = model->materialP; mat; mat = mat->nextP)
                if (mat->texture.name && mat->texture.htex)
                {
                    HTexture htex;
                    htex.SetHandle(mat->texture.htex);
                    g_TextureMgr.DeleteTexture(htex);
                    mat->texture.htex = 0;
                }
        model->texturesInited = false;
    }

    if (m_Name) { delete[] m_Name; m_Name = NULL; }
}

void Model3dx :: InvalidateElementRenderData(xElement *elem)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        InvalidateElementRenderData(selem);
    elem->renderData.vertexB = 0;
    elem->renderData.normalB = 0;
    elem->renderData.indexB = 0;
    elem->renderData.mode = xRENDERMODE_NULL;
}

void Model3dx :: FreeElementRenderData(xElement *elem, bool listOnly)
{
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
        FreeElementRenderData(selem, listOnly);

    if (listOnly && elem->renderData.mode != xRENDERMODE_LIST) return;

    if (elem->renderData.mode == xRENDERMODE_VBO)
    {
        GLuint p = elem->renderData.vertexB;
        if (p) glDeleteBuffersARB(1, &p);
        p = elem->renderData.normalB;
        if (p) glDeleteBuffersARB(1, &p);
        p = elem->renderData.indexB;
    }
    else
    if (elem->renderData.mode == xRENDERMODE_LIST && elem->renderData.listID)
        glDeleteLists(elem->renderData.listID, 1);
    elem->renderData.vertexB = 0;
    elem->renderData.normalB = 0;
    elem->renderData.indexB = 0;
    elem->renderData.mode = xRENDERMODE_NULL;
}