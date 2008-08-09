#ifndef __incl_Physics_BVHCollider_h
#define __incl_Physics_BVHCollider_h

#include "xBVHierarchy.h"
#include "FigureCollider.h"
#include <vector>

namespace Physics { namespace Colliders {
    using namespace ::Physics::Colliders;
    using namespace ::Math::Figures;

    typedef std::vector<CollisionInfo> CollisionSet;

    struct BVHCollider {

        CollisionSet Collide(xBVHierarchy &bvh1, xBVHierarchy &bvh2,
                             const xMatrix &MX_LocalToWorld1,
                             const xMatrix &MX_LocalToWorld2);

    private:
        FigureCollider FCollider;

        const xMatrix *MX_LocalToWorld1;
        const xMatrix *MX_LocalToWorld2;

        void CollideLevel (xBVHierarchy &bvh1, xBVHierarchy &bvh2,
                           CollisionSet &collisionSet);
    };

} } // namespace Physics.Colliders

#endif
