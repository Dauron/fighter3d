#include "xModel.h"
#include "xUtils.h"
#include <cstdio>
#include <vector>
#include "../../Utils/Debug.h"

#ifdef WIN32
#pragma warning(disable : 4996) // deprecated
#endif

////////////////////// Normals
struct smoothVert {
    xDWORD   smooth;    // smoothing group
    xVector3 normal;    // normal for this sgroup
    xWORD    count;     // count for normal calculation

    xVector3 mnormal;   // merged normal for this sgroup
    xWORD    mcount;    // merged count for normal calculation
    smoothVert *mergedWith;

    xDWORD   vertexId;  // new vertex id
    xWORD3  *flatFace;  // used when smooth == 0
};

void      xElementCalculateSmoothVertices(xElement *elem)
{
    size_t stride = elem->skeletized
        ? elem->textured
        ? sizeof (xVertexTexSkel)
        : sizeof (xVertexSkel)
        : elem->textured
        ? sizeof (xVertexTex)
        : sizeof (xVertex);
    xBYTE *verticesIn = (xBYTE*) elem->verticesP;

    elem->renderData.faceNormalsP = new xVector3[elem->facesC];
    xVector3 *faceNormal = elem->renderData.faceNormalsP;

    //// assign smoothing groups to vertices
    std::vector<smoothVert>::iterator iterF, iterE, iterF2, iterE2;
    std::vector<std::vector<smoothVert> > vertices; // vertex_id->(smooth, normal)[]
    vertices.resize(elem->verticesC);
    xDWORD *smooth = elem->smoothP;
    xWORD3 *faceIn = elem->facesP;
    for (int fP=elem->facesC; fP; --fP, ++smooth, ++faceIn, ++faceNormal)
    {
        xVector3 *v0 = (xVector3*)(verticesIn + (*faceIn)[0]*stride);
        xVector3 *v1 = (xVector3*)(verticesIn + (*faceIn)[1]*stride);
        xVector3 *v2 = (xVector3*)(verticesIn + (*faceIn)[2]*stride);
        *faceNormal = xVector3::CrossProduct(*v1-*v0, *v2-*v0).normalize();

        for (int i=0; i<3; ++i)
        {
            int      idx     = (*faceIn)[i];
            bool     found   = false;
            xVector3 vnormal = *faceNormal;
            xWORD    vcount  = 1;

            if (*smooth)
            {
                iterE = vertices[idx].end();
                for (iterF = vertices[idx].begin(); iterF != iterE; ++iterF)
                    if (iterF->smooth & *smooth)
                    {
                        if (iterF->smooth == *smooth)
                        {
                            iterF->normal += *faceNormal;
                            ++(iterF->count);
                            found = true;
                        }
                        else
                        {
                            vnormal += iterF->normal;
                            vcount  += iterF->count;
                        }
                        iterF->mnormal += *faceNormal;
                        ++(iterF->mcount);
                    }
                if (found) continue;
            }
            vertices[idx].resize(vertices[idx].size()+1);
            iterF = vertices[idx].end()-1;
            iterF->smooth   = *smooth;
            iterF->count    = 1;
            iterF->normal   = *faceNormal;
            iterF->mcount   = vcount;
            iterF->mnormal  = vnormal;
            iterF->flatFace = faceIn;
            iterF->mergedWith = NULL;
        }
    }

    //// smooth normals for original vertex duplicates
    xDWORD verticesC = elem->verticesC;
    std::vector<std::vector<smoothVert> >::iterator vertF = vertices.begin(), vertF2;
    std::vector<std::vector<smoothVert> >::iterator vertE = vertices.end();
    xBYTE *xvertI = verticesIn, *xvertI2;

    for (; vertF != vertE; ++vertF, xvertI += stride)
    {
        if (!vertF->size()) continue;
        xVector3 *v1 = (xVector3*)xvertI;

        xvertI2 = xvertI+1;
        for (vertF2 = vertF+1; vertF2 != vertE; ++vertF2, xvertI2 += stride)
        {
            xVector3 *v2 = (xVector3*)xvertI2;
            if (v1 != v2) continue;

            iterE  = vertF->end();
            iterE2 = vertF2->end();
            for (iterF = vertF->begin(); iterF != iterE; ++iterF)
                for (iterF2 = vertF2->begin(); iterF2 != iterE2; ++iterF2)
                    if (iterF->smooth & iterF2->smooth)
                    {
                        iterF->mnormal  += iterF2->normal;
                        iterF->mcount   += iterF2->count;
                        iterF2->mnormal += iterF->normal;
                        iterF2->mcount  += iterF->count;
                    }
        }
    }

    //// merge smoothing groups, where possible
    for (vertF = vertices.begin(); vertF != vertE; ++vertF)
    {
        if (!vertF->size()) continue;

        iterE = vertF->end();
        for (iterF = vertF->begin(); iterF != iterE; ++iterF)
            if (!iterF->mergedWith)
            {
                xDWORD msmooth = iterF->smooth;
                for (iterF2 = iterF+1; iterF2 != iterE; ++iterF2)
                    if (!iterF2->mergedWith && (msmooth & iterF2->smooth) && (iterF->count == iterF2->count))
                    {
                        msmooth |= iterF2->smooth;
                        iterF2->mergedWith = &*iterF;
                    }
            }
    }

    //// normalize normals and assign ids for new vertex duplicates
    for (vertF = vertices.begin(); vertF != vertE; ++vertF)
    {
        if (!vertF->size()) continue;

        bool used = false;
        iterE = vertF->end();
        for (iterF = vertF->begin(); iterF != iterE; ++iterF)
            if (!iterF->mergedWith)
            {
                iterF->mnormal /= (float)iterF->mcount;
                iterF->mnormal.normalize();
                if (!used)
                {
                    iterF->vertexId = 0; // no change
                    used = true;
                    continue;
                }
                iterF->vertexId = verticesC++;
            }
            else
                iterF->vertexId = iterF->mergedWith->vertexId;
    }

    elem->renderData.verticesC = verticesC;
    if (verticesC == elem->verticesC)
    {
        elem->renderData.verticesP = elem->verticesP;
        elem->renderData.facesP = elem->facesP;
    }
    else
    {
        elem->renderData.verticesP = (xVertex*) new xBYTE[stride*verticesC];
        memcpy(elem->renderData.verticesP, elem->verticesP, stride*elem->verticesC);
        elem->renderData.facesP = new xWORD3[elem->facesC];
        //// fill and correct faces
        smooth  = elem->smoothP;
        faceIn  = elem->facesP;
        xWORD3 *faceOut = elem->renderData.facesP;
        for (int fP=elem->facesC; fP; --fP, ++smooth, ++faceIn, ++faceOut)
            for (int i=0; i<3; ++i)
            {
                int  idx = (*faceOut)[i] = (*faceIn)[i];
                iterE = vertices[idx].end();
                for (iterF = vertices[idx].begin(); iterF != iterE; ++iterF)
                    if (iterF->smooth == *smooth && (*smooth || iterF->flatFace == faceIn))
                    {
                        if (iterF->vertexId) idx = iterF->vertexId;
                        (*faceOut)[i] = idx;
                        break;
                    }
            }
    }

    elem->renderData.normalP   = new xVector3[verticesC];
    //// duplicate vertices and fill normals
    xBYTE    *verticesOut = ((xBYTE*) elem->renderData.verticesP) + stride*elem->verticesC;
    xVector3 *normalP     = elem->renderData.normalP;
    xVector3 *normalP2    = elem->renderData.normalP + elem->verticesC;
    for (vertF = vertices.begin(); vertF != vertE; ++vertF, verticesIn += stride, ++normalP)
    {
        if (!vertF->size()) continue;
        iterE = vertF->end();
        for (iterF = vertF->begin(); iterF != iterE; ++iterF)
            if (!iterF->mergedWith)
                if (iterF->vertexId)
                {
                    memcpy(verticesOut, verticesIn, stride);
                    verticesOut += stride;
                    *(normalP2++) = iterF->mnormal;
                }
                else
                    *normalP = iterF->mnormal;
    }
}


xMaterial *xFaceGetMaterial(const xElement *elem, int faceIdx)
{
    xFaceList *faceL = elem->faceListP;
    for (int i=elem->faceListC; i; --i, ++faceL)
        if (faceIdx - faceL->indexOffset < faceL->indexCount)
            return faceL->materialP;
    return NULL;
}

bool xFaceTransparent(const xElement *elem, int faceIdx)
{
    xMaterial *mat = xFaceGetMaterial(elem, faceIdx);
    return mat && mat->transparency > 0.f;
}

void      xElementFillEdges(xElement *elem)
{
    xWORD edgesC = 3 * (elem->facesC >> 1);
    elem->edgesP = new xEdge[edgesC << 1];

    bool *flags = new bool[elem->facesC*3];
    memset(flags, 0, elem->facesC*3);

    xWORD3 *iterF1 = elem->facesP, *iterF2;
    bool   *iterUsed1 = flags, *iterUsed2;
    xEdge  *iterE = elem->edgesP;

    size_t stride = elem->skeletized
        ? elem->textured
        ? sizeof (xVertexTexSkel)
        : sizeof (xVertexSkel)
        : elem->textured
        ? sizeof (xVertexTex)
        : sizeof (xVertex);
    xBYTE *verticesIn = (xBYTE*) elem->verticesP;

    xWORD cnt = 0;
    for (int i = 0; i < elem->facesC; ++i, ++iterF1, iterUsed1 += 3)
        for (int e1 = 0; e1 < 3; ++e1)
            if (!iterUsed1[e1] && !xFaceTransparent(elem, i))
            {
                ++cnt;
                iterE->vert1 = (*iterF1)[e1];
                iterE->vert2 = (*iterF1)[(e1+1)%3];
                iterE->face1 = i;
                iterE->face2 = xWORD_MAX;
                iterUsed1[e1] = true;
                xVector3 *vc1a = (xVector3 *)(verticesIn + stride* iterE->vert1);
                xVector3 *vc1b = (xVector3 *)(verticesIn + stride* iterE->vert2);

                iterF2    = iterF1 + 1;
                iterUsed2 = iterUsed1 + 3;
                bool found = false;

                for (int j = i+1; j < elem->facesC && !found; ++j, ++iterF2, iterUsed2+=3)
                    for (int e2 = 0; e2 < 3 && !found; ++e2)
                        if (!iterUsed2[e2] && !xFaceTransparent(elem, i))
                        {
                            xWORD v1 = (*iterF2)[e2];
                            xWORD v2 = (*iterF2)[(e2+1)%3];
                            xVector3 *vc2a = (xVector3 *)(verticesIn + stride*v1);
                            xVector3 *vc2b = (xVector3 *)(verticesIn + stride*v2);

                            //if ((vc1a->nearlyEqual(*vc2a) && vc1b->nearlyEqual(*vc2b)) ||
                            //    (vc1a->nearlyEqual(*vc2b) && vc1b->nearlyEqual(*vc2a)) )
                            if ((*vc1a == *vc2a && *vc1b == *vc2b) ||
                                (*vc1a == *vc2b && *vc1b == *vc2a) )
                            {
                                iterE->face2 = j;
                                iterUsed2[e2] = true;
                                found = true;
                                --edgesC;
                            }
                        }
                ++iterE;
            }
    xEdge *tmp = elem->edgesP;
    elem->edgesC = cnt;
    elem->edgesP = new xEdge[cnt];
    memcpy(elem->edgesP, tmp, cnt*sizeof(xEdge));
    delete[] tmp;
}

////////////////////// xMaterial
xMaterial *xMaterialByName(const xFile *file, char *materialName)
{
    xMaterial *mat = file->materialP;
    for (; mat; mat = mat->nextP)
        if (!strcmp(materialName, mat->name))
            break;
    return mat;
}

xMaterial *xMaterialById(const xFile *file, xWORD materialId)
{
    xMaterial *mat = file->materialP;
    for (; mat; mat = mat->nextP)
        if (materialId == mat->id)
            break;
    return mat;
}

void       xMaterialFree(xMaterial *mat)
{
    if (mat->name)
        delete[] mat->name;
    if (mat->texture.name)
        delete[] mat->texture.name;
    delete mat;
}

xMaterial *xMaterialLoad(FILE *file)
{
    xMaterial *mat = new xMaterial();
    if (mat && fread(mat, sizeof(xMaterial), 1, file))
    {
        mat->nextP = NULL;
        mat->texture.htex = 0;
        //mat->ambient = mato->ambient;
        //mat->diffuse = mato->diffuse;
        //mat->specular = mato->specular;
        //mat->name = mato->name;
        //mat->shininess_gloss = mato->shininess_gloss;
        //mat->shininess_level = 0.f;
        //mat->texture.name = mato->texture.name;
        //mat->transparency = mato->transparency;

        if (mat->name)
        {
            size_t len = (size_t) mat->name;
            mat->name = new char[len];
            if (!(mat->name) ||
                fread(mat->name, 1, len, file) != len)
            {
                mat->texture.name = NULL;
                xMaterialFree(mat);
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
                xMaterialFree(mat);
                return NULL;
            }
        }
        return mat;
    }
    return NULL;
}

void       xMaterialSave(FILE *file, xMaterial *mat)
{
    char *name = mat->name;
    char *tname = mat->texture.name;
    if (name)
        mat->name = (char *) strlen(name)+1;
    if (tname)
        mat->texture.name = (char *) strlen(tname)+1;

    fwrite(mat, sizeof(xMaterial), 1, file);
    if (name)
        fwrite(name, 1, (size_t)mat->name, file);
    if (tname)
        fwrite(tname, 1, (size_t)mat->texture.name, file);

    mat->name = name;
    mat->texture.name = tname;
}

bool       xMaterialsLoad(FILE *file, xFile *xfile)
{
    if (!fread(&(xfile->materialC), sizeof(xWORD), 1, file))
        return false;

    xfile->materialP = NULL;
    if (xfile->materialC)
    {
        xMaterial *last = NULL;
        for (int i=0; i < xfile->materialC; ++i)
        {
            if (last)
                last = last->nextP = xMaterialLoad(file);
            else
                last = xfile->materialP = xMaterialLoad(file);
            if (!last)
            {
                xFileFree(xfile);
                return false;
            }
            bool transparent = last->transparency > 0.f;
            xfile->transparent |= transparent;
            xfile->opaque      |= !transparent;
        }
    }
    return true;
}

void       xMaterialsSave(FILE *file, const xFile *xfile)
{
    fwrite(&(xfile->materialC), sizeof(xWORD), 1, file);
    if (xfile->materialC)
    {
        xMaterial *last = xfile->materialP;
        for (; last; last = last->nextP)
            xMaterialSave(file, last);
    }
}

////////////////////// xCollisionData
void xCollisionHierarchyLoad(FILE *file, xElement *elem, xCollisionHierarchy *colH)
{
    fread(&colH->kidsC, sizeof(colH->kidsC), 1, file);
    if (colH->kidsC)
    {
        colH->kidsP = new xCollisionHierarchy[colH->kidsC];
        xCollisionHierarchy *iter = colH->kidsP;
        for (int i=colH->kidsC; i; --i, ++iter)
            xCollisionHierarchyLoad(file, elem, iter);
    }
    else
        colH->kidsP = 0;

    fread(&colH->facesC, sizeof(colH->facesC), 1, file);
    if (colH->facesC && !colH->kidsC)
    {
        colH->facesP = new xWORD3*[colH->facesC];
        xWORD3 **iter = colH->facesP;
        xDWORD idx;
        for (int i=colH->facesC; i; --i, ++iter)
        {
            fread(&idx, sizeof(idx), 1, file);
            *iter = elem->facesP + idx;
        }
    }
    else
        colH->facesP = 0;

//    colH->verticesC = 0;
//    colH->verticesP = 0;

    fread(&colH->verticesC, sizeof(colH->verticesC), 1, file);
    if (colH->verticesC && !colH->kidsC)
    {
        colH->verticesP = new xDWORD[colH->verticesC];
        fread(colH->verticesP, sizeof(xDWORD), colH->verticesC, file);
    }
    else
        colH->verticesP = 0;
}

void xCollisionHierarchySave(FILE *file, xElement *elem, xCollisionHierarchy *colH)
{
    fwrite(&colH->kidsC, sizeof(colH->kidsC), 1, file);
    if (colH->kidsC)
    {
        xCollisionHierarchy *iter = colH->kidsP;
        for (int i=colH->kidsC; i; --i, ++iter)
            xCollisionHierarchySave(file, elem, iter);
    }
    else
        colH->kidsP = 0;

    fwrite(&colH->facesC, sizeof(colH->facesC), 1, file);
    if (colH->facesC && !colH->kidsC && colH->facesP)
    {
        xWORD3 **iter = colH->facesP;
        xDWORD idx;
        for (int i=colH->facesC; i; --i, ++iter)
        {
            idx = *iter - elem->facesP;
            fwrite(&idx, sizeof(idx), 1, file);
        }
    }

    fwrite(&colH->verticesC, sizeof(colH->verticesC), 1, file);
    if (colH->verticesC && !colH->kidsC && colH->verticesP)
        fwrite(colH->verticesP, sizeof(xDWORD), colH->verticesC, file);
}

void xCollisionDataLoad(FILE *file, xElement *elem)
{
    fread(&elem->collisionData.kidsC, sizeof(elem->collisionData.kidsC), 1, file);
    if (elem->collisionData.kidsC)
    {
        elem->collisionData.kidsP = new xCollisionHierarchy[elem->collisionData.kidsC];
        xCollisionHierarchy *iter = elem->collisionData.kidsP;
        for (int i = elem->collisionData.kidsC; i; --i, ++iter)
            xCollisionHierarchyLoad(file, elem, iter);
    }
    else
        elem->collisionData.kidsP = NULL;
}

void xCollisionDataSave(FILE *file, xElement *elem)
{
    fwrite(&elem->collisionData.kidsC, sizeof(elem->collisionData.kidsC), 1, file);
    if (elem->collisionData.kidsC)
    {
        xCollisionHierarchy *iter = elem->collisionData.kidsP;
        for (int i = elem->collisionData.kidsC; i; --i, ++iter)
            xCollisionHierarchySave(file, elem, iter);
    }
}

void xCollisionInfo_Fill(xFile *xfile, xElement *elem)
{
    if (elem->kidsC)
    {
        xElement *last = elem->kidsP;
        for (; last; last = last->nextP)
            xCollisionInfo_Fill(xfile, last);
    }
    //if (elem->collisionData.kidsP) { // force Octree recalculation
    //    xElement_FreeCollisionHierarchy(elem->collisionData.kidsP, elem->collisionData.kidsC);
    //    elem->collisionData.kidsP = NULL;
    //}
    if (elem->collisionData.kidsP == NULL)
        xElement_GetCollisionHierarchy(xfile, elem);
}

////////////////////// xElement
xElement * _xElementById (xElement *elem, xWORD elementId, xWORD &currElemID)
{
    xElement *relem;
    for (xElement *selem = elem->kidsP; selem; selem = selem->nextP)
    {
        if ( ++currElemID == elementId )
            return selem;
        if ( (relem = _xElementById(selem, elementId, currElemID)) )
            return relem;
    }
    return NULL;
}

xElement * xElementById  (const xFile* model, xWORD elementId)
{
    xWORD     currElemID = xWORD_MAX;
    xElement *relem;
    for (xElement *selem = model->firstP; selem; selem = selem->nextP)
    {
        if ( ++currElemID == elementId )
            return selem;
        if ( (relem = _xElementById(selem, elementId, currElemID)) )
            return relem;
    }
    return NULL;
}
xWORD      _xElementCountKids (xElement *elem)
{
    xDWORD res = elem->kidsC;
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        res += _xElementCountKids(celem);
    return res;
}

xWORD      xElementCount (const xFile* model)
{
    xWORD res = 0;
    for (xElement *elem = model->firstP; elem; elem = elem->nextP)
        res += _xElementCountKids(elem) + 1;
    return res;
}

void      xElementFree(xElement *elem)
{
    if (elem->renderData.facesP && elem->renderData.facesP != elem->facesP)
        delete[] elem->renderData.facesP;
    if (elem->renderData.verticesP && elem->renderData.verticesP != elem->verticesP)
        delete[] elem->renderData.verticesP;
    if (elem->renderData.normalP)
        delete[] elem->renderData.normalP;
    if (elem->renderData.faceNormalsP)
        delete[] elem->renderData.faceNormalsP;
    if (elem->name)
        delete[] elem->name;
    if (elem->verticesP) {
        if (elem->textured && elem->skeletized)
            delete[] elem->verticesTSP;
        else
        if (elem->textured)
            delete[] elem->verticesTP;
        else
        if (elem->skeletized)
            delete[] elem->verticesSP;
        else
            delete[] elem->verticesP;
    }
    if (elem->facesP)
        delete[] elem->facesP;
    if (elem->smoothP)
        delete[] elem->smoothP;
    if (elem->faceListP)
        delete[] elem->faceListP;
    if (elem->edgesP)
        delete[] elem->edgesP;
    xElement *curr = elem->kidsP, *next;
    while (curr)
    {
        next = curr->nextP;
        xElementFree(curr);
        curr = next;
    }

    if (elem->collisionData.kidsP)
        xElement_FreeCollisionHierarchy(&elem->collisionData);

    delete elem;
}

#ifndef FILE_CURRENT
#define FILE_CURRENT 1
#endif

xElement *xElementLoad(FILE *file, xFile *xfile)
{
    xElement *elem = new xElement();
    if (!elem) return NULL;

    int i = 128;
    i -= fread(&elem->id,     sizeof(elem->id), 1, file) * sizeof(elem->id);
    i -= 2; fseek(file, 2, FILE_CURRENT);

    i -= fread(&elem->name,   sizeof(elem->name), 1, file) * sizeof(elem->name);
    i -= fread(&elem->color,  sizeof(elem->color), 1, file) * sizeof(elem->color);
    i -= fread(&elem->matrix, sizeof(elem->matrix), 1, file) * sizeof(elem->matrix);

    i -= fread(&elem->verticesP, sizeof(elem->verticesP), 1, file) * sizeof(elem->verticesP); // irrelevant
    i -= fread(&elem->verticesC, sizeof(elem->verticesC), 1, file) * sizeof(elem->verticesC);


    i -= 10; fseek(file, 8+2, FILE_CURRENT);
    i -= fread(&elem->facesC,    sizeof(elem->facesC), 1, file) * sizeof(elem->facesC);
    i -= 6; fseek(file, 4+2, FILE_CURRENT);
    i -= fread(&elem->faceListC, sizeof(elem->faceListC), 1, file) * sizeof(elem->faceListC);

    //xWORD2     *edgesP;

    i -= fread(&elem->textured,   sizeof(elem->textured), 1, file) * sizeof(elem->textured);
    i -= fread(&elem->skeletized, sizeof(elem->skeletized), 1, file) * sizeof(elem->skeletized);

    i -= 8; fseek(file, 8, FILE_CURRENT);
    i -= fread(&elem->kidsC,   sizeof(elem->kidsC), 1, file) * sizeof(elem->kidsC);
    i -= 2; fseek(file, 2, FILE_CURRENT);

    //xCollisionData    collisionData; // loaded later on
    //xRenderData       renderData;

    memset(&(elem->renderData),    0, sizeof(elem->renderData));
    memset(&(elem->collisionData), 0, sizeof(elem->collisionData));

    elem->verticesP = NULL;
    elem->smoothP   = NULL;
    elem->facesP    = NULL;
    elem->faceListP = NULL;
    elem->edgesP    = NULL;
    elem->edgesC    = 0;
    elem->kidsP = NULL;
    elem->nextP = NULL;

    if (elem->name)
    {
        size_t len = (size_t) elem->name;
        elem->name = new char[len];
        if (!(elem->name) ||
            fread(elem->name, 1, len, file) != len)
        {
            xElementFree(elem);
            return NULL;
        }
    }
    if (elem->verticesC)
    {
        size_t stride = elem->skeletized
            ? elem->textured
            ? sizeof (xVertexTexSkel)
            : sizeof (xVertexSkel)
            : elem->textured
            ? sizeof (xVertexTex)
            : sizeof (xVertex);
        elem->verticesP = (xVertex*) new xBYTE[stride*elem->verticesC];
        if (!elem->verticesTSP ||
                fread(elem->verticesP, stride, elem->verticesC, file)
                    != elem->verticesC)
        {
            xElementFree(elem);
            return NULL;
        }
    }
    if (elem->facesC)
    {
        elem->facesP = (xWORD(*)[3]) new xWORD[3*elem->facesC];
        if (!elem->facesP ||
            fread(elem->facesP, 3*sizeof(xWORD), elem->facesC, file)
                != elem->facesC)
        {
            xElementFree(elem);
            return NULL;
        }
        elem->smoothP = new xDWORD[elem->facesC];
        if (!elem->smoothP ||
            fread(elem->smoothP, sizeof(xDWORD), elem->facesC, file)
                != elem->facesC)
        {
            xElementFree(elem);
            return NULL;
        }
    }
    if (elem->faceListC)
    {
        elem->faceListP = new xFaceList[elem->faceListC];
        if (!elem->faceListP ||
            fread(elem->faceListP, sizeof(xFaceList), elem->faceListC, file)
                != elem->faceListC)
        {
            xElementFree(elem);
            return NULL;
        }

        xFaceList *faceL = elem->faceListP;
        for (int fL=elem->faceListC; fL; --fL, ++faceL)
        {
            faceL->materialP = xMaterialById(xfile, faceL->materialId);
            if (faceL->materialP)
            {
                bool transparent = faceL->materialP->transparency > 0.f;
                elem->renderData.transparent |= transparent;
                elem->renderData.opaque      |= !transparent;
            }
            else
                xfile->opaque = elem->renderData.opaque = true;
        }
        xElementCalculateSmoothVertices(elem);
    }
    xElementFillEdges(elem);
    if (elem->kidsC)
    {
        xElement *last = NULL;
        for (int i=0; i < elem->kidsC; ++i)
        {
            if (last)
                last = last->nextP = xElementLoad(file, xfile);
            else
                last = elem->kidsP = xElementLoad(file, xfile);
            if (!last)
            {
                xElementFree(elem);
                return NULL;
            }
        }
    }

    if (xfile->saveCollisionData)
        xCollisionDataLoad(file, elem);

    return elem;
}

void      xElementSave(FILE *file, const xFile *xfile, xElement *elem)
{
    if (!elem) return;

    char *name = elem->name;
    if (name)
        elem->name = (char *) strlen(name)+1;

    char buff[255];

    int i = 128;
    i -= fwrite(&elem->id,     sizeof(elem->id), 1, file) * sizeof(elem->id);
    i -= fwrite(buff, 1, 2, file);

    i -= fwrite(&elem->name,   sizeof(elem->name), 1, file) * sizeof(elem->name);
    i -= fwrite(&elem->color,  sizeof(elem->color), 1, file) * sizeof(elem->color);
    i -= fwrite(&elem->matrix, sizeof(elem->matrix), 1, file) * sizeof(elem->matrix);

    i -= fwrite(&elem->verticesP, sizeof(elem->verticesP), 1, file) * sizeof(elem->verticesP); // irrelevant
    i -= fwrite(&elem->verticesC, sizeof(elem->verticesC), 1, file) * sizeof(elem->verticesC);


    i -= fwrite(&elem->smoothP,   sizeof(elem->smoothP), 1, file) * sizeof(elem->smoothP); // irrelevant
    i -= fwrite(&elem->facesP,    sizeof(elem->facesP), 1, file) * sizeof(elem->facesP);  // irrelevant
    i -= fwrite(buff, 1, 2, file);
    i -= fwrite(&elem->facesC,    sizeof(elem->facesC), 1, file) * sizeof(elem->facesC);
    i -= fwrite(&elem->faceListP, sizeof(elem->facesP), 1, file) * sizeof(elem->facesP);  // irrelevant
    i -= fwrite(buff, 1, 2, file);
    i -= fwrite(&elem->faceListC, sizeof(elem->faceListC), 1, file) * sizeof(elem->faceListC);

    //xWORD2     *edgesP;

    i -= fwrite(&elem->textured,   sizeof(elem->textured), 1, file) * sizeof(elem->textured);
    i -= fwrite(&elem->skeletized, sizeof(elem->skeletized), 1, file) * sizeof(elem->skeletized);

    i -= fwrite(&elem->nextP,   sizeof(elem->nextP), 1, file) * sizeof(elem->nextP); // irrelevant
    i -= fwrite(&elem->kidsP,   sizeof(elem->kidsP), 1, file) * sizeof(elem->kidsP); // irrelevant
    i -= fwrite(&elem->kidsC,   sizeof(elem->kidsC), 1, file) * sizeof(elem->kidsC);
    i -= fwrite(buff, 1, 2, file);

    //xCollisionData    collisionData; // saved later on
    //xRenderData       renderData;

    if (name)
        fwrite(name, 1, (size_t)elem->name, file);
    if (elem->verticesC) {
        size_t stride = elem->skeletized
                ? elem->textured
                ? sizeof (xVertexTexSkel)
                : sizeof (xVertexSkel)
                : elem->textured
                ? sizeof (xVertexTex)
                : sizeof (xVertex);
        fwrite(elem->verticesP, stride, elem->verticesC, file);
    }
    if (elem->facesC)
    {
        fwrite(elem->facesP, 3*sizeof(xWORD), elem->facesC, file);
        fwrite(elem->smoothP, sizeof(xDWORD), elem->facesC, file);
    }
    if (elem->faceListC)
        fwrite(elem->faceListP, sizeof(xFaceList), elem->faceListC, file);
    if (elem->kidsC)
    {
        xElement *last = elem->kidsP;
        for (; last; last = last->nextP)
            xElementSave(file, xfile, last);
    }

    elem->name = name;

    if (xfile->saveCollisionData)
        xCollisionDataSave(file, elem);
}

////////////////////// xBone
void   xBoneFree(xBone *bone)
{
    if (!bone) return;
    if (bone->name)
        delete[] bone->name;
    xBone *curr = bone->kidsP, *next;
    while (curr)
    {
        next = curr->nextP;
        xBoneFree(curr);
        curr = next;
    }
    delete bone;
}

void   xBoneCopy(const xBone *boneSrc, xBone *&boneDst)
{
    boneDst = new xBone();
    if (boneDst)
    {
        boneDst->ending = boneSrc->ending;
        boneDst->id     = boneSrc->id;
        boneDst->kidsC  = boneSrc->kidsC;
        if (boneSrc->name)
            boneDst->name = strdup(boneSrc->name);
        else
            boneDst->name = NULL;
        boneDst->quaternion = boneSrc->quaternion;
        boneDst->kidsP = NULL;
        boneDst->nextP = NULL;

        if (boneDst->kidsC)
        {
            xBone *iter = boneSrc->kidsP;
            xBone *last = NULL;
            for (int i=boneDst->kidsC; i ; --i)
            {
                if (last) {
                    xBoneCopy(iter, last->nextP);
                    last = last->nextP;
                }
                else {
                    xBoneCopy(iter, boneDst->kidsP);
                    last = boneDst->kidsP;
                }
                iter = iter->nextP;
                if (!last)
                {
                    xBoneFree(boneDst);
                    boneDst = NULL;
                }
            }
        }
    }
}

xBone *xBoneLoad(FILE *file, xFile *xfile)
{
    xBone *bone = new xBone();
    if (bone && fread(bone, sizeof(xBone), 1, file))
    {
        bone->kidsP = NULL;
        bone->nextP = NULL;

        if (bone->name)
        {
            size_t len = (size_t) bone->name;
            bone->name = new char[len];
            if (!(bone->name) ||
                fread(bone->name, 1, len, file) != len)
            {
                xBoneFree(bone);
                return NULL;
            }
        }
        if (bone->kidsC)
        {
            xBone *last = NULL;
            for (int i=0; i < bone->kidsC; ++i)
            {
                if (last)
                    last = last->nextP = xBoneLoad(file, xfile);
                else
                    last = bone->kidsP = xBoneLoad(file, xfile);
                if (!last)
                {
                    xBoneFree(bone);
                    return NULL;
                }
            }
        }
        return bone;
    }
    return NULL;
}

void   xBoneSave(FILE *file, xBone *bone)
{
    char *name = bone->name;
    if (name)
        bone->name = (char *) strlen(name)+1;

    int i = fwrite(bone, sizeof(xBone), 1, file);
    if (name)
        i = fwrite(name, 1, (size_t)bone->name, file);
    if (bone->kidsC)
    {
        xBone *last = bone->kidsP;
        for (; last; last = last->nextP)
            xBoneSave(file, last);
    }

    bone->name = name;
}

////////////////////// xFile
void   xFileFree(xFile *xfile)
{
    if (!xfile) return;

    if (xfile->fileName)
        delete[] xfile->fileName;
    xMaterial *curr = xfile->materialP, *next;
    while (curr)
    {
        next = curr->nextP;
        xMaterialFree(curr);
        curr = next;
    }
    if (xfile->firstP)
        xElementFree(xfile->firstP);
    if (xfile->spineP)
        xBoneFree(xfile->spineP);
    delete xfile;
}


xFile *xFileLoad(const char *fileName, bool createCollisionInfo)
{
    FILE *file;
    file = fopen(fileName, "rb");
    if (file)
    {
        xFile *xfile = new xFile();
        xfile->fileName = strdup(fileName);
        if (xfile)
        {
            xfile->texturesInited = false;
            xfile->transparent = false;
            xfile->opaque = false;

            xDWORD len;
            fread(&len, sizeof(len), 1, file);
            fread(&xfile->saveCollisionData, sizeof(xfile->saveCollisionData), 1, file);

            xfile->materialP = NULL;
            xfile->firstP = NULL;

            if (!xMaterialsLoad(file, xfile))
            {
                xFileFree(xfile);
                fclose(file);
                return NULL;
            }

            xfile->firstP = xElementLoad(file, xfile);
            if (!xfile->firstP)
            {
                xFileFree(xfile);
                fclose(file);
                return NULL;
            }
            xfile->elementC = xElementCount(xfile);

            bool skeletized;
            fread(&skeletized, sizeof(bool), 1, file);
            if (skeletized) {
                xfile->spineP = xBoneLoad(file, xfile);
            }

            if (createCollisionInfo)
                xCollisionInfo_Fill(xfile, xfile->firstP);
        }
        fclose(file);
        return xfile;
    }
    return NULL;
}

void   xFileSave(const xFile *xfile)
{
    FILE *file;
    file = fopen(xfile->fileName, "wb");
    if (file)
    {
        xDWORD len = sizeof(xfile->saveCollisionData);
        fwrite(&len, sizeof(len), 1, file);
        fwrite(&xfile->saveCollisionData, sizeof(xfile->saveCollisionData), 1, file);

        xMaterialsSave(file, xfile);
        xElementSave(file, xfile, xfile->firstP);

        // are the bones defined?
        bool skeletized = xfile->spineP != NULL;
        fwrite(&skeletized, sizeof(bool), 1, file);
        if (skeletized)
            xBoneSave(file, xfile->spineP);

        fclose(file);
    }
}
