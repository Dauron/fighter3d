#ifndef __incl_Physics_FigureCollider_h
#define __incl_Physics_FigureCollider_h

#include "../../Math/Figures/xIFigure3d.h"
#include "CollisionInfo.h"

namespace Physics { namespace Colliders {
    using namespace ::Math::Figures;

    struct FigureCollider {

        bool          Test   (const xIFigure3d *figure1, const xIFigure3d *figure2);
        CollisionInfo Collide(const xIFigure3d *figure1, const xIFigure3d *figure2);

    };

} } // namespace Physics.Colliders

#endif