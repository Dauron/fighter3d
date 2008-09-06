#ifndef __incl_xModel_h
#define __incl_xModel_h

#include "xSkeleton.h"
#include "xMaterial.h"
#include "xElement.h"

struct xModel {
    char      *FileName;

    xMaterial *L_material; // first material
    xBYTE      I_material; // no of materials

    xElement  *L_kids;     // model's elements
    xBYTE      I_kids;     // model's elements
    xBYTE      I_elements; // count of all kids

    xSkeleton  Spine;

    bool       FL_textures_loaded; // Are textures handles loaded from the TextureManager
    bool       FL_save_rdata;      // Save render data info?
    bool       FL_save_binfo;      // Save bounding info?
    bool       FL_transparent;     // Are there any transparent faces?
    bool       FL_opaque;          // Are there any opaque faces?

    xBVHierarchy *BVHierarchy;

    void Free();
    void BoneDelete( xBYTE ID_bone );
    void SkeletonAdd();
    void SkeletonReset();

    void CreateBVH(Math::Figures::xBVHierarchy &BVH_node, Math::Figures::xMeshData *&MeshData);
    void ReFillBVH(Math::Figures::xBVHierarchy &BVH_node, Math::Figures::xMeshData *&MeshData, const xMatrix &MX_LocalToWorld );

    static xModel *Load( const char *fileName, bool FL_create_CollisionInfo = true );
           void    Save();
};

struct xModelInstance {
    xMatrix       MX_LocalToWorld;
    xPoint3       P_center;

    xBYTE         I_bones;
    xMatrix     * MX_bones;
    xQuaternion * QT_bones;
    xPoint3     * P_bone_roots;
    xPoint3     * P_bone_trans;
    bool        * FL_modified;

    // flattened hierarchy indexed by element ID
    xElementInstance  *L_elements;
    xBYTE              I_elements;

    void Zero();
    void ZeroElements();
    void FreeVertices();
    void Clear();
    void ClearSkeleton();
};

#endif
