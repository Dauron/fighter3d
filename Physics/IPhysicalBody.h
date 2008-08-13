#ifndef __incl_Physics_IPhysicalBody_h
#define __incl_Physics_IPhysicalBody_h

#include "../Math/xMath.h"
#include "../Math/Figures/xBVHierarchy.h"

namespace Physics {
    using namespace Math::Figures;

    class IPhysicalBody
    {
    private:
        bool         FL_initialized;
        bool         FL_stationary;

    protected:
        bool         IsInitialized() { return FL_initialized; }

    public:
        xBVHierarchy BVHierarchy;

        bool         IsStationary() { return FL_stationary; }

    public:
        virtual void     Stop() = 0;

        virtual xFLOAT   GetMass() = 0;
        virtual const xMatrix &MX_LocalToWorld_Get() const = 0;
        virtual       xMatrix &MX_LocalToWorld_Set() = 0;

        virtual xVector3 GetVelocity() = 0;
        virtual xVector3 GetVelocity(xPoint3 P_point) = 0;

        virtual xVector3 GetForce(xFLOAT T_time_inv) = 0;
        virtual xVector3 GetForce(xFLOAT T_time_inv, xPoint3 P_point) = 0;

        virtual void     ApplyAcceleration(xVector3 NW_accel, xFLOAT T_time) = 0;
        virtual void     ApplyAcceleration(xVector3 NW_accel, xFLOAT T_time, xPoint3 P_point) = 0;

        virtual void     ApplyForce(xVector3 NW_force, xFLOAT T_time) = 0;
        virtual void     ApplyForce(xVector3 NW_force, xFLOAT T_time, xPoint3 P_point) = 0;

        virtual void     Initialize()               { FL_initialized = true; FL_stationary = false; }
        virtual void     FrameStart()               {}
        virtual void     FrameUpdate(xFLOAT T_time) {}
        virtual void     FrameRender()              {}
        virtual void     FrameEnd()                 {}
        virtual void     Finalize()                 { FL_initialized = false; }
    };

} // namespace Physics

#endif
