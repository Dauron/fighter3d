#include "xElement.h"

#include <vector>

////////////////////// Normals
struct smoothVert {
    xDWORD   smooth;    // smoothing group
    xVector3 normal;    // normal for this sgroup
    xWORD    count;     // count for normal calculation

    xVector3 mnormal;   // merged normal for this sgroup
    xWORD    mcount;    // merged count for normal calculation
    smoothVert *mergedWith;

    xDWORD   vertexId;  // new vertex id
    xFace  *flatFace;  // used when smooth == 0
};

void xElement :: CalculateSmoothVertices ()
{
    size_t stride = this->skeletized
        ? this->textured
        ? sizeof (xVertexTexSkel)
        : sizeof (xVertexSkel)
        : this->textured
        ? sizeof (xVertexTex)
        : sizeof (xVertex);
    xBYTE *verticesIn = (xBYTE*) this->verticesP;

    this->renderData.faceNormalsP = new xVector3[this->facesC];
    xVector3 *faceNormal = this->renderData.faceNormalsP;

    //// assign smoothing groups to vertices
    std::vector<smoothVert>::iterator iterF, iterE, iterF2, iterE2;
    std::vector<std::vector<smoothVert> > vertices; // vertex_id->(smooth, normal)[]
    vertices.resize(this->verticesC);
    xDWORD *smooth = this->smoothP;
    xFace *faceIn  = this->facesP;
    for (int fP=this->facesC; fP; --fP, ++smooth, ++faceIn, ++faceNormal)
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
    xDWORD verticesC = this->verticesC;
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

    this->renderData.verticesC = verticesC;
    if (verticesC == this->verticesC)
    {
        this->renderData.verticesP = this->verticesP;
        this->renderData.facesP    = this->facesP;
    }
    else
    {
        this->renderData.verticesP = (xVertex*) new xBYTE[stride*verticesC];
        memcpy(this->renderData.verticesP, this->verticesP, stride*this->verticesC);
        this->renderData.facesP = new xFace[this->facesC];
        //// fill and correct faces
        smooth  = this->smoothP;
        faceIn  = this->facesP;
        xFace *faceOut = this->renderData.facesP;
        for (int fP=this->facesC; fP; --fP, ++smooth, ++faceIn, ++faceOut)
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

    this->renderData.normalP   = new xVector3[verticesC];
    //// duplicate vertices and fill normals
    xBYTE    *verticesOut = ((xBYTE*) this->renderData.verticesP) + stride*this->verticesC;
    xVector3 *normalP     = this->renderData.normalP;
    xVector3 *normalP2    = this->renderData.normalP + this->verticesC;
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

////////////////////// Shadow Edges
xMaterial *xFaceGetMaterial(const xElement *elem, int faceIdx)
{
    xFaceList *faceL = elem->faceListP;
    for (int i=elem->faceListC; i; --i, ++faceL)
        if (faceIdx - faceL->indexOffset < faceL->indexCount)
            return faceL->materialP;
    return NULL;
}

bool       xFaceTransparent(const xElement *elem, int faceIdx)
{
    xMaterial *mat = xFaceGetMaterial(elem, faceIdx);
    return mat && mat->transparency > 0.f;
}
bool       xFaceTransparentOr2Sided(const xElement *elem, int faceIdx)
{
    xMaterial *mat = xFaceGetMaterial(elem, faceIdx);
    return mat && (mat->transparency > 0.f || mat->two_sided);
}


void xElement :: FillShadowEdges ()
{
    xWORD edgesC = 3 * (this->facesC);
    this->edgesP = new xEdge[edgesC << 1];

    bool *flags = new bool[this->facesC*3];
    memset(flags, 0, this->facesC*3);

    xFace *iterF1 = this->facesP, *iterF2;
    bool  *iterUsed1 = flags, *iterUsed2;
    xEdge *iterE = this->edgesP;

    size_t stride = this->skeletized
        ? this->textured
        ? sizeof (xVertexTexSkel)
        : sizeof (xVertexSkel)
        : this->textured
        ? sizeof (xVertexTex)
        : sizeof (xVertex);
    xBYTE *verticesIn = (xBYTE*) this->verticesP;

    xWORD cnt = 0;
    for (int i = 0; i < this->facesC; ++i, ++iterF1, iterUsed1 += 3)
        for (int e1 = 0; e1 < 3; ++e1)
            if (!iterUsed1[e1] && !xFaceTransparentOr2Sided(this, i))
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

                for (int j = i+1; j < this->facesC && !found; ++j, ++iterF2, iterUsed2+=3)
                    for (int e2 = 0; e2 < 3 && !found; ++e2)
                        if (!iterUsed2[e2] && !xFaceTransparentOr2Sided(this, i))
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
    xEdge *tmp = this->edgesP;
    this->edgesC = cnt;
    this->edgesP = new xEdge[cnt];
    memcpy(this->edgesP, tmp, cnt*sizeof(xEdge));
    delete[] tmp;
}