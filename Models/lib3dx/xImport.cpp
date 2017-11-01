#include "xImport.h"
#include <cstring>
#include <cassert>
#include <fstream>
#include <vector>

#ifdef WIN32

#include "../lib3ds/camera.h"
#include "../lib3ds/mesh.h"
#include "../lib3ds/node.h"
#include "../lib3ds/material.h"
#include "../lib3ds/matrix.h"
#include "../lib3ds/vector.h"
#include "../lib3ds/light.h"

#else

#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>

#endif

xElement *xImportElementFrom3ds(Lib3dsFile *model, xModel *xmodel, Lib3dsNode *node, xWORD &currId)
{
    xElement *elem = new xElement();
    elem->ID = currId;
    elem->Color.r =  elem->Color.g = elem->Color.b = elem->Color.a = 1.0f;
    elem->Next = NULL;
    elem->Name = strdup(node->name);
    elem->L_vertices = NULL;
    elem->L_kids = NULL;
    elem->I_kids = 0;

    memset(&(elem->renderData), 0, sizeof(elem->renderData));
    memset(&(elem->boundingData), 0, sizeof(elem->boundingData));
    memcpy(elem->MX_MeshToLocal.matrix, node->matrix, sizeof(elem->MX_MeshToLocal));

    int mirrorFaces = 0;
    xMatrix scl;
    scl.identity();
    if (node->data.object.scl_track.keyL)
    {
        scl.x0 *= node->data.object.scl_track.keyL->value[0];
        scl.y1 *= node->data.object.scl_track.keyL->value[1];
        scl.z2 *= node->data.object.scl_track.keyL->value[2];
        if (scl.x0 < 0.f) ++mirrorFaces;
        if (scl.y1 < 0.f) ++mirrorFaces;
        if (scl.z2 < 0.f) ++mirrorFaces;
        if (mirrorFaces == 2) mirrorFaces = 0;
    }
    xVector4 pivot; pivot.init(-node->data.object.pivot[0], -node->data.object.pivot[1], -node->data.object.pivot[2], 1.f);
    pivot = scl * pivot;
    elem->MX_MeshToLocal.preTranslateT(pivot.vector3);

    elem->L_smooth    = NULL;
    elem->L_faces     = NULL;
    elem->I_faces     = 0;
    elem->L_faces     = NULL;
    elem->I_faceLists = 0;
    elem->L_faceLists = NULL;
    elem->I_edges     = 0;
    elem->L_edges     = NULL;
    elem->I_vertices  = 0;
    elem->L_vertices  = NULL;
    elem->FL_textured = elem->FL_skeletized = false;

    if (node->type == LIB3DS_OBJECT_NODE && strcmp(node->name,"$$$DUMMY")!=0)
    {
        Lib3dsMesh *mesh = lib3ds_file_mesh_by_name(model, node->name);
        if (mesh)
        {
            elem->I_faces = mesh->faces;
            elem->I_vertices = mesh->points;
            elem->FL_skeletized = xmodel->Spine.I_bones != 0;

            assert(!mesh->texels || mesh->points == mesh->texels);

            size_t stride;
            elem->FL_textured = mesh->texels;
            if (elem->FL_textured)
            {
                elem->L_verticesT = new xVertexTex[elem->I_vertices];
                stride = sizeof(xVertexTex);
            }
            else
            {
                elem->L_vertices = new xVertex[elem->I_vertices];
                stride = sizeof(xVertex);
            }

            xVertex     *xvert   = elem->L_vertices;
            xVertexTex  *xvertT  = elem->L_verticesT;
            Lib3dsPoint *L_kids = mesh->pointL;
            Lib3dsPoint *lastP  = L_kids + mesh->points;
            Lib3dsTexel *firstT = mesh->texelL;
            for(; L_kids != lastP; ++L_kids)
            {
                if (elem->FL_textured) {
                    xvertT->pos.x = L_kids->pos[0];
                    xvertT->pos.y = L_kids->pos[1];
                    xvertT->pos.z = L_kids->pos[2];
                    if (firstT) {
                        xvertT->tex.u = (*firstT)[0];
                        xvertT->tex.v = (*firstT)[1];
                        ++firstT;
                    }
                    ++xvertT;
                }
                else
                {
                    xvert->pos.x = L_kids->pos[0];
                    xvert->pos.y = L_kids->pos[1];
                    xvert->pos.z = L_kids->pos[2];
                    ++xvert;
                }
            }

            Lib3dsFace *firstF = mesh->faceL;
            Lib3dsFace *lastF  = firstF + mesh->faces;
            xMaterial  *lastM  = NULL;
            std::vector<std::vector<std::vector<xWORD> > > faces; // material=>smoothing=>faces
            faces.resize(xmodel->I_material+1);

            xWORD mid = 0, faceListC = 0;
            for(; firstF != lastF; ++firstF)
            {
                if (firstF->material[0]) {
                    if (!lastM || strcmp(firstF->material, lastM->Name))
                    {
                        lastM = xmodel->L_material->ByName(firstF->material);
                        bool transparent = lastM->transparency > 0.f;
                        elem->FL_transparent |= transparent;
                        elem->FL_opaque      |= !transparent;
                    }
                    mid = lastM->ID;
                }
                else
                {
                    mid = 0;
                    xmodel->FL_opaque = elem->FL_opaque = true;
                }

                xDWORD smooth = firstF->smoothing;
                xBYTE  smGrp = smooth ? 1 : 0;
                if (faces[mid].size() == 0) faces[mid].resize(2);
                if (!faces[mid][smGrp].size()) ++faceListC;
                if (!mirrorFaces)
                {
                    faces[mid][smGrp].push_back(firstF->points[0]);
                    faces[mid][smGrp].push_back(firstF->points[1]);
                    faces[mid][smGrp].push_back(firstF->points[2]);
                }
                else
                {
                    faces[mid][smGrp].push_back(firstF->points[0]);
                    faces[mid][smGrp].push_back(firstF->points[2]);
                    faces[mid][smGrp].push_back(firstF->points[1]);
                }
                faces[mid][smGrp].push_back((xWORD)smooth);
                faces[mid][smGrp].push_back((xWORD)(smooth >> 16));
            }

            elem->I_faceLists  = faceListC;
            elem->L_faceLists  = new xFaceList[faceListC];
            xFaceList *faceL = elem->L_faceLists;
            elem->L_faces     = new xFace[elem->I_faces];
            elem->L_smooth    = new xDWORD[elem->I_faces];

            xFace  *faceP   = elem->L_faces;
            xDWORD *L_smooth = elem->L_smooth;
            xWORD   offset  = 0;
            for (mid=0; mid<faces.size(); ++mid)
                for (size_t group=0; group<faces[mid].size(); ++group)
                    if (faces[mid][group].size()) {

                        faceL->I_offset    = offset;
                        faceL->FL_smooth   = group;
                        faceL->ID_material = mid;
                        if (xmodel->L_material)
                            faceL->Material = xmodel->L_material->ById(mid);
                        else
                            faceL->Material = NULL;

                        std::vector<xWORD>::iterator iter = faces[mid][group].begin();
                        std::vector<xWORD>::iterator end  = faces[mid][group].end();

                        for (; iter != end; ++faceP, ++offset, ++L_smooth)
                        {
                            (*faceP)[0] = *(iter++);
                            (*faceP)[1] = *(iter++);
                            (*faceP)[2] = *(iter++);
                            *((xWORD*)L_smooth+0) = *(iter++);
                            *((xWORD*)L_smooth+1) = *(iter++);
                        }
                        faceL->I_count = offset - faceL->I_offset;
                        ++faceL;
                    }
            elem->FillShadowEdges();
            elem->CalculateSmoothVertices();
        }
    }

    elem->boundingData.Fill(*xmodel, *elem);

    xElement *laste = NULL;
    for (Lib3dsNode *snode = node->childs; snode != NULL; snode = snode->next)
    {
        if (laste)
            laste = laste->Next = xImportElementFrom3ds(model,xmodel,snode, ++currId);
        else
            laste = elem->L_kids = xImportElementFrom3ds(model,xmodel,snode, ++currId);
        ++(elem->I_kids);
    }

    return elem;
}

xModel *xImportFileFrom3ds(Lib3dsFile *model)
{
    xModel *xmodel = new xModel();

    xmodel->BVHierarchy = NULL;
    xmodel->L_material = 0;
    xmodel->I_material = 0;
    xmodel->FL_textures_loaded = false;
    xmodel->FL_transparent = false;
    xmodel->FL_opaque      = false;
    xmodel->Spine.I_bones  = 0;
    xmodel->Spine.L_bones  = NULL;
    xmodel->FL_save_binfo  = true;
    xmodel->FL_save_rdata  = true;
    xmodel->FileName = 0;

    xMaterial *lastm = NULL;
    xWORD matId = 0;

    for (Lib3dsMaterial *mat = model->materials; mat != NULL; mat = mat->next)
    {
        if (lastm)
            lastm = lastm->Next = new xMaterial();
        else
            lastm = xmodel->L_material = new xMaterial();

        memcpy(lastm->ambient.col, mat->ambient, sizeof(Lib3dsRgba));
        memcpy(lastm->diffuse.col, mat->diffuse, sizeof(Lib3dsRgba));
        memcpy(lastm->specular.col, mat->specular, sizeof(Lib3dsRgba));
        lastm->Name = strdup(mat->name);
        lastm->ID = ++matId;
        lastm->Next = NULL;
        lastm->shininess_gloss = mat->shininess;
        lastm->shininess_level = mat->shin_strength / 100.f;
        lastm->transparency = mat->transparency;
        lastm->self_illum = mat->self_illum;
        lastm->shader = 0;
        lastm->two_sided = mat->two_sided;
        lastm->use_wire = mat->use_wire;
        lastm->use_wire_abs = mat->use_wire_abs;
        lastm->wire_size = mat->wire_size;
        lastm->texture.Name = mat->texture1_map.name[0] ? strdup(mat->texture1_map.name) : NULL;
        bool transparent = lastm->transparency > 0.f;
        xmodel->FL_transparent |= transparent;
        xmodel->FL_opaque      |= !transparent;
        ++(xmodel->I_material);
    }

    xmodel->L_kids = NULL;
    xmodel->I_kids = 0;

    xWORD currId = -1;
    xElement *laste = NULL;
    for (Lib3dsNode *node = model->nodes; node != NULL; node = node->next)
    {
        if (laste)
            laste = laste->Next  = xImportElementFrom3ds(model,xmodel,node, ++currId);
        else
            laste = xmodel->L_kids = xImportElementFrom3ds(model,xmodel,node, ++currId);
        ++(xmodel->I_kids);
    }
    xmodel->I_elements = xmodel->L_kids->CountAll();

    return xmodel;
}
