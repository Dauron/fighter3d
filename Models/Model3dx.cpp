#include "Model3dx.h"

#include "lib3dx/xImport.h"
#include "../Graphics/OGL/Textures/TextureMgr.h"
#include "../Utils/Filesystem.h"

bool Model3dx :: Create ()
{
    assert (model == NULL);
    assert (Name.size());

    const char *name = Name.c_str();
    if (!strcasecmp(name + Name.size() - 4, ".3dx"))
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
            model->FileName = strdup (Filesystem::ChangeFileExt(Name, "3dx").c_str());
            model->Save();
        }
        else
            model->FileName = strdup (name);
    }
    //model->Save();
    return model;
}

void Model3dx :: Dispose()
{
    if (model)
    {
        if (model->FL_textures_loaded)
            for (xMaterial *mat = model->L_material; mat; mat = mat->Next)
                if (mat->texture.Name && mat->texture.htex)
                {
                    HTexture htex;
                    htex.SetHandle(mat->texture.htex);
                    g_TextureMgr.Release(htex);
                    mat->texture.htex = 0;
                }
        model->FL_textures_loaded = false;
        model->Free();
        model = NULL;
    }
}
