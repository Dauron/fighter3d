#include "xModel.h"

////////////////////// xCollisionHierarchy
void xCollisionHierarchy :: Load ( FILE *file, xElement *elem )
{
    xCollisionData::Load(file, elem);

    fread(&this->I_faces, sizeof(this->I_faces), 1, file);
    if (this->I_faces && !this->I_kids)
    {
        this->L_faces = new xFace*[this->I_faces];
        xFace **iter = this->L_faces;
        xDWORD idx;
        for (int i=this->I_faces; i; --i, ++iter)
        {
            fread(&idx, sizeof(idx), 1, file);
            *iter = elem->L_faces + idx;
        }
    }
    else
        this->L_faces = 0;

    fread(&this->I_vertices, sizeof(xWORD), 1, file);
    if (this->I_vertices && !this->I_kids)
    {
        this->L_vertices = new xWORD[this->I_vertices];
        fread(this->L_vertices, sizeof(xWORD), this->I_vertices, file);
    }
    else
        this->L_vertices = 0;
}

void xCollisionHierarchy :: Save ( FILE *file, xElement *elem )
{
    xCollisionData::Save(file, elem);

    fwrite(&this->I_faces, sizeof(this->I_faces), 1, file);
    if (this->I_faces && !this->I_kids && this->L_faces)
    {
        xFace **iter = this->L_faces;
        xDWORD idx;
        for (int i=this->I_faces; i; --i, ++iter)
        {
            idx = *iter - elem->L_faces;
            fwrite(&idx, sizeof(idx), 1, file);
        }
    }

    fwrite(&this->I_vertices, sizeof(this->I_vertices), 1, file);
    if (this->I_vertices && !this->I_kids && this->L_vertices)
        fwrite(this->L_vertices, sizeof(xWORD), this->I_vertices, file);
}

////////////////////// xCollisionData

void xCollisionData :: Load (FILE *file, xElement *elem)
{
    fread(&this->I_kids, sizeof(this->I_kids), 1, file);
    if (this->I_kids)
    {
        this->L_kids = new xCollisionHierarchy[this->I_kids];
        xCollisionHierarchy *iter = this->L_kids;
        for (int i = this->I_kids; i; --i, ++iter)
            iter->Load(file, elem);
    }
    else
        this->L_kids = NULL;
}

void xCollisionData :: Save (FILE *file, xElement *elem)
{
    fwrite(&this->I_kids, sizeof(this->I_kids), 1, file);
    if (this->I_kids)
    {
        xCollisionHierarchy *iter = this->L_kids;
        for (int i = this->I_kids; i; --i, ++iter)
            iter->Save(file, elem);
    }
}

void xCollisionData :: FreeKids()
{
    if (this->L_kids)
    {
        xCollisionHierarchy *ch = this->L_kids;
        for (int i = this->I_kids; i; --i)
        {
            if (ch->L_faces)    delete[] ch->L_faces;
            if (ch->L_vertices) delete[] ch->L_vertices;
            ch->FreeKids();
        }
        delete[] this->L_kids;
        this->L_kids = NULL;
        this->I_kids = 0;
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
    //if (this->L_kids) // force Octree recalculation
    //    this->FreeKids();
    if (this->L_kids == NULL)
    {
        if (!elem->I_vertices)
        {
            this->L_kids = NULL;
            this->I_kids = 0;
            return;
        }
        if (elem->I_faces < 20)
        {
            this->L_kids = new xCollisionHierarchy[1];
            this->I_kids = 1;
            this->L_kids->I_faces = elem->I_faces;
            this->L_kids->L_faces = new xFace*[elem->I_faces];
            this->L_kids->I_vertices = elem->I_vertices;
            this->L_kids->L_vertices = new xWORD[elem->I_vertices];
            this->L_kids->I_kids = 0;
            this->L_kids->L_kids = NULL;

            xFace **iterHF = this->L_kids->L_faces;
            xFace  *iterF  = elem->L_faces;
            for (int i = elem->I_faces; i; --i, ++iterHF, ++iterF)
                *iterHF = iterF;

            xWORD *iterHV = this->L_kids->L_vertices;
            for (xWORD i = 0; i < elem->I_vertices; ++i, ++iterHV)
                *iterHV = i;
            return;
        }

        xBYTE *src    = (xBYTE *) elem->L_vertices;
        xDWORD stride = elem->FL_skeletized
            ? (elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
            : (elem->FL_textured ? sizeof(xVertexTex)     : sizeof(xVertex));
        std::vector<xCollisionHierarchy> cHierarchy;

        if (!elem->FL_skeletized || !xmodel->Spine.I_bones)
        {
            xCollisionHierarchy hierarchy;
            hierarchy.I_kids = 0;
            hierarchy.L_kids = NULL;
            hierarchy.I_faces    = elem->I_faces;
            hierarchy.I_vertices = elem->I_vertices;
            xFace **iterHF = hierarchy.L_faces    = new xFace*[elem->I_faces];
            xWORD   *iterHV = hierarchy.L_vertices = new xWORD[elem->I_vertices];
            xFace  *iterF  = elem->L_faces;

            xBoxA bounding;
            bounding.P_max.x = -1000000000.f; bounding.P_max.y = -1000000000.f; bounding.P_max.z = -1000000000.f;
            bounding.P_min.x =  1000000000.f; bounding.P_min.y =  1000000000.f; bounding.P_min.z =  1000000000.f;

            for (int i = elem->I_faces; i; --i, ++iterHF, ++iterF)
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

            for (xDWORD i = 0; i < elem->I_vertices; ++i, ++iterHV)
                *iterHV = i;

            hierarchy.Subdivide(elem, 5.f, 1, bounding);
            cHierarchy.push_back(hierarchy);
        }
        else
        {
            this->I_kids = xmodel->Spine.I_bones;

            std::vector<xBoxA> cBoundings;
            cBoundings.resize(this->I_kids);

            xBYTE *iterV = src;
            for (xDWORD i = elem->I_vertices; i; --i, iterV += stride)
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
                if (cHierarchy[i].I_faces > 20 && cHierarchy[i].I_kids == 0)
                {
                    float scale = cHierarchy[i].I_faces / 20.f;
                    if (scale < 2.0f) scale = 2.0f;
                    if (scale > 10.f) scale = 10.f;
                    cHierarchy[i].Subdivide(elem, scale, 1, cBoundings[i]);
                }
        }

        for (size_t i = 0; i<cHierarchy.size(); ++i)
            if (cHierarchy[i].I_faces == 0)
            {
                cHierarchy[i--] = *cHierarchy.rbegin();
                cHierarchy.resize(cHierarchy.size()-1);
            }
        this->I_kids = cHierarchy.size();
        this->L_kids = new xCollisionHierarchy[cHierarchy.size()];
        for (size_t i = 0; i<cHierarchy.size(); ++i)
            this->L_kids[i] = cHierarchy[i];
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
            if (cHierarchy[i].I_faces > 20)
            {
                float scale = cHierarchy[i].I_faces / 20.f;
                if (scale < 2.f)  scale = 2.f;
                if (scale > 10.f) scale = 10.f;
                cHierarchy[i].Subdivide(elem, scale, depth+1, cBoundings[i]);
            }

    for (size_t i = 0; i<cHierarchy.size(); ++i)
        if (cHierarchy[i].I_faces == 0)
        {
            cHierarchy[i--] = *cHierarchy.rbegin();
            cHierarchy.resize(cHierarchy.size()-1);
        }

    if (cHierarchy.empty())
        return;

    delete[] this->L_faces;
    this->L_faces = NULL;
    delete[] this->L_vertices;
    this->L_vertices = NULL;
    if (cHierarchy.size() == 1)
    {
        this->L_vertices = cHierarchy[0].L_vertices;
        this->L_faces = cHierarchy[0].L_faces;
        this->I_kids  = cHierarchy[0].I_kids;
        this->L_kids  = cHierarchy[0].L_kids;
    }
    else
    {
        this->I_kids = cHierarchy.size();
        this->L_kids = new xCollisionHierarchy[cHierarchy.size()];
        for (size_t i = 0; i<cHierarchy.size(); ++i)
            this->L_kids[i] = cHierarchy[i];
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

    xBYTE *src    = (xBYTE *) elem->L_vertices;
    xDWORD stride = elem->FL_skeletized
        ? (elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel))
        : (elem->FL_textured ? sizeof(xVertexTex)     : sizeof(xVertex));

    if (baseHierarchy)
    {
        xFace **iterF = baseHierarchy->L_faces;
        for (int i = 0; i < baseHierarchy->I_faces; ++i, ++iterF)
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
        xFace *iterF = elem->L_faces;
        for (int i = 0; i < elem->I_faces; ++i, ++iterF)
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
        hierarchy.I_kids = 0;
        hierarchy.L_kids = NULL;
        hierarchy.I_faces = cFaces[j].size();
        hierarchy.I_vertices = cVerts[j].size();

        if (hierarchy.I_vertices)
        {
            xWORD *iterHV = hierarchy.L_vertices = new xWORD[hierarchy.I_vertices];
            std::vector<xWORD>::iterator endBV = cVerts[j].end();
            for (std::vector<xWORD>::iterator iterBV = cVerts[j].begin();
                 iterBV != endBV; ++iterBV, ++iterHV)
                *iterHV = *iterBV;
        }
        else
            hierarchy.L_vertices = NULL;

        if (hierarchy.I_faces)
        {
            xFace **iterHF = hierarchy.L_faces = new xFace*[hierarchy.I_faces];
            std::vector<xFace*>::iterator endBF = cFaces[j].end();
            for (std::vector<xFace*>::iterator iterBF = cFaces[j].begin();
                 iterBF != endBF; ++iterBF, ++iterHF)
                *iterHF = *iterBF;
        }
        else
            hierarchy.L_faces = NULL;

        cHierarchy.push_back(hierarchy);
    }
}
