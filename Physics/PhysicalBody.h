#ifndef __incl_Physics_PhysicalBody_h
#define __incl_Physics_PhysicalBody_h

#include "IPhysicalBody.h"
#include "Constants.h"

namespace Physics {

    class PhysicalBody : public IPhysicalBody
    {
    private:
        xMatrix     MX_LocalToWorld;
        bool        FL_modified;

    public:
        xVector3    NW_velocity;
        xVector3    NW_velocity_new;
        // REWRITE: Should be sth. different to store velocity of rotation.
        //          QT may only store up to 720 degrees of rotation, faster rotations can't exist.
        xQuaternion QT_velocity;
        xQuaternion QT_velocity_new;

        xPoint3     P_center;
        xPoint3     P_center_Trfm;
        xFLOAT      S_radius;

        xFLOAT      M_mass;

        PhysicalBody &Modify() { FL_modified = true; return *this; }
        bool          IsModified() { return FL_modified; }

        virtual const xMatrix &MX_LocalToWorld_Get() const
        {
            return MX_LocalToWorld;
        }
        virtual       xMatrix &MX_LocalToWorld_Set()
        {
            return Modify().MX_LocalToWorld;
        }

    public:

        virtual void Stop()
        {
            NW_velocity.zero();
            QT_velocity.zeroQ();
            NW_velocity_new.zero();
            QT_velocity_new.zeroQ();
        }

        virtual xFLOAT   GetMass() { return M_mass; }

        virtual xVector3 GetVelocity()
        { return NW_velocity; }
        virtual xVector3 GetVelocity(xPoint3 P_point)
        {
            return NW_velocity + xVector3::CrossProduct(QT_velocity.angularVelocity(), P_point - P_center_Trfm);
        }

        virtual xVector3 GetForce(xFLOAT T_time_inv)
        { return GetVelocity() * GetMass() * T_time_inv; }
        virtual xVector3 GetForce(xFLOAT T_time_inv, xPoint3 P_point)
        { return GetVelocity(P_point) * GetMass() * T_time_inv; }

        virtual void     ApplyAcceleration(xVector3 NW_accel, xFLOAT T_time)
        { NW_velocity_new += NW_accel * T_time; }
        virtual void     ApplyAcceleration(xVector3 NW_accel, xFLOAT T_time, xPoint3 P_point)
        {
            xVector3 N_arm = P_point - P_center_Trfm;
            xFLOAT   S_arm = N_arm.length();
            N_arm /= S_arm;

            xFLOAT   W_cos_arm = fabs(xVector3::DotProduct( N_arm, xVector3::Normalize(NW_accel) ));
            xFLOAT   W_arm;
            if (S_arm < S_radius)
                W_cos_arm = S_arm / S_radius * ( 1.f - W_cos_arm );
            else
                W_cos_arm = 1.f - W_cos_arm;

            xVector3 NW_vel = NW_accel * T_time;
            // linear velocity
            NW_velocity_new += NW_vel * (1 - W_cos_arm); // a * (1 -  x * (1 - |cosA|))
            // angular velocity
            QT_velocity_new = xQuaternion::product(QT_velocity_new, xQuaternion::angularVelocity( // a * x * (1 - |cosA|)
                                                                      xVector3::CrossProduct(NW_vel*W_cos_arm, N_arm)));
        }

        virtual void     ApplyForce(xVector3 NW_force, xFLOAT T_time)
        { ApplyAcceleration(NW_force / GetMass(), T_time); }
        virtual void     ApplyForce(xVector3 NW_force, xFLOAT T_time, xPoint3 P_point)
        { ApplyAcceleration(NW_force / GetMass(), T_time, P_point); }

        virtual void Initialize()
        {
            IPhysicalBody::Initialize();
            FL_modified = true;
            MX_LocalToWorld.identity();
            P_center.zero(); P_center_Trfm.zero();
            M_mass = 10.f;
            S_radius = 1.f;
            Stop();
        }

        virtual void FrameUpdate(xFLOAT T_time)
        {
            if (!IsStationary())
            {
                NW_velocity = NW_velocity_new;
                if (T_time != 0 && !NW_velocity.isZero())
                {
                    MX_LocalToWorld.postTranslateT(NW_velocity * T_time);
                    FL_modified = true;

                    //drag
                    xFLOAT V_vel = NW_velocity.length();
                    xFLOAT W_air_drag = air_drag(S_radius, V_vel*V_vel) * T_time / M_mass;
                    if (V_vel > W_air_drag)
                        NW_velocity_new -= W_air_drag * NW_velocity / V_vel;
                    else
                        NW_velocity_new.zero();
                }
            }

            if  (FL_modified) P_center_Trfm = MX_LocalToWorld.preTransformP(P_center);

            if (!IsStationary())
            {
                QT_velocity = QT_velocity_new;
                if (T_time != 0 && QT_velocity.w != 1.f)
                {
                    MX_LocalToWorld *= xMatrixFromQuaternion(xQuaternion::interpolateFull(QT_velocity, T_time))
                        .preTranslateT(-P_center_Trfm).postTranslateT(P_center_Trfm);
                    FL_modified = true;

                    //drag
                    xFLOAT V_omega = QT_velocity.angularVelocity().length();
                    xFLOAT W_air_drag = air_drag(S_radius, V_omega*V_omega * S_radius*S_radius * 0.25f)
                        * T_time / M_mass;
                    if (V_omega > W_air_drag)
                        QT_velocity_new = xQuaternion::interpolateFull(QT_velocity, 1.f - W_air_drag / V_omega);
                    else
                        QT_velocity_new.zeroQ();
                }
            }
        }

        virtual void FrameEnd() { FL_modified = false; }
    };

} // namespace Physics

#endif
