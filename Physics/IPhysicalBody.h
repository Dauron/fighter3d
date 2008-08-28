#ifndef __incl_Physics_IPhysicalBody_h
#define __incl_Physics_IPhysicalBody_h

#include "../Math/xMath.h"
#include "../Math/Figures/xBVHierarchy.h"
#include "../Math/Tracking/TrackedObject.h"
#include "Colliders/CollisionInfo.h"
#include <vector>

namespace Physics {
    using namespace Math::Figures;
    using namespace Math::Tracking;
    using namespace Physics::Colliders;

    class IPhysicalBody : public TrackedObject
    {
    private:
        bool         FL_defaults_applied;
        bool         FL_initialized;
        bool         FL_modified;

    protected:
        bool         IsDefaultsApplied() const { return FL_defaults_applied; }
        bool         IsInitialized()     const { return FL_initialized; }
        bool         IsModified()        const { return FL_modified; }

    public:
        bool         FL_stationary; // no movement
        bool         FL_phantom;    // no collisions
        bool         FL_physical;   // affected by gravity?
        float        W_restitution; // how much energy will the object pass during collisions
        float        W_restitution_self; // how much energy will the object retain during collisions

        std::vector<CollisionPoint> Collisions;

        xBVHierarchy BVHierarchy;

    public:
        IPhysicalBody &Modify() { FL_modified = true; return *this; }

        virtual       xMatrix &MX_LocalToWorld_Set()
        { FL_modified = true; return TrackedObject::MX_LocalToWorld_Set(); }

        virtual void     Stop() = 0;

        virtual xFLOAT   GetMass() const = 0;

        virtual xVector3 GetVelocity() const = 0;
        virtual xVector3 GetVelocity(const CollisionPoint &CP_point) const = 0;

        virtual xVector3 GetForce(xFLOAT T_time_inv) const = 0;
        virtual xVector3 GetForce(xFLOAT T_time_inv, const CollisionPoint &CP_point) const = 0;

        virtual void     ApplyFix(const CollisionPoint &CP_point) = 0;
        virtual void     ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time) = 0;
        virtual void     ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time, const CollisionPoint &CP_point) = 0;

        virtual void     ApplyForce(const xVector3 &NW_force, xFLOAT T_time) = 0;
        virtual void     ApplyForce(const xVector3 &NW_force, xFLOAT T_time, const CollisionPoint &CP_point) = 0;

        IPhysicalBody() : FL_defaults_applied(false), FL_initialized(false) { BVHierarchy.zero(); }

        virtual void     ApplyDefaults() {
            FL_defaults_applied = true;
            FL_stationary       = false;
            FL_phantom          = false;
            FL_physical         = true;
            FL_modified         = true;
            W_restitution       = 0.5f;
            W_restitution_self  = 0.5f;
            S_radius            = 1.f;
            P_center.zero(); P_center_Trfm.zero();
            TrackedObject::MX_LocalToWorld_Set().identity();
        }
        virtual void     Initialize()
        {
            if (!IsDefaultsApplied()) ApplyDefaults();
            FL_initialized = true;
            FL_modified = true;
        }
        virtual void     FrameStart()               {}
        virtual void     FrameUpdate(xFLOAT T_time) { if  (IsModified()) P_center_Trfm = MX_LocalToWorld_Get().preTransformP(P_center); }
        virtual void     FrameRender()              {}
        virtual void     FrameEnd()                 { FL_modified = false; }
        virtual void     Finalize()                 { FL_initialized = false; BVHierarchy.free(); BVHierarchy.zero(); }
    };

} // namespace Physics

#endif
