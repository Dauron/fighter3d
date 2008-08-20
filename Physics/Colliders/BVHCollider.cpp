#include "BVHCollider.h"
using namespace ::Physics::Colliders;

xDWORD BVHCollider :: Collide(IPhysicalBody *body1, IPhysicalBody *body2,
                              xBVHierarchy &bvh1, xBVHierarchy &bvh2,
                              const xMatrix &MX_LocalToWorld1,
                              const xMatrix &MX_LocalToWorld2,
                              CollisionSet &cset)
{
    if ( !bvh1.I_items && !bvh2.I_items ) // leafs - perform full collision query
    {
        return FCollider.Collide( body1, body2,
                                  bvh1.GetTransformed(MX_LocalToWorld1),
                                  bvh2.GetTransformed(MX_LocalToWorld2), cset );
    }

    if ( ! FCollider.Test( bvh1.GetTransformed(MX_LocalToWorld1),
                           bvh2.GetTransformed(MX_LocalToWorld2) ) )
        return false;

    xDWORD I_cols = 0;

    if ( !bvh1.I_items )
    {
        xBVHierarchy *bvh2_child = bvh2.L_items;
        for (int i = bvh2.I_items; i; --i, ++bvh2_child)
            I_cols += Collide ( body1, body2, bvh1, *bvh2_child, bvh1.MX_LocalToWorld_Get(), bvh2.MX_RawToWorld_Get(), cset );
        return I_cols;
    }

    if ( !bvh2.I_items )
    {
        xBVHierarchy *bvh1_child = bvh1.L_items;
        for (int i = bvh1.I_items; i; --i, ++bvh1_child)
            I_cols += Collide ( body1, body2, *bvh1_child, bvh2, bvh1.MX_RawToWorld_Get(), bvh2.MX_LocalToWorld_Get(), cset );
        return I_cols;
    }

    xBVHierarchy *bvh1_child = bvh1.L_items;
    xBVHierarchy *bvh2_child;
    for (int i = bvh1.I_items; i; --i, ++bvh1_child)
    {
        bvh2_child = bvh2.L_items;
        for (int j = bvh2.I_items; j; --j, ++bvh2_child)
            I_cols += Collide ( body1, body2, *bvh1_child, *bvh2_child, bvh1.MX_RawToWorld_Get(), bvh2.MX_RawToWorld_Get(), cset );
    }
    return I_cols;
}
