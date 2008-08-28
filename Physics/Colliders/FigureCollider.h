#ifndef __incl_Physics_Colliders_FigureCollider_h
#define __incl_Physics_Colliders_FigureCollider_h

#include "CollisionInfo.h"
#include "../IPhysicalBody.h"

namespace Physics { namespace Colliders {
    using namespace ::Physics;
    using namespace ::Math::Figures;

    struct FigureCollider {

        bool   Test   (const xIFigure3d *figure1, const xIFigure3d *figure2);
        xDWORD Collide(IPhysicalBody *body1,      IPhysicalBody *body2,
                       const xIFigure3d *figure1, const xIFigure3d *figure2, CollisionSet &cs);

    };

} } // namespace Physics.Colliders

#endif
