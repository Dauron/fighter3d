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
    bool       FL_save_bvh;        // Save bounding volumes hierarchy?
    bool       FL_transparent;     // Are there any transparent faces?
    bool       FL_opaque;          // Are there any opaque faces?

    void   Free();
    void   BoneDelete( xBYTE ID_bone );
    void   SkeletonAdd();
    void   SkeletonReset();

    void CreateBVH(Math::Figures::xBVHierarchy &BVH_node);

    static xModel *Load( const char *fileName, bool FL_create_CollisionInfo = true );
           void    Save();
};

struct xModelInstance {
    xMatrix       MX_LocalToWorld;
    xPoint3       P_center;

    xBYTE         I_bones;
    xMatrix     * MX_bones;
    xQuaternion * QT_bones;
    bool        * FL_modified;

    xElementInstance  *L_elements;
    xBYTE              I_elements;

    void Zero();
    void ZeroElements();
    void Clear();
    void ClearSkeleton();
};

#endif
