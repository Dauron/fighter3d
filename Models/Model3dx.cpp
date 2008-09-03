#include "Model3dx.h"

#include "lib3dx/xImport.h"
#include "../Graphics/OGL/Textures/TextureMgr.h"
#include "../Utils/Filesystem.h"

bool Model3dx :: Load ( const char *name )
{
    assert (model == NULL);

    m_Name = name;

    int size = m_Name.size();
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
            model->FileName = strdup (Filesystem::ChangeFileExt(m_Name, "3dx").c_str());
            model->Save();
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
                    g_TextureMgr.DeleteReference(htex);
                    mat->texture.htex = 0;
                }
        model->FL_textures_loaded = false;
        model->Free();
        model = NULL;
    }
    m_Name.clear();
}
