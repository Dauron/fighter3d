#include "xMaterial.h"

////////////////////// xMaterial
xMaterial *xMaterial :: ByName(const char *materialName)
{
    xMaterial *mat = this;
    for (; mat; mat = mat->Next)
        if (!strcmp(materialName, mat->Name))
            break;
    return mat;
}

xMaterial *xMaterial :: ById(xBYTE ID_material)
{
    xMaterial *mat = this;
    for (; mat; mat = mat->Next)
        if (ID_material == mat->ID)
            break;
    return mat;
}

void       xMaterial :: Free()
{
    if (this->Name)
        delete[] this->Name;
    if (this->shader)
        delete[] this->shader;
    if (this->texture.Name)
        delete[] this->texture.Name;
    delete this;
}

xMaterial *xMaterial :: Load(FILE *file)
{
    xMaterial *mat = new xMaterial();
    if (!mat) return NULL;

    fread(&mat->Name, sizeof(char*), 1, file);
    fread(&mat->ID, sizeof(xBYTE), 1, file);
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

    fread(&mat->texture.Name,  sizeof(mat->texture.Name), 1, file);

    mat->Next = NULL;
    mat->texture.htex = 0;

    if (mat->Name)
    {
        size_t len = (size_t) mat->Name;
        mat->Name = new char[len];
        if (!(mat->Name) ||
            fread(mat->Name, 1, len, file) != len)
        {
            mat->texture.Name = NULL;
            mat->Free();
            return NULL;
        }
    }
    if (mat->texture.Name)
    {
        size_t len = (size_t) mat->texture.Name;
        mat->texture.Name = new char[len];
        if (!(mat->texture.Name) ||
            fread(mat->texture.Name, 1, len, file) != len)
        {
            mat->Free();
            return NULL;
        }
    }
    return mat;
}

void       xMaterial :: Save(FILE *file)
{
    char *name = this->Name;
    char *tname = this->texture.Name;
    if (name)
        this->Name = (char *) strlen(name)+1;
    if (tname)
        this->texture.Name = (char *) strlen(tname)+1;

    fwrite(&this->Name,            sizeof(char*), 1, file);
    fwrite(&this->ID,              sizeof(xBYTE), 1, file);
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

    fwrite(&this->texture.Name,    sizeof(char*), 1, file);

    if (name)
        fwrite(name, 1, (size_t)this->Name, file);
    if (tname)
        fwrite(tname, 1, (size_t)this->texture.Name, file);

    this->Name = name;
    this->texture.Name = tname;
}
