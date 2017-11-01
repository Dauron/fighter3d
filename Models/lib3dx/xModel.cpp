#include "xModel.h"
#include "xUtils.h"
#include <vector>
#include "../../Utils/Debug.h"

/* SKELETON */
void _xElement_SkeletonAdd(xElement *elem)
{
    if (!elem->FL_skeletized)
    {
        elem->FL_skeletized = true;
        size_t strideO = elem->FL_textured ? sizeof(xVertexTex) : sizeof(xVertex);
        size_t strideN = elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        for (int dataSrc = 0; dataSrc < 2; ++dataSrc)
        {
            int    count = dataSrc ? elem->I_vertices : elem->renderData.I_vertices;
            if (!count) continue;
            xBYTE *ptrO  = (xBYTE *) (dataSrc ? elem->L_vertices : elem->renderData.L_vertices);
            xBYTE *ptrN  = new xBYTE[strideN*count];
            xBYTE *oldF = ptrO, *oldE = ptrO + strideO*count, *newF = ptrN; int i = 0;
            for (; oldF != oldE; oldF+=strideO, newF+=strideN, ++i) {
                memcpy(newF, oldF, 3*sizeof(xFLOAT));
                if (elem->FL_textured)
                    memcpy(newF + 7*sizeof(xFLOAT), oldF + 3*sizeof(xFLOAT), 2*sizeof(xFLOAT));
                xVertexSkel *curr = (xVertexSkel*)newF;
                curr->b0 = 0.100f;
                curr->b1 = 0.000f;
                curr->b2 = 0.000f;
                curr->b3 = 0.000f;
            }
            if (dataSrc)
            {
                delete[] elem->L_vertices;
                elem->L_vertices = (xVertex*)ptrN;
            }
            else
            {
                delete[] elem->renderData.L_vertices;
                if (elem->renderData.L_vertices == elem->L_vertices)
                {
                    elem->L_vertices = elem->renderData.L_vertices = (xVertex*)ptrN;
                    break;
                }
                else
                    elem->renderData.L_vertices = (xVertex*)ptrN;
            }
        }
    }
    for (xElement *last = elem->L_kids; last; last = last->Next)
        _xElement_SkeletonAdd(last);
}

void  xModel :: SkeletonAdd()
{
    if (!this->Spine.I_bones)
    {
        this->Spine.I_bones = 1;
        this->Spine.L_bones = new xBone[1];
        this->Spine.L_bones->Zero();

        for (xElement *last = this->L_kids; last; last = last->Next)
            _xElement_SkeletonAdd(last);
    }
}

/* BONES */
void  _xBoneDelete_CorrectElementIds(xElement *elem, xWORD ID_delete, xWORD ID_parent, xWORD ID_top)
{
    xElement *last = elem->L_kids;
    for (; last; last = last->Next)
        _xBoneDelete_CorrectElementIds(last, ID_delete, ID_parent, ID_top);

    if (elem->FL_skeletized)
    {
        xDWORD stride = elem->FL_textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
        for (int dataSrc = 0; dataSrc < 2; ++dataSrc)
        {
            int    count = dataSrc ? elem->I_vertices : elem->renderData.I_vertices;
            xBYTE *src   = (xBYTE *) (dataSrc ? elem->L_vertices : elem->renderData.L_vertices);
            for (int i = count; i; --i, src += stride)
            {
                xVertexSkel *vert = (xVertexSkel *)src;
                int i0 = (int) floor(vert->b0);
                int i1 = (int) floor(vert->b1);
                int i2 = (int) floor(vert->b2);
                int i3 = (int) floor(vert->b3);

                if (i0 == ID_delete) vert->b0 += -i0 + ID_parent; // vertices from deleted bone are reassigned to parent
                else
                if (i0 == ID_top)    vert->b0 += -i0 + ID_delete; // vertices from top bone are reassigned to its new id

                if (i1 == ID_delete) vert->b1 += -i1 + ID_parent; // vertices from deleted bone are reassigned to parent
                else
                if (i1 == ID_top)    vert->b1 += -i1 + ID_delete; // vertices from top bone are reassigned to its new id

                if (i2 == ID_delete) vert->b2 += -i2 + ID_parent; // vertices from deleted bone are reassigned to parent
                else
                if (i2 == ID_top)    vert->b2 += -i2 + ID_delete; // vertices from top bone are reassigned to its new id

                if (i3 == ID_delete) vert->b3 += -i3 + ID_parent; // vertices from deleted bone are reassigned to parent
                else
                if (i3 == ID_top)    vert->b3 += -i3 + ID_delete; // vertices from top bone are reassigned to its new id

                i0 = (int) floor(vert->b0);
                i1 = (int) floor(vert->b1);
                i2 = (int) floor(vert->b2);
                i3 = (int) floor(vert->b3);

                if (i0 == i3)
                {
                    vert->b0 += vert->b3 - i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i0 == i2)
                {
                    vert->b0 += vert->b2 - i2;
                    vert->b2 = vert->b3; i2 = i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i0 == i1)
                {
                    vert->b0 += vert->b1 - i1;
                    vert->b1 = vert->b2; i1 = i2;
                    vert->b2 = vert->b3; i2 = i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i1 >= 0 && i1 == i3)
                {
                    vert->b1 += vert->b3 - i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i1 >= 0 && i1 == i2)
                {
                    vert->b1 += vert->b2 - i2;
                    vert->b2 = vert->b3; i2 = i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
                if (i2 >= 0 && i2 == i3)
                {
                    vert->b2 += vert->b3 - i3;
                    vert->b3 = 0.f;      i3 = -1;
                }
            }
        }
    }
}

void   xModel :: BoneDelete(xBYTE ID_bone)
{
    xBone &bone = Spine.L_bones[ID_bone];

    if (ID_bone) {                                   // cannot delete spine root
        xBone &parent = Spine.L_bones[bone.ID_parent];

        parent.KidDelete(ID_bone);

        xBYTE *ID_iter = bone.ID_kids;
        for (int i = bone.I_kids; i; --i, ++ID_iter)
        {
            parent.KidAdd(*ID_iter);
            xBone &kid = Spine.L_bones[*ID_iter];
            kid.ID_parent   = parent.ID;
            kid.P_begin     = parent.P_end;
            kid.S_lengthSqr = (kid.P_end-kid.P_begin).lengthSqr();
            kid.S_length    = sqrt(kid.S_lengthSqr);
        }

        xWORD ID_last = Spine.I_bones-1, ID_parent = bone.ID_parent;
        if (ID_bone != ID_last) // if it is not the bone with the last id,
        {                       //   then we have to correct largest id (= count)
            bone = Spine.L_bones[ID_last];
            bone.ID = ID_bone;

            Spine.L_bones[bone.ID_parent].KidReplace(ID_last, ID_bone);
            ID_iter = bone.ID_kids;
            for (int i = bone.I_kids; i; --i, ++ID_iter)
                Spine.L_bones[*ID_iter].ID_parent = ID_bone;
        }

        --(Spine.I_bones);
        xBone *bones = new xBone[Spine.I_bones];
        memcpy(bones, Spine.L_bones, sizeof(xBone)*Spine.I_bones);
        delete[] Spine.L_bones;
        Spine.L_bones = bones;

        xElement *last = this->L_kids;
        for (; last; last = last->Next)
            _xBoneDelete_CorrectElementIds(last, ID_bone, ID_parent, ID_last);

        Spine.FillBoneConstraints();
    }
}



////////////////////// xModel
void   xModel :: Free()
{
    if (!this) return;

    if (this->FileName)
        delete[] this->FileName;
    xMaterial *currM = this->L_material, *nextM;
    while (currM)
    {
        nextM = currM->Next;
        currM->Free();
        currM = nextM;
    }
    xElement *currE = this->L_kids, *nextE;
    while (currE)
    {
        nextE = currE->Next;
        currE->Free();
        currE = nextE;
    }
    this->Spine.Clear();

    if (this->BVHierarchy)
    {
        this->BVHierarchy->free();
        delete this->BVHierarchy;
    }

    delete this;
}

xModel *xModel :: Load(const char *fileName, bool FL_create_CollisionInfo)
{
    FILE *file;
    file = fopen(fileName, "rb");
    if (file)
    {
        xModel *xmodel = new xModel();
        memset(xmodel, 0, sizeof(xModel));

        xmodel->FileName = strdup(fileName);
        if (xmodel)
        {
            xmodel->FL_textures_loaded = false;
            xmodel->FL_transparent     = false;
            xmodel->FL_opaque          = false;
            xmodel->FL_save_rdata      = false;
            bool FL_save_bvh           = false;

            xDWORD len;
            fread(&len, sizeof(len), 1, file);
            fread(&xmodel->FL_save_binfo, sizeof(xmodel->FL_save_binfo), 1, file);
            if (len > 1)
                fread(&FL_save_bvh, sizeof(FL_save_bvh), 1, file);
            if (len > 2)
                fread(&xmodel->FL_save_rdata, sizeof(xmodel->FL_save_rdata), 1, file);

            xmodel->L_material = NULL;
            xmodel->L_kids = NULL;

            fread(&xmodel->I_material, sizeof(xBYTE), 1, file);
            if (xmodel->I_material)
            {
                xMaterial *last = NULL;
                for (int i=0; i < xmodel->I_material; ++i)
                {
                    if (last)
                        last = last->Next = xMaterial::Load(file);
                    else
                        last = xmodel->L_material = xMaterial::Load(file);
                    if (!last)
                    {
                        xmodel->Free();
                        fclose(file);
                        return false;
                    }
                    bool transparent = last->transparency > 0.f;
                    xmodel->FL_transparent |= transparent;
                    xmodel->FL_opaque      |= !transparent;
                }
            }

            fread(&xmodel->I_kids, sizeof(xBYTE), 1, file);
            if (xmodel->I_kids)
            {
                xElement *last = NULL;
                for (int i=0; i < xmodel->I_kids; ++i)
                {
                    if (last)
                        last = last->Next   = xElement::Load(file, xmodel, FL_create_CollisionInfo);
                    else
                        last = xmodel->L_kids = xElement::Load(file, xmodel, FL_create_CollisionInfo);
                    if (!last)
                    {
                        xmodel->Free();
                        fclose(file);
                        return NULL;
                    }
                }
            }
            xmodel->I_elements = xmodel->L_kids->CountAll();

            bool skeletized;
            fread(&skeletized, sizeof(bool), 1, file);
            if (skeletized)
                xmodel->Spine.Load(file);

            if (FL_save_bvh)
            {
                xmodel->BVHierarchy = new xBVHierarchy();
                xmodel->BVHierarchy->Load(file);
            }

            if (xmodel->I_kids && FL_create_CollisionInfo && !xmodel->FL_save_binfo)
                xmodel->L_kids->FillBoundingData(*xmodel);
        }
        fclose(file);
        return xmodel;
    }
    return NULL;
}

void   xModel :: Save()
{
    FILE *file;
    file = fopen(this->FileName, "wb");
    if (file)
    {
        bool   FL_save_bvh = BVHierarchy != NULL;
        //bool   FL_save_smooth = true;
        this->FL_save_rdata = true;

        xDWORD len = 3;
        fwrite(&len, sizeof(len), 1, file);
        fwrite(&this->FL_save_binfo, sizeof(this->FL_save_binfo), 1, file);
        fwrite(&FL_save_bvh, sizeof(FL_save_bvh), 1, file);
        fwrite(&this->FL_save_rdata, sizeof(this->FL_save_rdata), 1, file);

        fwrite(&this->I_material, sizeof(xBYTE), 1, file);
        if (this->I_material)
        {
            xMaterial *last = this->L_material;
            for (; last; last = last->Next)
                last->Save(file);
        }

        fwrite(&this->I_kids, sizeof(xBYTE), 1, file);
        if (this->I_kids)
        {
            xElement *last = this->L_kids;
            for (; last; last = last->Next)
                last->Save(file, this);
        }

        // are the bones defined?
        bool skeletized = this->Spine.I_bones;
        fwrite(&skeletized, sizeof(bool), 1, file);
        if (skeletized)
            this->Spine.Save(file);

        if (FL_save_bvh)
            BVHierarchy->Save(file);

        fclose(file);
    }
}

#include "../../Math/Figures/xSphere.h"
using namespace Math::Figures;

void xModel::ReFillBVH(xBVHierarchy &BVH_node, xMeshData *&MeshData, const xMatrix &MX_LocalToWorld)
{
    xSphere &sphere = *(xSphere*) BVH_node.GetTransformed(MX_LocalToWorld);

    if (!this->I_kids)
        return;

    xBoxA boxA, boxAc;
    boxA.P_min.init(xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE);
    boxA.P_max.init(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);

    xElement *elem = L_kids;
    for (; elem; elem = elem->Next)
    {
        boxAc = elem->ReFillBVH( BVH_node.L_items, MeshData, MX_LocalToWorld );
        if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
        if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
        if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
        if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
        if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
        if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
    }

    xVector3 NW_extends = (boxA.P_max - boxA.P_min) * 0.5f;
    sphere.P_center = boxA.P_min + NW_extends;
    sphere.S_radius = NW_extends.length();
}

void xModel::CreateBVH(xBVHierarchy &BVH_node, xMeshData *&MeshData)
{
    BVH_node.init(*new xSphere());
    xSphere &sphere = *(xSphere*) BVH_node.Figure;

    if (!this->I_kids)
    {
        sphere.S_radius = 0.f;
        sphere.P_center.init(xFLOAT_HUGE_POSITIVE,xFLOAT_HUGE_POSITIVE,xFLOAT_HUGE_POSITIVE);
        return;
    }

    MeshData         = new xMeshData[I_elements];
    BVH_node.L_items = new xBVHierarchy[I_elements];
    BVH_node.I_items = I_elements;

    xBoxA boxA, boxAc;
    boxA.P_min.init(xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE, xFLOAT_HUGE_POSITIVE);
    boxA.P_max.init(xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE, xFLOAT_HUGE_NEGATIVE);

    xElement *elem = L_kids;
    for (; elem; elem = elem->Next)
    {
        boxAc = elem->FillBVH( BVH_node.L_items, MeshData );
        if (boxAc.P_min.x < boxA.P_min.x) boxA.P_min.x = boxAc.P_min.x;
        if (boxAc.P_min.y < boxA.P_min.y) boxA.P_min.y = boxAc.P_min.y;
        if (boxAc.P_min.z < boxA.P_min.z) boxA.P_min.z = boxAc.P_min.z;
        if (boxAc.P_max.x > boxA.P_max.x) boxA.P_max.x = boxAc.P_max.x;
        if (boxAc.P_max.y > boxA.P_max.y) boxA.P_max.y = boxAc.P_max.y;
        if (boxAc.P_max.z > boxA.P_max.z) boxA.P_max.z = boxAc.P_max.z;
    }

    xVector3 NW_extends = (boxA.P_max - boxA.P_min) * 0.5f;
    sphere.P_center = boxA.P_min + NW_extends;
    sphere.S_radius = NW_extends.length();
}

/* model instance */
void xModelInstance :: Zero()
{
    MX_bones     = NULL;
    QT_bones     = NULL;
    P_bone_roots = NULL;
    P_bone_trans = NULL;
    I_bones      = 0;
    FL_modified  = NULL;
    I_elements   = 0;
    L_elements   = NULL;
}

void xModelInstance :: ZeroElements()
{
    if (L_elements)
    {
        xElementInstance *iter = L_elements;
        for (int i = I_elements; i; --i, ++iter)
            iter->Zero();
    }
}

void xModelInstance :: ClearSkeleton()
{
    if (MX_bones)     { delete[] MX_bones;     MX_bones = NULL; }
    if (QT_bones)     { delete[] QT_bones;     QT_bones = NULL; }
    if (P_bone_roots) { delete[] P_bone_roots; P_bone_roots = NULL; }
    if (P_bone_trans) { delete[] P_bone_trans; P_bone_trans = NULL; }
    if (FL_modified)  { delete[] FL_modified;  FL_modified = NULL; }
}

void xModelInstance :: FreeVertices()
{
    ClearSkeleton();

    if (L_elements)
    {
        xElementInstance *iter = L_elements;
        for (; I_elements; --I_elements, ++iter)
            iter->Free();
    }
}

void xModelInstance :: Clear()
{
    ClearSkeleton();

    if (L_elements)
    {
        xElementInstance *iter = L_elements;
        for (; I_elements; --I_elements, ++iter)
            iter->Clear();

        delete[] L_elements;
        L_elements = NULL;
    }
}
