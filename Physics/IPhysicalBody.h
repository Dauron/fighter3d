#ifndef __incl_Physics_IPhysicalBody_h
#define __incl_Physics_IPhysicalBody_h

#include "../Math/xMath.h"
#include "../Math/Figures/xBVHierarchy.h"
#include "Colliders/CollisionInfo.h"
#include <vector>

namespace Physics {
    using namespace Math::Figures;
    using namespace Physics::Colliders;

    class IPhysicalBody
    {
    private:
        bool         FL_defaults_applied;
        bool         FL_initialized;

    protected:
        bool         IsDefaultsApplied() { return FL_defaults_applied; }
        bool         IsInitialized()     { return FL_initialized; }

    public:
        bool         FL_stationary; // no movement
        bool         FL_phantom;    // no collisions
        bool         FL_physical;   // affected by gravity?
        float        W_restitution; // how much energy will the object pass during collisions
        float        W_restitution_self; // how much energy will the object retain during collisions

        std::vector<CollisionPoint> Collisions;

        xBVHierarchy BVHierarchy;

    public:
        virtual void     Stop() = 0;

        virtual xFLOAT   GetMass() const = 0;
        virtual const xMatrix &MX_LocalToWorld_Get() const = 0;
        virtual       xMatrix &MX_LocalToWorld_Set() = 0;

        virtual xVector3 GetVelocity() const = 0;
        virtual xVector3 GetVelocity(const CollisionPoint &CP_point) const = 0;

        virtual xVector3 GetForce(xFLOAT T_time_inv) const = 0;
        virtual xVector3 GetForce(xFLOAT T_time_inv, const CollisionPoint &CP_point) const = 0;

        virtual void     ApplyFix(const CollisionPoint &CP_point) = 0;
        virtual void     ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time) = 0;
        virtual void     ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time, const CollisionPoint &CP_point) = 0;

        virtual void     ApplyForce(const xVector3 &NW_force, xFLOAT T_time) = 0;
        virtual void     ApplyForce(const xVector3 &NW_force, xFLOAT T_time, const CollisionPoint &CP_point) = 0;

        IPhysicalBody() : FL_initialized(false), FL_defaults_applied(false) { BVHierarchy.zero(); }

        virtual void     ApplyDefaults() {
            FL_defaults_applied = true;
            FL_stationary       = false;
            FL_phantom          = false;
            FL_physical         = true;
            W_restitution       = 0.5f;
            W_restitution_self  = 0.5f;
        }
        virtual void     Initialize()
        {
            if (!IsDefaultsApplied()) ApplyDefaults();
            FL_initialized = true;
        }
        virtual void     Invalidate()               {}
        virtual void     FrameStart()               {}
        virtual void     FrameUpdate(xFLOAT T_time) {}
        virtual void     FrameRender()              {}
        virtual void     FrameEnd()                 {}
        virtual void     Finalize()                 { FL_initialized = false; BVHierarchy.free(); BVHierarchy.zero(); }
    };

} // namespace Physics

#endif
