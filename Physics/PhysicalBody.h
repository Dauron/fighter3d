#ifndef __incl_Physics_PhysicalBody_h
#define __incl_Physics_PhysicalBody_h

#include "IPhysicalBody.h"
#include "Constants.h"

namespace Physics {

    class PhysicalBody : public IPhysicalBody
    {
    public:
        xVector3    NW_velocity;
        xVector3    NW_velocity_new;
        // REWRITE: Should be sth. different to store velocity of rotation.
        //          QT may only store up to 720 degrees of rotation, faster rotations can't exist.
        xQuaternion QT_velocity;
        xQuaternion QT_velocity_new;

        xFLOAT      M_mass;

    public:

        virtual void Stop()
        {
            NW_velocity.zero();
            QT_velocity.zeroQ();
            NW_velocity_new.zero();
            QT_velocity_new.zeroQ();
        }

        virtual xFLOAT   GetMass() const { return M_mass; }

        virtual xVector3 GetVelocity() const
        { return NW_velocity; }
        virtual xVector3 GetVelocity(const CollisionPoint &CP_point) const
        { return NW_velocity + xVector3::CrossProduct(QT_velocity.angularVelocity(), CP_point.P_collision - P_center_Trfm); }

        virtual xVector3 GetForce(xFLOAT T_time_inv) const
        { return GetVelocity() * GetMass() * T_time_inv; }
        virtual xVector3 GetForce(xFLOAT T_time_inv, const CollisionPoint &CP_point) const
        { return GetVelocity(CP_point) * GetMass() * T_time_inv; }

        virtual void     ApplyFix(const CollisionPoint &CP_point)
        { MX_LocalToWorld_Set().postTranslateT(CP_point.NW_fix * CP_point.W_fix); }

        virtual void     ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time)
        { NW_velocity_new += NW_accel * T_time; }
        virtual void     ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time, const CollisionPoint &CP_point);

        virtual void     ApplyForce(const xVector3 &NW_force, xFLOAT T_time)
        { ApplyAcceleration(NW_force / GetMass(), T_time); }
        virtual void     ApplyForce(const xVector3 &NW_force, xFLOAT T_time, const CollisionPoint &CP_point)
        { ApplyAcceleration(NW_force / GetMass(), T_time, CP_point); }

    public:
        virtual void LocationChanged() { BVHierarchy.invalidateTransformation(); }

        void Translate(xFLOAT x, xFLOAT y, xFLOAT z)
        {
            Stop();
            MX_LocalToWorld_Set() *= xMatrixTranslateT(x,y,z);
            if (IsCreated()) LocationChanged();
        }

        void Rotate(xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ)
        {
            Stop();
            MX_LocalToWorld_Set() *= xMatrixRotateRad(DegToRad(rotX), DegToRad(rotY), DegToRad(rotZ));
            if (IsCreated()) LocationChanged();
        }

    public:
        virtual void ApplyDefaults()
        {
            IPhysicalBody::ApplyDefaults();
            M_mass      = FL_stationary ? xFLOAT_HUGE_POSITIVE : 10.f;
            Stop();
        }
        virtual void Update(xFLOAT T_time);
    };

} // namespace Physics

#endif
