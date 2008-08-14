#ifndef __incl_lib3dx_xMaterial_h
#define __incl_lib3dx_xMaterial_h

#include <cstdio>
#include "../../Math/xMath.h"

struct xTexture {
    char    *Name;
    xDWORD   htex;
};

struct xMaterial {
    char    *Name;
    xBYTE    ID;
    xColor   ambient;
    xColor   diffuse;
    xColor   specular;
    xFLOAT   shininess_gloss;
    xFLOAT   shininess_level;
    xFLOAT   transparency;
    bool     self_illum;
    char    *shader;
    bool     two_sided;
    bool     use_wire;
    bool     use_wire_abs;
    xFLOAT   wire_size;

    xTexture texture;

    xMaterial *Next; // next sibling

    void Free();

    xMaterial *ByName( const char *materialName );
    xMaterial *ById  ( xBYTE ID_material );

           void       Save( FILE *file );
    static xMaterial *Load( FILE *file );
};

#endif
