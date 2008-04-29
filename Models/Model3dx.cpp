#include "lib3dx/xImport.h"
#include "../Utils/Utils.h"

#include "Model3dx.h"

bool Model3dx :: Load ( const char *name )
{
    assert (renderer.xModel == NULL);

    if (m_Name != name) m_Name = strdup(name); // set name if not reloading

    int size = strlen(name);
    if (!strcasecmp(name + size - 4, ".3dx"))
        renderer.Initialize(xFileLoad (name));
    else
    if (!strcasecmp(name + size - 4, ".3dm"))
        renderer.Initialize(xLoadFrom3dmFile (name));
    else
    {
        Lib3dsFile *file3ds = lib3ds_file_load (name);
        renderer.Initialize(xImportFileFrom3ds(file3ds));
        lib3ds_file_free(file3ds);
    }

    // save
    //char *fname = strdup (name);
    //fname[size-1] = 'x';
    //xFileSave(fname, renderer.xModel);
    //delete[] fname;

    if (renderer.xModel) renderer.xModel->texturesInited = false;
    return renderer.xModel != NULL;
}
