#include "xImport.h"
#include <cstring>
#include <cassert>
#include <fstream>
#include <vector>

#ifdef WIN32

#pragma warning(disable : 4996) // deprecated
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
    elem->id = currId;
    elem->color.r =  elem->color.g = elem->color.b = elem->color.a = 1.0f;
    elem->nextP = NULL;
    elem->name = strdup(node->name);
    elem->verticesP = NULL;
    elem->kidsP = NULL;
    elem->kidsC = 0;

    memset(&(elem->renderData), 0, sizeof(elem->renderData));
    memset(&(elem->collisionData), 0, sizeof(elem->collisionData));
    memcpy(elem->matrix.matrix, node->matrix, sizeof(elem->matrix));

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
    elem->matrix.preTranslateT(pivot.vector3);

    elem->smoothP = NULL;
    elem->facesP = NULL;
    elem->facesC = 0;
    elem->facesP = NULL;
    elem->faceListC = 0;
    elem->faceListP = NULL;
    elem->edgesP = NULL;
    elem->verticesC = 0;
    elem->verticesP = NULL;
    elem->textured = elem->skeletized = false;

    if (node->type == LIB3DS_OBJECT_NODE && strcmp(node->name,"$$$DUMMY")!=0)
    {
        Lib3dsMesh *mesh = lib3ds_file_mesh_by_name(model, node->name);
        if (mesh)
        {
            elem->facesC = mesh->faces;
            elem->verticesC = mesh->points;
            elem->skeletized = xmodel->spine.I_bones != 0;

            assert(!mesh->texels || mesh->points == mesh->texels);

            size_t stride;
            elem->textured = mesh->texels;
            if (elem->textured)
            {
                elem->verticesTP = new xVertexTex[elem->verticesC];
                stride = sizeof(xVertexTex);
            }
            else
            {
                elem->verticesP = new xVertex[elem->verticesC];
                stride = sizeof(xVertex);
            }

            xVertex     *xvert   = elem->verticesP;
            xVertexTex  *xvertT  = elem->verticesTP;
            Lib3dsPoint *kidsP = mesh->pointL;
            Lib3dsPoint *lastP  = kidsP + mesh->points;
            Lib3dsTexel *firstT = mesh->texelL;
            for(; kidsP != lastP; ++kidsP)
            {
                if (elem->textured) {
                    xvertT->x = kidsP->pos[0];
                    xvertT->y = kidsP->pos[1];
                    xvertT->z = kidsP->pos[2];
                    if (firstT) {
                        xvertT->tx = (*firstT)[0];
                        xvertT->ty = (*firstT)[1];
                        ++firstT;
                    }
                    ++xvertT;
                }
                else
                {
                    xvert->x = kidsP->pos[0];
                    xvert->y = kidsP->pos[1];
                    xvert->z = kidsP->pos[2];
                    ++xvert;
                }
            }

            Lib3dsFace *firstF = mesh->faceL;
            Lib3dsFace *lastF  = firstF + mesh->faces;
            xMaterial  *lastM  = NULL;
            std::vector<std::vector<std::vector<xWORD> > > faces; // material=>smoothing=>faces
            faces.resize(xmodel->materialC+1);

            xWORD mid = 0, faceListC = 0;
            for(; firstF != lastF; ++firstF)
            {
                if (firstF->material[0]) {
                    if (!lastM || strcmp(firstF->material, lastM->name))
                    {
                        lastM = xmodel->materialP->ByName(firstF->material);
                        bool transparent = lastM->transparency > 0.f;
                        elem->transparent |= transparent;
                        elem->opaque      |= !transparent;
                    }
                    mid = lastM->id;
                }
                else
                {
                    mid = 0;
                    xmodel->opaque = elem->opaque = true;
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

            elem->faceListC  = faceListC;
            elem->faceListP  = new xFaceList[faceListC];
            xFaceList *faceL = elem->faceListP;
            elem->facesP     = new xFace[elem->facesC];
            elem->smoothP    = new xDWORD[elem->facesC];

            xFace  *faceP   = elem->facesP;
            xDWORD *smoothP = elem->smoothP;
            xWORD   offset  = 0;
            for (mid=0; mid<faces.size(); ++mid)
                for (size_t group=0; group<faces[mid].size(); ++group)
                    if (faces[mid][group].size()) {

                        faceL->indexOffset = offset;
                        faceL->smooth      = group;
                        faceL->materialId  = mid;
                        if (xmodel->materialP)
                            faceL->materialP = xmodel->materialP->ById(mid);
                        else
                            faceL->materialP = NULL;

                        std::vector<xWORD>::iterator iter = faces[mid][group].begin();
                        std::vector<xWORD>::iterator end  = faces[mid][group].end();

                        for (; iter != end; ++faceP, ++offset, ++smoothP)
                        {
                            (*faceP)[0] = *(iter++);
                            (*faceP)[1] = *(iter++);
                            (*faceP)[2] = *(iter++);
                            *((xWORD*)smoothP+0) = *(iter++);
                            *((xWORD*)smoothP+1) = *(iter++);
                        }
                        faceL->indexCount = offset - faceL->indexOffset;
                        ++faceL;
                    }
            elem->FillShadowEdges();
            elem->CalculateSmoothVertices();
        }
    }

    elem->collisionData.Fill(xmodel, elem);

    xElement *laste = NULL;
    for (Lib3dsNode *snode = node->childs; snode != NULL; snode = snode->next)
    {
        if (laste)
            laste = laste->nextP = xImportElementFrom3ds(model,xmodel,snode, ++currId);
        else
            laste = elem->kidsP = xImportElementFrom3ds(model,xmodel,snode, ++currId);
        ++(elem->kidsC);
    }

    return elem;
}

xModel *xImportFileFrom3ds(Lib3dsFile *model)
{
    xModel *xmodel = new xModel();

    xmodel->materialP = 0;
    xmodel->materialC = 0;
    xmodel->texturesInited = false;
    xmodel->transparent = false;
    xmodel->opaque = false;
    xmodel->spine.I_bones = 0;
    xmodel->spine.L_bones = NULL;
    xmodel->saveCollisionData = true;
    xmodel->fileName = 0;
    xmodel->saveCollisionData = true;

    xMaterial *lastm = NULL;
    xWORD matId = 0;

    for (Lib3dsMaterial *mat = model->materials; mat != NULL; mat = mat->next)
    {
        if (lastm)
            lastm = lastm->nextP = new xMaterial();
        else
            lastm = xmodel->materialP = new xMaterial();

        memcpy(lastm->ambient.col, mat->ambient, sizeof(Lib3dsRgba));
        memcpy(lastm->diffuse.col, mat->diffuse, sizeof(Lib3dsRgba));
        memcpy(lastm->specular.col, mat->specular, sizeof(Lib3dsRgba));
        lastm->name = strdup(mat->name);
        lastm->id = ++matId;
        lastm->nextP = NULL;
        lastm->shininess_gloss = mat->shininess;
        lastm->shininess_level = mat->shin_strength / 100.f;
        lastm->transparency = mat->transparency;
        lastm->self_illum = mat->self_illum;
        lastm->shader = 0;
        lastm->two_sided = mat->two_sided;
        lastm->use_wire = mat->use_wire;
        lastm->use_wire_abs = mat->use_wire_abs;
        lastm->wire_size = mat->wire_size;
        lastm->texture.name = mat->texture1_map.name[0] ? strdup(mat->texture1_map.name) : NULL;
        bool transparent = lastm->transparency > 0.f;
        xmodel->transparent |= transparent;
        xmodel->opaque      |= !transparent;
        ++(xmodel->materialC);
    }

    xmodel->kidsP = NULL;
    xmodel->kidsC = 0;

    xWORD currId = -1;
    xElement *laste = NULL;
    for (Lib3dsNode *node = model->nodes; node != NULL; node = node->next)
    {
        if (laste)
            laste = laste->nextP  = xImportElementFrom3ds(model,xmodel,node, ++currId);
        else
            laste = xmodel->kidsP = xImportElementFrom3ds(model,xmodel,node, ++currId);
        ++(xmodel->kidsC);
    }
    xmodel->elementC = xmodel->kidsP->CountAll();

    return xmodel;
}
