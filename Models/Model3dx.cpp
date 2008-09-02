#include "Model3dx.h"

#include "lib3dx/xImport.h"
#include "../Graphics/OGL/Textures/TextureMgr.h"

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
        if (model && Config::Save3dsTo3dx) {
            // save
            char *fname = strdup (name);
            fname[size-1] = 'x';
            model->FileName = fname;
        }
        else
            model->FileName = strdup (name);
    }
    //model->Save();
    return model;
}

void Model3dx :: Unload( void )
{
    if (model)
    {
        if (model->FL_textures_loaded)
            for (xMaterial *mat = model->L_material; mat; mat = mat->Next)
                if (mat->texture.Name && mat->texture.htex)
                {
                    HTexture htex;
                    htex.SetHandle(mat->texture.htex);
                    g_TextureMgr.DeleteTexture(htex);
                    mat->texture.htex = 0;
                }
        model->FL_textures_loaded = false;
        model->Free();
        model = NULL;
    }

    if (m_Name) { delete[] m_Name; m_Name = NULL; }
}
