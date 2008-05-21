#include "xMaterial.h"

////////////////////// xMaterial
xMaterial *xMaterial :: ByName(const char *materialName)
{
    xMaterial *mat = this;
    for (; mat; mat = mat->nextP)
        if (!strcmp(materialName, mat->name))
            break;
    return mat;
}

xMaterial *xMaterial :: ById(xBYTE materialId)
{
    xMaterial *mat = this;
    for (; mat; mat = mat->nextP)
        if (materialId == mat->id)
            break;
    return mat;
}

void       xMaterial :: Free()
{
    if (this->name)
        delete[] this->name;
    if (this->shader)
        delete[] this->shader;
    if (this->texture.name)
        delete[] this->texture.name;
    delete this;
}

xMaterial *xMaterial :: Load(FILE *file)
{
    xMaterial *mat = new xMaterial();
    if (!mat) return NULL;

    fread(&mat->name, sizeof(char*), 1, file);
    fread(&mat->id, sizeof(xBYTE), 1, file);
    fread(&mat->ambient,  sizeof(xColor), 1, file);
    fread(&mat->diffuse,  sizeof(xColor), 1, file);
    fread(&mat->specular,  sizeof(xColor), 1, file);
    fread(&mat->shininess_gloss,  sizeof(xFLOAT), 1, file);
    fread(&mat->shininess_level,  sizeof(xFLOAT), 1, file);
    fread(&mat->transparency,  sizeof(xFLOAT), 1, file);

    fread(&mat->self_illum,  sizeof(bool), 1, file);
    fread(&mat->shader,  sizeof(char*), 1, file);
    fread(&mat->two_sided,  sizeof(bool), 1, file);
    fread(&mat->use_wire,  sizeof(bool), 1, file);
    fread(&mat->use_wire_abs,  sizeof(bool), 1, file);
    fread(&mat->wire_size,  sizeof(xFLOAT), 1, file);

    fread(&mat->texture.name,  sizeof(mat->texture.name), 1, file);

    mat->nextP = NULL;
    mat->texture.htex = 0;

    if (mat->name)
    {
        size_t len = (size_t) mat->name;
        mat->name = new char[len];
        if (!(mat->name) ||
            fread(mat->name, 1, len, file) != len)
        {
            mat->texture.name = NULL;
            mat->Free();
            return NULL;
        }
    }
    if (mat->texture.name)
    {
        size_t len = (size_t) mat->texture.name;
        mat->texture.name = new char[len];
        if (!(mat->texture.name) ||
            fread(mat->texture.name, 1, len, file) != len)
        {
            mat->Free();
            return NULL;
        }
    }
    return mat;
}

void       xMaterial :: Save(FILE *file)
{
    char *name = this->name;
    char *tname = this->texture.name;
    if (name)
        this->name = (char *) strlen(name)+1;
    if (tname)
        this->texture.name = (char *) strlen(tname)+1;

    fwrite(&this->name,            sizeof(char*), 1, file);
    fwrite(&this->id,              sizeof(xBYTE), 1, file);
    fwrite(&this->ambient,         sizeof(xColor), 1, file);
    fwrite(&this->diffuse,         sizeof(xColor), 1, file);
    fwrite(&this->specular,        sizeof(xColor), 1, file);
    fwrite(&this->shininess_gloss, sizeof(xFLOAT), 1, file);
    fwrite(&this->shininess_level, sizeof(xFLOAT), 1, file);
    fwrite(&this->transparency,    sizeof(xFLOAT), 1, file);

    fwrite(&this->self_illum,      sizeof(bool), 1, file);
    fwrite(&this->shader,          sizeof(char*), 1, file);
    fwrite(&this->two_sided,       sizeof(bool), 1, file);
    fwrite(&this->use_wire,        sizeof(bool), 1, file);
    fwrite(&this->use_wire_abs,    sizeof(bool), 1, file);
    fwrite(&this->wire_size,       sizeof(xFLOAT), 1, file);

    fwrite(&this->texture.name,    sizeof(char*), 1, file);

    if (name)
        fwrite(name, 1, (size_t)this->name, file);
    if (tname)
        fwrite(tname, 1, (size_t)this->texture.name, file);

    this->name = name;
    this->texture.name = tname;
}
