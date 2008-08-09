#include "BVHCollider.h"
using namespace ::Physics::Colliders;

void BVHCollider :: CollideLevel (xBVHierarchy &bvh1, xBVHierarchy &bvh2,
                                  CollisionSet &collisionSet)
{
    if ( !bvh1.I_items && !bvh2.I_items ) // leafs - perform full collision query
    {
        CollisionInfo ci = FCollider.Collide( bvh1.GetTransformed(*MX_LocalToWorld1),
                                              bvh2.GetTransformed(*MX_LocalToWorld2) );
        if (ci.FL_collided) collisionSet.push_back(ci);
        return;
    }

    if ( ! FCollider.Test( bvh1.GetTransformed(*MX_LocalToWorld1),
                           bvh2.GetTransformed(*MX_LocalToWorld2) ) )
        return;

    if ( !bvh1.I_items )
    {
        xBVHierarchy *bvh2_child = bvh2.L_items;
        for (int i = bvh2.I_items; i; --i, ++bvh2_child)
            CollideLevel ( bvh1, *bvh2_child, collisionSet );
        return;
    }

    if ( !bvh2.I_items )
    {
        xBVHierarchy *bvh1_child = bvh1.L_items;
        for (int i = bvh1.I_items; i; --i, ++bvh1_child)
            CollideLevel ( *bvh1_child, bvh2, collisionSet );
        return;
    }

    xBVHierarchy *bvh1_child = bvh1.L_items;
    xBVHierarchy *bvh2_child;
    for (int i = bvh1.I_items; i; --i, ++bvh1_child)
    {
        bvh2_child = bvh2.L_items;
        for (int j = bvh2.I_items; j; --j, ++bvh2_child)
            CollideLevel ( *bvh1_child, *bvh2_child, collisionSet );
    }
}

CollisionSet BVHCollider :: Collide(xBVHierarchy &bvh1, xBVHierarchy &bvh2,
                                    const xMatrix &MX_LocalToWorld1, const xMatrix &MX_LocalToWorld2)
{
    this->MX_LocalToWorld1 = &MX_LocalToWorld1;
    this->MX_LocalToWorld2 = &MX_LocalToWorld2;

    CollisionSet collisionSet;
    CollideLevel(bvh1, bvh2, collisionSet);
    return collisionSet;
}
