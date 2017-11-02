#ifndef __incl_Math_Tracking_TrackedObject_h
#define __incl_Math_Tracking_TrackedObject_h

#include "../Figures/xSphere.h"

namespace Math { namespace Tracking {
    using namespace ::Math::Figures;

    struct TrackedObject : public xSphere
    {
    private:
        xMatrix MX_LocalToWorld;

    public:
        xPoint3 P_center_Trfm;
        xBYTE   Type;

        TrackedObject()
        {}
        TrackedObject(const xMatrix &mx_LocalToWorld,
                      const xPoint3 &p_center,
                      xFLOAT s_radius = 0.f)
            : MX_LocalToWorld(mx_LocalToWorld)
        {
            P_center = p_center;
            S_radius = s_radius;
            Type     = 0;
            P_center_Trfm.zero();
        }

        virtual const xMatrix &MX_LocalToWorld_Get() const
        { return MX_LocalToWorld; }
        virtual       xMatrix &MX_LocalToWorld_Set()
        { return MX_LocalToWorld; }

        virtual TrackedObject &GetSubObject(xBYTE ID_sub)
        {
            return *this;
        }
    };

} } // namespace Math.Tracking

#endif
