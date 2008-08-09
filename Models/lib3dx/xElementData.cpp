#include "xModel.h"

////////////////////// xCollisionHierarchy
void xCollisionHierarchy :: Load ( FILE *file, xElement *elem )
{
    xCollisionData::Load(file, elem);

    fread(&this->facesC, sizeof(this->facesC), 1, file);
    if (this->facesC && !this->kidsC)
    {
        this->facesP = new xFace*[this->facesC];
        xFace **iter = this->facesP;
        xDWORD idx;
        for (int i=this->facesC; i; --i, ++iter)
        {
            fread(&idx, sizeof(idx), 1, file);
            *iter = elem->facesP + idx;
        }
    }
    else
        this->facesP = 0;

    fread(&this->verticesC, sizeof(xWORD), 1, file);
    if (this->verticesC && !this->kidsC)
    {
        this->verticesP = new xWORD[this->verticesC];
        fread(this->verticesP, sizeof(xWORD), this->verticesC, file);
    }
    else
        this->verticesP = 0;
}

void xCollisionHierarchy :: Save ( FILE *file, xElement *elem )
{
    xCollisionData::Save(file, elem);

    fwrite(&this->facesC, sizeof(this->facesC), 1, file);
    if (this->facesC && !this->kidsC && this->facesP)
    {
        xFace **iter = this->facesP;
        xDWORD idx;
        for (int i=this->facesC; i; --i, ++iter)
        {
            idx = *iter - elem->facesP;
            fwrite(&idx, sizeof(idx), 1, file);
        }
    }

    fwrite(&this->verticesC, sizeof(this->verticesC), 1, file);
    if (this->verticesC && !this->kidsC && this->verticesP)
        fwrite(this->verticesP, sizeof(xWORD), this->verticesC, file);
}

////////////////////// xCollisionData

void xCollisionData :: Load (FILE *file, xElement *elem)
{
    fread(&this->kidsC, sizeof(this->kidsC), 1, file);
    if (this->kidsC)
    {
        this->kidsP = new xCollisionHierarchy[this->kidsC];
        xCollisionHierarchy *iter = this->kidsP;
        for (int i = this->kidsC; i; --i, ++iter)
            iter->Load(file, elem);
    }
    else
        this->kidsP = NULL;
}

void xCollisionData :: Save (FILE *file, xElement *elem)
{
    fwrite(&this->kidsC, sizeof(this->kidsC), 1, file);
    if (this->kidsC)
    {
        xCollisionHierarchy *iter = this->kidsP;
        for (int i = this->kidsC; i; --i, ++iter)
            iter->Save(file, elem);
    }
}

void xCollisionData :: FreeKids()
{
    if (this->kidsP)
    {
        xCollisionHierarchy *ch = this->kidsP;
        for (int i = this->kidsC; i; --i)
        {
            if (ch->facesP)    delete[] ch->facesP;
            if (ch->verticesP) delete[] ch->verticesP;
            ch->FreeKids();
        }
        delete[] this->kidsP;
        this->kidsP = NULL;
        this->kidsC = 0;
    }
}

#include <vector>
#include <algorithm>

void CreateHierarchyFromVertices(const xElement                   *elem,
                                 xCollisionHierarchy              *baseHierarchy,
                                 std::vector<xBoxA>               &cBoundings,
                                 std::vector<xCollisionHierarchy> &cHierarchy);

void xCollisionData :: Fill (xModel *xmodel, xElement *elem)
{
    //if (this->kidsP) // force Octree recalculation
    //    this->FreeKids();
    if (this->kidsP == NULL)
    {
        if (!elem->verticesC)
        {
            this->kidsP = NULL;
            this->kidsC = 0;
            return;
        }
        if (elem->facesC < 20)
        {
            this->kidsP = new xCollisionHierarchy[1];
            this->kidsC = 1;
            this->kidsP->facesC = elem->facesC;
            this->kidsP->facesP = new xFace*[elem->facesC];
            this->kidsP->verticesC = elem->verticesC;
            this->kidsP->verticesP = new xWORD[elem->verticesC];
            this->kidsP->kidsC = 0;
            this->kidsP->kidsP = NULL;

            xFace **iterHF = this->kidsP->facesP;
            xFace  *iterF  = elem->facesP;
            for (int i = elem->facesC; i; --i, ++iterHF, ++iterF)
                *iterHF = iterF;

            xWORD *iterHV = this->kidsP->verticesP;
            for (xWORD i = 0; i < elem->verticesC; ++i, ++iterHV)
                *iterHV = i;
            return;
        }

        xBYTE *src    = (xBYTE *) elem->verticesP;
        xDWORD stride = elem->skeletized
            ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
            : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));
        std::vector<xCollisionHierarchy> cHierarchy;

        if (!elem->skeletized || !xmodel->spine.I_bones)
        {
            xCollisionHierarchy hierarchy;
            hierarchy.kidsC = 0;
            hierarchy.kidsP = NULL;
            hierarchy.facesC    = elem->facesC;
            hierarchy.verticesC = elem->verticesC;
            xFace **iterHF = hierarchy.facesP    = new xFace*[elem->facesC];
            xWORD   *iterHV = hierarchy.verticesP = new xWORD[elem->verticesC];
            xFace  *iterF  = elem->facesP;

            xBoxA bounding;
            bounding.P_max.x = -1000000000.f; bounding.P_max.y = -1000000000.f; bounding.P_max.z = -1000000000.f;
            bounding.P_min.x =  1000000000.f; bounding.P_min.y =  1000000000.f; bounding.P_min.z =  1000000000.f;

            for (int i = elem->facesC; i; --i, ++iterHF, ++iterF)
            {
                *iterHF = iterF;

                const xVector3 *v1 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[0]))->pos;
                const xVector3 *v2 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[1]))->pos;
                const xVector3 *v3 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[2]))->pos;

                float minx = min(v1->x, min(v2->x, v3->x));
                float miny = min(v1->y, min(v2->y, v3->y));
                float minz = min(v1->z, min(v2->z, v3->z));
                float maxx = max(v1->x, max(v2->x, v3->x));
                float maxy = max(v1->y, max(v2->y, v3->y));
                float maxz = max(v1->z, max(v2->z, v3->z));

                if (minx < bounding.P_min.x) bounding.P_min.x = minx;
                if (miny < bounding.P_min.y) bounding.P_min.y = miny;
                if (minz < bounding.P_min.z) bounding.P_min.z = minz;
                if (maxx > bounding.P_max.x) bounding.P_max.x = maxx;
                if (maxy > bounding.P_max.y) bounding.P_max.y = maxy;
                if (maxz > bounding.P_max.z) bounding.P_max.z = maxz;
            }

            for (xDWORD i = 0; i < elem->verticesC; ++i, ++iterHV)
                *iterHV = i;

            hierarchy.Subdivide(elem, 5.f, 1, bounding);
            cHierarchy.push_back(hierarchy);
        }
        else
        {
            this->kidsC = xmodel->spine.I_bones;

            std::vector<xBoxA> cBoundings;
            cBoundings.resize(this->kidsC);

            xBYTE *iterV = src;
            for (xDWORD i = elem->verticesC; i; --i, iterV += stride)
            {
                xVertexSkel *vert = (xVertexSkel *) iterV;

                int   bone   = (int) floor(vert->b0);
                float weight = (vert->b0 - bone)*10;

                int   bi = (int) floor(vert->b1);
                float bw = (vert->b1 - bi)*10;
                if (bw > weight) { bone = bi; weight = bw; }

                bi = (int) floor(vert->b2);
                bw = (vert->b2 - bi)*10;
                if (bw > weight) { bone = bi; weight = bw; }

                bi = (int) floor(vert->b3);
                bw = (vert->b3 - bi)*10;
                if (bw > weight) { bone = bi; weight = bw; }

                xBoxA &bounding = cBoundings[bone];
                if (vert->x < bounding.P_min.x) bounding.P_min.x = vert->x;
                if (vert->y < bounding.P_min.y) bounding.P_min.y = vert->y;
                if (vert->z < bounding.P_min.z) bounding.P_min.z = vert->z;
                if (vert->x > bounding.P_max.x) bounding.P_max.x = vert->x;
                if (vert->y > bounding.P_max.y) bounding.P_max.y = vert->y;
                if (vert->z > bounding.P_max.z) bounding.P_max.z = vert->z;
            }

            CreateHierarchyFromVertices(elem, NULL, cBoundings, cHierarchy);

            for (size_t i = 0; i<cHierarchy.size(); ++i)
                if (cHierarchy[i].facesC > 20 && cHierarchy[i].kidsC == 0)
                {
                    float scale = cHierarchy[i].facesC / 20.f;
                    if (scale < 2.0f) scale = 2.0f;
                    if (scale > 10.f) scale = 10.f;
                    cHierarchy[i].Subdivide(elem, scale, 1, cBoundings[i]);
                }
        }

        for (size_t i = 0; i<cHierarchy.size(); ++i)
            if (cHierarchy[i].facesC == 0)
            {
                cHierarchy[i--] = *cHierarchy.rbegin();
                cHierarchy.resize(cHierarchy.size()-1);
            }
        this->kidsC = cHierarchy.size();
        this->kidsP = new xCollisionHierarchy[cHierarchy.size()];
        for (size_t i = 0; i<cHierarchy.size(); ++i)
            this->kidsP[i] = cHierarchy[i];
    }
}

void xCollisionHierarchy :: Subdivide(const xElement *elem, float scale, int depth, const xBoxA &bounding)
{
    float dx = bounding.P_max.x - bounding.P_min.x;
    float dy = bounding.P_max.y - bounding.P_min.y;
    float dz = bounding.P_max.z - bounding.P_min.z;
    float d  = max (dx, max(dy, dz));
    float w  = d / scale;
    float wx = min (w, dx);
    float wy = min (w, dy);
    float wz = min (w, dz);
    int   cx = (int)ceil(dx / wx);
    int   cy = (int)ceil(dy / wy);
    int   cz = (int)ceil(dz / wz);

    std::vector<xBoxA> cBoundings;
    cBoundings.resize(cx*cy*cz);

    for (int ix = 0; ix < cx; ++ix)
        for (int iy = 0; iy < cy; ++iy)
            for (int iz = 0; iz < cz; ++iz)
            {
                int hrch = ix + cx * (iy + cy * iz);
                xBoxA &cBounding = cBoundings[hrch];
                cBounding.P_min.x = ix * wx + bounding.P_min.x;
                cBounding.P_min.y = iy * wy + bounding.P_min.y;
                cBounding.P_min.z = iz * wz + bounding.P_min.z;
                cBounding.P_max.x = (ix+1 != cx) ? (ix+1) * wx + bounding.P_min.x : bounding.P_max.x;
                cBounding.P_max.y = (iy+1 != cy) ? (iy+1) * wy + bounding.P_min.y : bounding.P_max.y;
                cBounding.P_max.z = (iz+1 != cz) ? (iz+1) * wz + bounding.P_min.z : bounding.P_max.z;
            }

    std::vector<xCollisionHierarchy> cHierarchy;
    CreateHierarchyFromVertices(elem, this, cBoundings, cHierarchy);

    if (depth < MAX_HIERARCHY_DEPTH)
        for (size_t i = 0; i<cHierarchy.size(); ++i)
            if (cHierarchy[i].facesC > 20)
            {
                float scale = cHierarchy[i].facesC / 20.f;
                if (scale < 2.f)  scale = 2.f;
                if (scale > 10.f) scale = 10.f;
                cHierarchy[i].Subdivide(elem, scale, depth+1, cBoundings[i]);
            }

    for (size_t i = 0; i<cHierarchy.size(); ++i)
        if (cHierarchy[i].facesC == 0)
        {
            cHierarchy[i--] = *cHierarchy.rbegin();
            cHierarchy.resize(cHierarchy.size()-1);
        }

    if (cHierarchy.empty())
        return;

    delete[] this->facesP;
    this->facesP = NULL;
    delete[] this->verticesP;
    this->verticesP = NULL;
    if (cHierarchy.size() == 1)
    {
        this->verticesP = cHierarchy[0].verticesP;
        this->facesP = cHierarchy[0].facesP;
        this->kidsC  = cHierarchy[0].kidsC;
        this->kidsP  = cHierarchy[0].kidsP;
    }
    else
    {
        this->kidsC = cHierarchy.size();
        this->kidsP = new xCollisionHierarchy[cHierarchy.size()];
        for (size_t i = 0; i<cHierarchy.size(); ++i)
            this->kidsP[i] = cHierarchy[i];
    }
}

void CreateHierarchyFromVertices(const xElement                   *elem,
                                 xCollisionHierarchy              *baseHierarchy,
                                 std::vector<xBoxA>               &cBoundings,
                                 std::vector<xCollisionHierarchy> &cHierarchy)
{
    std::vector<std::vector<xFace*> > cFaces;
    std::vector<std::vector<xWORD> >   cVerts;
    std::vector<xWORD>::iterator       found;
    cFaces.resize(cBoundings.size());
    cVerts.resize(cBoundings.size());

    xBYTE *src    = (xBYTE *) elem->verticesP;
    xDWORD stride = elem->skeletized
        ? (elem->textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (baseHierarchy)
    {
        xFace **iterF = baseHierarchy->facesP;
        for (int i = 0; i < baseHierarchy->facesC; ++i, ++iterF)
            for (int j = cBoundings.size()-1; j >= 0 ; --j)
            {
                const xBoxA &bounding = cBoundings[j];
                const xVector3 *vert1 = (xVector3*) &((xVertex*) (src + stride*(**iterF)[0]))->pos;
                const xVector3 *vert2 = (xVector3*) &((xVertex*) (src + stride*(**iterF)[1]))->pos;
                const xVector3 *vert3 = (xVector3*) &((xVertex*) (src + stride*(**iterF)[2]))->pos;

                if (bounding.Contains(*vert1) ||
                    bounding.Contains(*vert2) ||
                    bounding.Contains(*vert3))
                {
                    cFaces[j].push_back(*iterF);

                    found = std::find<std::vector<xWORD>::iterator, xWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (**iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((**iterF)[0]);
                    found = std::find<std::vector<xWORD>::iterator, xWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (**iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((**iterF)[0]);
                    found = std::find<std::vector<xWORD>::iterator, xWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (**iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((**iterF)[0]);
                    break;
                }
            }
    }
    else
    {
        xFace *iterF = elem->facesP;
        for (int i = 0; i < elem->facesC; ++i, ++iterF)
            for (int j = cBoundings.size()-1; j >= 0 ; --j)
            {
                const xBoxA &bounding = cBoundings[j];
                const xVector3 *vert1 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[0]))->pos;
                const xVector3 *vert2 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[1]))->pos;
                const xVector3 *vert3 = (xVector3*) &((xVertex*) (src + stride*(*iterF)[2]))->pos;

                if (bounding.Contains(*vert1) ||
                    bounding.Contains(*vert2) ||
                    bounding.Contains(*vert3))
                {
                    cFaces[j].push_back(iterF);

                    found = std::find<std::vector<xWORD>::iterator, xWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (*iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((*iterF)[0]);
                    found = std::find<std::vector<xWORD>::iterator, xWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (*iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((*iterF)[0]);
                    found = std::find<std::vector<xWORD>::iterator, xWORD>
                                        (cVerts[j].begin(), cVerts[j].end(), (*iterF)[0]);
                    if (found == cVerts[j].end()) cVerts[j].push_back((*iterF)[0]);
                    break;
                }
            }
    }

    size_t size = cFaces.size();
    for (size_t j = 0; j < size; ++j)
    {
        xCollisionHierarchy hierarchy;
        hierarchy.kidsC = 0;
        hierarchy.kidsP = NULL;
        hierarchy.facesC = cFaces[j].size();
        hierarchy.verticesC = cVerts[j].size();

        if (hierarchy.verticesC)
        {
            xWORD *iterHV = hierarchy.verticesP = new xWORD[hierarchy.verticesC];
            std::vector<xWORD>::iterator endBV = cVerts[j].end();
            for (std::vector<xWORD>::iterator iterBV = cVerts[j].begin();
                 iterBV != endBV; ++iterBV, ++iterHV)
                *iterHV = *iterBV;
        }
        else
            hierarchy.verticesP = NULL;

        if (hierarchy.facesC)
        {
            xFace **iterHF = hierarchy.facesP = new xFace*[hierarchy.facesC];
            std::vector<xFace*>::iterator endBF = cFaces[j].end();
            for (std::vector<xFace*>::iterator iterBF = cFaces[j].begin();
                 iterBF != endBF; ++iterBF, ++iterHF)
                *iterHF = *iterBF;
        }
        else
            hierarchy.facesP = NULL;

        cHierarchy.push_back(hierarchy);
    }
}
