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
    size_t stride     = this->GetVertexStride();
    xBYTE *verticesIn = (xBYTE*) this->L_vertices;

    this->renderData.L_face_normals = new xVector3[this->I_faces];
    xVector3 *faceNormal = this->renderData.L_face_normals;

    //// assign smoothing groups to vertices
    std::vector<smoothVert>::iterator iterF, iterE, iterF2, iterE2;
    std::vector<std::vector<smoothVert> > vertices; // vertex_id->(smooth, normal)[]
    vertices.resize(this->I_vertices);
    xDWORD *smooth = this->L_smooth;
    xFace *faceIn  = this->L_faces;
    for (int fP=this->I_faces; fP; --fP, ++smooth, ++faceIn, ++faceNormal)
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
    xDWORD verticesC = this->I_vertices;
    std::vector<std::vector<smoothVert> >::iterator vertF = vertices.begin(), vertF2;
    std::vector<std::vector<smoothVert> >::iterator vertE = vertices.end();
    xBYTE *xvertI = verticesIn, *xvertI2;

    for (int i=0; vertF != vertE; ++vertF, xvertI += stride, ++i)
    {
        if (!vertF->size()) continue;
        xVector3 *v1 = (xVector3*)xvertI;

        xvertI2 = xvertI+stride;
        int j = i+1;
        for (vertF2 = vertF+1; vertF2 != vertE; ++vertF2, xvertI2 += stride, ++j)
        {
            xVector3 *v2 = (xVector3*)xvertI2;
            if (!v1->nearlyEqual(*v2)) continue;

            iterE  = vertF->end();
            iterE2 = vertF2->end();
            for (iterF = vertF->begin(); iterF != iterE; ++iterF)
            {
                for (iterF2 = vertF2->begin(); iterF2 != iterE2; ++iterF2)
                {
                    if (iterF->smooth & iterF2->smooth)
                    {
                        iterF->mnormal  += iterF2->normal;
                        iterF->mcount   += iterF2->count;
                        iterF2->mnormal += iterF->normal;
                        iterF2->mcount  += iterF->count;
                    }
                }
            }
        }
    }

    //// merge smoothing groups, where possible
    for (vertF = vertices.begin(); vertF != vertE; ++vertF)
    {
        if (!vertF->size()) continue;

        iterE = vertF->end();
        for (iterF = vertF->begin(); iterF != iterE; ++iterF)
        {
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
    }

    //// normalize normals and assign ids for new vertex duplicates
    for (vertF = vertices.begin(); vertF != vertE; ++vertF)
    {
        if (!vertF->size()) continue;

        bool used = false;
        iterE = vertF->end();
        for (iterF = vertF->begin(); iterF != iterE; ++iterF)
        {
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
    }

    this->renderData.I_vertices = verticesC;
    if (verticesC == this->I_vertices)
    {
        this->renderData.L_vertices = this->L_vertices;
        this->renderData.L_faces    = this->L_faces;
    }
    else
    {
        this->renderData.L_vertices = (xVertex*) new xBYTE[stride*verticesC];
        memcpy(this->renderData.L_vertices, this->L_vertices, stride*this->I_vertices);
        this->renderData.L_faces = new xFace[this->I_faces];
        //// fill and correct faces
        smooth  = this->L_smooth;
        faceIn  = this->L_faces;
        xFace *faceOut = this->renderData.L_faces;
        for (int fP=this->I_faces; fP; --fP, ++smooth, ++faceIn, ++faceOut)
        {
            for (int i=0; i<3; ++i)
            {
                int  idx = (*faceOut)[i] = (*faceIn)[i];
                iterE = vertices[idx].end();
                for (iterF = vertices[idx].begin(); iterF != iterE; ++iterF)
                {
                    if (iterF->smooth == *smooth && (*smooth || iterF->flatFace == faceIn))
                    {
                        if (iterF->vertexId) idx = iterF->vertexId;
                        (*faceOut)[i] = idx;
                        break;
                    }
                }
            }
        }
    }

    this->renderData.L_normals   = new xVector3[verticesC];
    //// duplicate vertices and fill normals
    xBYTE    *verticesOut = ((xBYTE*) this->renderData.L_vertices) + stride*this->I_vertices;
    xVector3 *L_normals     = this->renderData.L_normals;
    xVector3 *normalP2    = this->renderData.L_normals + this->I_vertices;
    for (vertF = vertices.begin(); vertF != vertE; ++vertF, verticesIn += stride, ++L_normals)
    {
        if (!vertF->size()) continue;
        iterE = vertF->end();
        for (iterF = vertF->begin(); iterF != iterE; ++iterF)
        {
            if (!iterF->mergedWith)
            {
                if (iterF->vertexId)
                {
                    memcpy(verticesOut, verticesIn, stride);
                    verticesOut += stride;
                    *(normalP2++) = iterF->mnormal;
                }
                else
                    *L_normals = iterF->mnormal;
            }
        }
    }
}

////////////////////// Shadow Edges
xMaterial *xFaceGetMaterial(const xElement *elem, int faceIdx)
{
    xFaceList *faceL = elem->L_faceLists;
    for (int i=elem->I_faceLists; i; --i, ++faceL)
        if (faceIdx - faceL->I_offset < faceL->I_count)
            return faceL->Material;
    return NULL;
}

bool       xFaceTransparent(const xElement *elem, int faceIdx)
{
    xMaterial *mat = xFaceGetMaterial(elem, faceIdx);
    return mat && mat->transparency > 0.f;
}
void       xFaceTransparentOr2Sided(const xElement *elem, int faceIdx, bool &outTransp, bool &outTwoSide)
{
    xMaterial *mat = xFaceGetMaterial(elem, faceIdx);
    outTransp  = mat && mat->transparency > 0.f;
    outTwoSide = mat && mat->two_sided;
}


void xElement :: FillShadowEdges ()
{
    xWORD edgesC = 3 * (this->I_faces);
    this->L_edges = new xEdge[edgesC << 1];

    bool *flags = new bool[this->I_faces*3];
    memset(flags, 0, this->I_faces*3);

    xFace *iterF1 = this->L_faces, *iterF2;
    bool  *iterUsed1 = flags, *iterUsed2;
    xEdge *iterE = this->L_edges;

    size_t stride = this->FL_skeletized
        ? this->FL_textured
        ? sizeof (xVertexTexSkel)
        : sizeof (xVertexSkel)
        : this->FL_textured
        ? sizeof (xVertexTex)
        : sizeof (xVertex);
    xBYTE *verticesIn = (xBYTE*) this->L_vertices;

    xDWORD cnt = 0;
    for (int i = 0; i < this->I_faces; ++i, ++iterF1, iterUsed1 += 3)
    {
        bool transp1;
        bool twoSide1;
        xFaceTransparentOr2Sided(this, i, transp1, twoSide1);
        if (transp1) continue;
        for (int e1 = 0; e1 < 3; ++e1)
            if (!iterUsed1[e1])
            {
                ++cnt;
                iterE->ID_vert_1 = (*iterF1)[e1];
                iterE->ID_vert_2 = (*iterF1)[(e1+1)%3];
                iterE->ID_face_1 = i;
                iterE->ID_face_2 = xWORD_MAX;
                iterUsed1[e1] = true;
                xVector3 *vc1a = (xVector3 *)(verticesIn + stride* iterE->ID_vert_1);
                xVector3 *vc1b = (xVector3 *)(verticesIn + stride* iterE->ID_vert_2);

                iterF2    = iterF1 + 1;
                iterUsed2 = iterUsed1 + 3;
                bool found = false;

                for (int j = i+1; j < this->I_faces && !found; ++j, ++iterF2, iterUsed2+=3)
                {
                    bool transp2;
                    bool twoSide2;
                    xFaceTransparentOr2Sided(this, i, transp2, twoSide2);
                    if (transp2) continue;
                    for (int e2 = 0; e2 < 3 && !found; ++e2)
                        if (!iterUsed2[e2] && !(twoSide1 ^ twoSide2))
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
                                iterE->ID_face_2 = j;
                                iterUsed2[e2] = true;
                                found = true;
                                --edgesC;
                            }
                        }
                }
                if (iterE->ID_face_2 == xWORD_MAX)
                {
                    iterE->ID_face_2 = xWORD_MAX;
                }
                ++iterE;
            }
    }
    xEdge *tmp = this->L_edges;
    this->I_edges = cnt;
    this->L_edges = new xEdge[cnt];
    memcpy(this->L_edges, tmp, cnt*sizeof(xEdge));
    delete[] tmp;
}

#include "../../Math/Figures/xSphere.h"
#include "../../Math/Figures/xBoxO.h"
#include "../../Math/Figures/xMesh.h"
using namespace Math::Figures;

xBoxA xElement :: ReFillBVHNode(xBVHierarchy        &BVH_node,
                                xMeshData           &MeshData,
                                const xMatrix &MX_LocalToWorld)
{
    xBoxA boxA;
    if (BVH_node.I_items)
    {
        boxA.P_min.init(xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE);
        boxA.P_max.init(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);

        xBoxA boxAc;
        xBVHierarchy *BVH_kid = BVH_node.L_items;
        for (int i = BVH_node.I_items; i; --i, ++BVH_kid)
        {
            boxAc = ReFillBVHNode(*BVH_kid, MeshData, MX_LocalToWorld);
            if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
            if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
            if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
            if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
            if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
            if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
        }

        xVector3 NW_extends = (boxA.P_max - boxA.P_min) * 0.5f;
        xBoxO &boxO = *(xBoxO*) BVH_node.GetTransformed(MX_LocalToWorld);
        boxO.P_center = boxA.P_min + NW_extends;
        boxO.S_side  = NW_extends.x;
        boxO.S_front = NW_extends.y;
        boxO.S_top   = NW_extends.z;
    }
    else
    {
        xMesh &mesh = *(xMesh*) BVH_node.GetTransformed(MX_LocalToWorld);

        boxA.P_max = boxA.P_min = MeshData.GetVertexTransf(mesh.L_VertexIndices[0]);

        xDWORD *MV_iter = mesh.L_VertexIndices + 1;
        for (xDWORD i = mesh.I_VertexIndices-1; i; --i, ++MV_iter)
        {
            xPoint3 &P_tmp = MeshData.GetVertexTransf(*MV_iter);
            if (P_tmp.x < boxA.P_min.x) boxA.P_min.x = P_tmp.x;
            else
            if (P_tmp.x > boxA.P_max.x) boxA.P_max.x = P_tmp.x;
            if (P_tmp.y < boxA.P_min.y) boxA.P_min.y = P_tmp.y;
            else
            if (P_tmp.y > boxA.P_max.y) boxA.P_max.y = P_tmp.y;
            if (P_tmp.z < boxA.P_min.z) boxA.P_min.z = P_tmp.z;
            else
            if (P_tmp.z > boxA.P_max.z) boxA.P_max.z = P_tmp.z;
        }
    }

    return boxA;
}

xBoxA xElement :: ReFillBVH  ( xBVHierarchy *L_BVH, xMeshData *MeshData,
                               const xMatrix &MX_LocalToWorld )
{
    xBVHierarchy &BVH_node  = L_BVH[ID];
    xMeshData    &Mesh_node = MeshData[ID];

    xBoxA boxA, boxAc;
    boxA.P_min.init(xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE);
    boxA.P_max.init(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);

    if (BVH_node.I_items)
    {
        xBVHierarchy *BVH_kid = BVH_node.L_items;
        for (int i = boundingData.I_kids; i; --i, ++BVH_kid)
        {
            boxAc = ReFillBVHNode(*BVH_kid, Mesh_node, MX_LocalToWorld);
            if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
            if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
            if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
            if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
            if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
            if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
        }

        xVector3 NW_extends = (boxA.P_max - boxA.P_min) * 0.5f;
        xBoxO &boxO = *(xBoxO*) BVH_node.GetTransformed(MX_LocalToWorld);
        boxO.P_center = boxA.P_min + NW_extends;
        boxO.S_side  = NW_extends.x;
        boxO.S_front = NW_extends.y;
        boxO.S_top   = NW_extends.z;
        boxO.N_side.init(1.f, 0.f, 0.f);
        boxO.N_front.init(0.f, 1.f, 0.f);
        boxO.N_top.init(0.f, 0.f, 1.f);
    }

    xElement *elem = L_kids;
    for (; elem; elem = elem->Next)
    {
        boxAc = elem->ReFillBVH( L_BVH, MeshData, MX_LocalToWorld );
        if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
        if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
        if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
        if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
        if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
        if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
    }

    return boxA;
}

xBoxA xElement :: FillBVHNode(xBoundingHierarchy  &BH_node,
                              xBVHierarchy        &BVH_node,
                              xMeshData           &MeshData)
{
    xBoxA boxA;
    BVH_node.init(*new xBoxO());

    if (BH_node.I_kids)
    {
        boxA.P_min.init(xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE);
        boxA.P_max.init(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);

        BVH_node.I_items = BH_node.I_kids;
        BVH_node.L_items = new xBVHierarchy[BVH_node.I_items];

        xBoxA boxAc;
        xBoundingHierarchy *BH_kid  = BH_node.L_kids;
        xBVHierarchy       *BVH_kid = BVH_node.L_items;
        for (int i = BH_node.I_kids; i; --i, ++BH_kid, ++BVH_kid)
        {
            boxAc = FillBVHNode(*BH_kid, *BVH_kid, MeshData);
            if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
            if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
            if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
            if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
            if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
            if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
        }
    }
    else
    {
        BVH_node.I_items = 1;
        BVH_node.L_items = new xBVHierarchy[1];

        BVH_node.L_items[0].init(*new xMesh());
        xMesh &mesh   = *(xMesh*) BVH_node.L_items[0].Figure;
        mesh.MeshData = &MeshData;
        mesh.I_FaceIndices = BH_node.I_faces;
        mesh.L_FaceIndices = new xDWORD[mesh.I_FaceIndices];
        xDWORD *MF_iter = mesh.L_FaceIndices;
        xFace **CF_iter = BH_node.L_faces;
        for (xDWORD i = mesh.I_FaceIndices; i; --i, ++MF_iter, ++CF_iter)
            *MF_iter = (*CF_iter - L_faces);// % MeshData->I_FaceStride;

        boxA.P_max = boxA.P_min = MeshData.GetVertex(BH_node.L_vertices[0]);

        mesh.I_VertexIndices = BH_node.I_vertices;
        mesh.L_VertexIndices = new xDWORD[mesh.I_VertexIndices];
        xDWORD *MV_iter = mesh.L_VertexIndices;
        xWORD  *CV_iter = BH_node.L_vertices;
        for (xDWORD i = mesh.I_VertexIndices; i; --i, ++MV_iter, ++CV_iter)
        {
            xPoint3 &P_tmp = MeshData.GetVertex(*MV_iter = *CV_iter);
            if (P_tmp.x < boxA.P_min.x) boxA.P_min.x = P_tmp.x;
            else
            if (P_tmp.x > boxA.P_max.x) boxA.P_max.x = P_tmp.x;
            if (P_tmp.y < boxA.P_min.y) boxA.P_min.y = P_tmp.y;
            else
            if (P_tmp.y > boxA.P_max.y) boxA.P_max.y = P_tmp.y;
            if (P_tmp.z < boxA.P_min.z) boxA.P_min.z = P_tmp.z;
            else
            if (P_tmp.z > boxA.P_max.z) boxA.P_max.z = P_tmp.z;
        }
    }

    xVector3 NW_extends = (boxA.P_max - boxA.P_min) * 0.5f;
    xBoxO &boxO = *(xBoxO*) BVH_node.Figure;
    boxO.P_center = boxA.P_min + NW_extends;
    boxO.S_side  = NW_extends.x;
    boxO.S_front = NW_extends.y;
    boxO.S_top   = NW_extends.z;
    boxO.N_side.init(1.f, 0.f, 0.f);
    boxO.N_front.init(0.f, 1.f, 0.f);
    boxO.N_top.init(0.f, 0.f, 1.f);

    return boxA;
}

xBoxA xElement :: FillBVH  ( xBVHierarchy *L_BVH, xMeshData *MeshData  )
{
    xBVHierarchy &BVH_node  = L_BVH[ID];
    xMeshData    &Mesh_node = MeshData[ID];

    xBoxA boxA, boxAc;
    boxA.P_min.init(xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE);
    boxA.P_max.init(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);

    if (boundingData.I_kids)
    {
        BVH_node.init(*new xBoxO());
        BVH_node.I_items = boundingData.I_kids;
        BVH_node.L_items = new xBVHierarchy[BVH_node.I_items];

        //xMeshData *MeshData = new xMeshData();
        Mesh_node.ID = ID;
        Mesh_node.I_FaceCount    = I_faces;
        Mesh_node.I_FaceStride   = sizeof(xFace);
        Mesh_node.L_FaceData     = (xBYTE*) L_faces;
        Mesh_node.I_VertexCount  = I_vertices;
        Mesh_node.I_VertexStride = GetVertexStride();
        Mesh_node.L_VertexData   = (xBYTE*) L_vertices;
        Mesh_node.MX_MeshToLocal = MX_MeshToLocal;
        if (FL_skeletized)
        {
            Mesh_node.L_BoneData   = (xBYTE*) L_verticesS->bone;
            Mesh_node.I_BoneStride = Mesh_node.I_VertexStride;
        }

        xBoundingHierarchy *CH_kid  = boundingData.L_kids;
        xBVHierarchy       *BVH_kid = BVH_node.L_items;
        for (int i = boundingData.I_kids; i; --i, ++CH_kid, ++BVH_kid)
        {
            boxAc = FillBVHNode(*CH_kid, *BVH_kid, Mesh_node);
            if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
            if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
            if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
            if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
            if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
            if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
        }

        xVector3 NW_extends = (boxA.P_max - boxA.P_min) * 0.5f;
        xBoxO &boxO = *(xBoxO*) BVH_node.Figure;
        boxO.P_center = boxA.P_min + NW_extends;
        boxO.S_side  = NW_extends.x;
        boxO.S_front = NW_extends.y;
        boxO.S_top   = NW_extends.z;
        boxO.N_side.init(1.f, 0.f, 0.f);
        boxO.N_front.init(0.f, 1.f, 0.f);
        boxO.N_top.init(0.f, 0.f, 1.f);
    }
    else
    {
        BVH_node.init(*new xSphere());
        xSphere &sphere = *(xSphere*) BVH_node.Figure;
        sphere.P_center.zero();
        sphere.S_radius = 0.f;

        Mesh_node.I_FaceCount    = 0;
        Mesh_node.I_FaceStride   = 0;
        Mesh_node.L_FaceData     = 0;
        Mesh_node.I_VertexCount  = 0;
        Mesh_node.I_VertexStride = 0;
        Mesh_node.L_VertexData   = 0;
        Mesh_node.MX_MeshToLocal = MX_MeshToLocal;
        Mesh_node.L_BoneData     = 0;
        Mesh_node.I_BoneStride   = 0;
    }

    xElement *elem = L_kids;
    for (; elem; elem = elem->Next)
    {
        boxAc = elem->FillBVH( L_BVH, MeshData );
        if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
        if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
        if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
        if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
        if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
        if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
    }

    return boxA;
}
