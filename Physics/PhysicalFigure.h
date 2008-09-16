#ifndef __incl_Physics_PhysicalFigure_h
#define __incl_Physics_PhysicalFigure_h

#include "PhysicalBody.h"
#include "../Math/Figures/xIFigure3d.h"

namespace Physics {
    using namespace Math::Figures;

    class PhysicalFigure : public PhysicalBody
    {
    public:

        void Create(xIFigure3d *figure)
        {
            BVHierarchy.init(*figure);
        }

        virtual void Create()
        {
            PhysicalBody::Create();
        }

        virtual void Update(xFLOAT T_time)
        {
            P_center = BVHierarchy.Figure->P_center;
            PhysicalBody::Update(T_time);

            if (IsModified())
                BVHierarchy.GetTransformed(MX_LocalToWorld_Get());
        }

        virtual void ApplyDefaults()
        {
            xBVHierarchy bvh = BVHierarchy;
            PhysicalBody::ApplyDefaults();
            BVHierarchy = bvh;

            M_mass = BVHierarchy.Figure->W_Volume_Get();
            if (BVHierarchy.Figure->Type != xIFigure3d::Capsule)
                S_radius = 2.f * sqrt(BVHierarchy.Figure->S_Radius_Sqr_Get());
            else
                S_radius = 0.75 * ((xCapsule*) BVHierarchy.Figure)->S_radius
                         + 0.25 * ((xCapsule*) BVHierarchy.Figure)->S_top;
        }
    };

} // namespace Physics

#endif
