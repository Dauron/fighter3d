#include "xImport.h"
#include <cstring>
#include <cassert>
#include <fstream>
#include <vector>
#include "xSkeleton.h"
#include "xUtils.h"

#include "../lib3ds/camera.h"
#include "../lib3ds/mesh.h"
#include "../lib3ds/node.h"
#include "../lib3ds/material.h"
#include "../lib3ds/matrix.h"
#include "../lib3ds/vector.h"
#include "../lib3ds/light.h"

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

xElement *xImportElementFrom3ds(Lib3dsFile *model, xFile *file, Lib3dsNode *node, xWORD &currId)
{
    xElement *elem = new xElement();
    elem->id = currId;
    elem->color.r =  elem->color.g = elem->color.b = elem->color.a = 1.0f;
    elem->nextP = NULL;
    elem->name = strdup(node->name);
    elem->verticesP = NULL;
    elem->facesP = NULL;
    elem->kidsP = NULL;
    elem->kidsC = elem->facesC = elem->verticesC = 0;

    memset(&(elem->renderData), 0, sizeof(elem->renderData));
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
    elem->matrix = xMatrixTranslate(pivot.vector3.xyz).transpose() * elem->matrix;

    elem->collisionData.kidsP = NULL;
    elem->collisionData.kidsC = 0;

    elem->smoothP = NULL;
    elem->facesC = 0;
    elem->facesP = NULL;
    elem->faceListC = 0;
    elem->faceListP = NULL;
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
            elem->skeletized = file->spineP != NULL;

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
            Lib3dsPoint *firstP = mesh->pointL;
            Lib3dsPoint *lastP  = firstP + mesh->points;
            Lib3dsTexel *firstT = mesh->texelL;
            for(; firstP != lastP; ++firstP)
            {
                if (elem->textured) {
                    xvertT->x = firstP->pos[0];
                    xvertT->y = firstP->pos[1];
                    xvertT->z = firstP->pos[2];
                    if (firstT) {
                        xvertT->tx = (*firstT)[0];
                        xvertT->ty = (*firstT)[1];
                        ++firstT;
                    }
                    ++xvertT;
                }
                else
                {
                    xvert->x = firstP->pos[0];
                    xvert->y = firstP->pos[1];
                    xvert->z = firstP->pos[2];
                    ++xvert;
                }
            }

            Lib3dsFace *firstF = mesh->faceL;
            Lib3dsFace *lastF  = firstF + mesh->faces;
            xMaterial  *lastM  = NULL;
            std::vector<std::vector<std::vector<xWORD> > > faces; // material=>smoothing=>faces
            faces.resize(file->materialC+1);

            xWORD mid = 0, faceListC = 0;
            for(; firstF != lastF; ++firstF)
            {
                if (firstF->material[0]) {
                    if (!lastM || strcmp(firstF->material, lastM->name))
                    {
                        lastM = xMaterialByName(file,firstF->material);
                        bool transparent = lastM->transparency > 0.f;
                        elem->renderData.transparent |= transparent;
                        elem->renderData.opaque      |= !transparent;
                    }
                    mid = lastM->id;
                }
                else
                {
                    mid = 0;
                    file->opaque = elem->renderData.opaque = true;
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
            elem->facesP     = new xWORD3[elem->facesC];
            elem->smoothP    = new xDWORD[elem->facesC];

            xWORD3 *faceP   = elem->facesP;
            xDWORD *smoothP = elem->smoothP;
            xWORD   offset  = 0;
            for (mid=0; mid<faces.size(); ++mid)
                for (size_t group=0; group<faces[mid].size(); ++group)
                    if (faces[mid][group].size()) {

                        faceL->indexOffset = offset;
                        faceL->normalP     = NULL;
                        faceL->smooth      = group;
                        faceL->materialId  = mid;
                        faceL->materialP   = xMaterialById(file, mid);

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
            xElementCalculateSmoothVertices(elem);
        }
    }

    file->saveCollisionData = true;
    xElement_GetCollisionHierarchy(file, elem);

    xElement *laste = NULL;
    for (Lib3dsNode *snode = node->childs; snode != NULL; snode = snode->next)
    {
        if (laste)
            laste = laste->nextP = xImportElementFrom3ds(model,file,snode, ++currId);
        else
            laste = elem->kidsP = xImportElementFrom3ds(model,file,snode, ++currId);
        ++(elem->kidsC);
    }

    return elem;
}

xFile *xImportFileFrom3ds(Lib3dsFile *model)
{
    xFile *file = new xFile();

    file->materialP = 0;
    file->materialC = 0;
    file->texturesInited = false;
    file->transparent = false;
    file->opaque = false;
    file->spineP = 0;
    file->saveCollisionData = true;
    file->fileName = 0;
    xMaterial *lastm = NULL;
    xWORD matId = 0;
    for (Lib3dsMaterial *mat = model->materials; mat != NULL; mat = mat->next)
    {
        if (lastm)
            lastm = lastm->nextP = new xMaterial();
        else
            lastm = file->materialP = new xMaterial();

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
        file->transparent |= transparent;
        file->opaque      |= !transparent;
        ++(file->materialC);
    }

    file->firstP = new xElement();
    memset(file->firstP, 0, sizeof(xElement));

    xWORD currId = 0;
    xElement *laste = NULL;
    for (Lib3dsNode *node = model->nodes; node != NULL; node = node->next)
    {
        if (laste)
            laste = laste->nextP = xImportElementFrom3ds(model,file,node, ++currId);
        else
            laste = file->firstP->kidsP = xImportElementFrom3ds(model,file,node, ++currId);
        ++(file->firstP->kidsC);
    }

    return file;
}

xFile *xLoadFrom3dmFile(const char *fileName)
{
    int v1,v2,v3,r,g,b,a,t, noVerts;
    char buffer[1000];

    std::ifstream file(fileName);
    if (!file.good())
    {
        strcpy(buffer, "Unable to open the object file: ");
        strcat(buffer, fileName);
        return NULL;
    }

    xFile *model = new xFile();
    model->texturesInited = false;
    model->saveCollisionData = true;
    model->fileName = 0;
    model->materialC = 0;
    model->materialP = 0;
    model->spineP = 0;
    xElement *elem = model->firstP = new xElement();

    memset(&(elem->renderData), 0, sizeof(elem->renderData));
    elem->color.a = elem->color.r = elem->color.g = elem->color.b = 1.0;
    elem->kidsC = 0;
    elem->kidsP = 0;
    elem->nextP = 0;
    for (int x=0; x < 4; ++x)
        for (int y=0; y < 4; ++y)
            elem->matrix.matrix[x][y] = (x == y ? 1.0f : 0.0f);
    elem->skeletized = true;
    elem->textured = true;

    while (file.good() && file.getline(buffer, 1000))
    {
        if (buffer[0] == '#') continue;
        elem->name = strdup(buffer);
        break;
    }

    // textures
    while (file.good() && file.getline(buffer, 1000))
    {
        if (buffer[0] == '#') continue;
        sscanf(buffer, "%d", &noVerts);
        break;
    }
    if (file.good() && noVerts > 0)
    {
        //pTextures = new HTexture[nTextures];
        //HTexture *iter = pTextures;
        //HTexture *end  = pTextures + nTextures;
        int iter = 0;
        int end = noVerts;

        do
        {
            while (file.good() && file.getline(buffer, 1000))
            {
                if (buffer[0] == '#') continue;
                //*iter = g_TextureMgr.GetTexture(buffer);
                break;
            }
        }
        while (++iter != end);
    }
    // bones
    while (file.good() && file.getline(buffer, 1000))
    {
        if (buffer[0] == '#') continue;
        sscanf(buffer, "%d", &noVerts);
        break;
    }
    if (file.good() && noVerts > 0)
    {
        int prevVerts = noVerts;
        xSkeletonAdd(model);

        do
        {
            while (file.good() && file.getline(buffer, 1000))
            {
                if (buffer[0] == '#') continue;

                sscanf(buffer, "%d\t%d", &a, &t);
                xBone *parent = xBoneById(model->spineP, a);
                xBone *bone   = xBoneById(model->spineP, t);

                if (parent && !bone) {
                    ++parent->kidsC;
                    if (!parent->kidsP)
                        bone = parent->kidsP = new xBone();
                    else {
                        bone = parent->kidsP;
                        while (bone->nextP)
                            bone = bone->nextP;
                        bone = bone->nextP = new xBone();
                    }
                    xBoneReset(bone);
                    bone->id = t;
                }
                break;
            }
        }
        while (--noVerts);

        noVerts = prevVerts;
        do
        {
            while (file.good() && file.getline(buffer, 1000))
            {
                if (buffer[0] == '#') continue;

                xBone *bone = xBoneById(model->spineP, prevVerts-noVerts);
                sscanf(buffer, "%f\t%f\t%f",
                    &bone->ending.x, &bone->ending.y, &bone->ending.z);
                break;
            }
        }
        while (--noVerts >= 0);
    }
    // vertices
    while (file.good() && file.getline(buffer, 1000))
    {
        if (buffer[0] == '#') continue;
        sscanf(buffer, "%d", &noVerts);
        elem->verticesC = noVerts;
        break;
    }
    if (file.good() && elem->verticesC > 0)
    {
        elem->verticesTSP = new xVertexTexSkel[elem->verticesC];
        xVertexTexSkel *iter = elem->verticesTSP;
        xVertexTexSkel *end  = elem->verticesTSP + elem->verticesC;

        do
        {
            while (file.good() && file.getline(buffer, 1000))
            {
                if (buffer[0] == '#') continue;
                sscanf(buffer, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",
                    &iter->x, &iter->y, &iter->z, &iter->tx, &iter->ty,
                    &iter->b0, &iter->b1, &iter->b2, &iter->b3);
                break;
            }
        }
        while (++iter != end);
    }

    // faces
    while (file.good() && file.getline(buffer, 1000))
    {
        if (buffer[0] == '#') continue;
        sscanf(buffer, "%d", &noVerts);
        elem->facesC = noVerts;
        break;
    }
    if (file.good() && elem->facesC > 0)
    {
        elem->facesP = (xWORD(*)[3]) new xWORD[3*elem->facesC];
        elem->faceListC = 1;
        elem->faceListP = new xFaceList[1];
        elem->faceListP->indexOffset = 0;
        elem->faceListP->indexCount = elem->facesC;
        elem->faceListP->normalP = NULL;
        elem->faceListP->materialId = 0;
        elem->faceListP->materialP = 0;
        elem->faceListP->smooth = 0;

        xWORD (*iter)[3] = elem->facesP;
        xWORD (*end)[3]  = elem->facesP + elem->facesC;

        do
        {
            while (file.good() && file.getline(buffer, 1000))
            {
                if (buffer[0] == '#') continue;
                sscanf(buffer, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", &v1, &v2, &v3, &r, &g, &b, &a, &t);
                (*iter)[0] = v1;
                (*iter)[1] = v2;
                (*iter)[2] = v3;
                break;
            }
        }
        while (++iter != end);

        xElementCalculateSmoothVertices(elem);
    }

    file.close();
    return model;
}
