#ifndef __incl_Physics_Colliders_BVHCollider_h
#define __incl_Physics_Colliders_BVHCollider_h

#include "../../Math/Figures/xBVHierarchy.h"
#include "FigureCollider.h"
#include <vector>

namespace Physics { namespace Colliders {
    using namespace ::Physics::Colliders;
    using namespace ::Math::Figures;

    struct BVHCollider {

        xDWORD Collide(xBVHierarchy &bvh1, xBVHierarchy &bvh2,
                       const xMatrix &MX_LocalToWorld1,
                       const xMatrix &MX_LocalToWorld2,
                       CollisionSet &cset);

    private:
        FigureCollider FCollider;
    };

} } // namespace Physics.Colliders

#endif
