#include "PhysicalBody.h"

using namespace Physics;

void PhysicalBody :: ApplyAcceleration(xVector3 NW_accel, xFLOAT T_time, xPoint3 P_point)
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

void PhysicalBody :: FrameUpdate(xFLOAT T_time)
{
    if (!FL_stationary)
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

    if (!FL_stationary)
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

    if (IsModified()) BVHierarchy.invalidateTransformation();
}
