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
        model = xModel::Load(name);
    else
    {
        Lib3dsFile *file3ds = lib3ds_file_load (name);
        if (file3ds)
        {
            model = xImportFileFrom3ds(file3ds);
            lib3ds_file_free(file3ds);
        }
        if (model) {
            // save
            char *fname = strdup (name);
            fname[size-1] = 'x';
            model->fileName = fname;
            model->Save();
        }
    }
    //model->Save();
    return model;
}

void Model3dx :: Unload( void )
{
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
        delete[] model;
        model = NULL;
    }

    if (m_Name) { delete[] m_Name; m_Name = NULL; }
}
