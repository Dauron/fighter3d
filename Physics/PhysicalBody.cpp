#include "PhysicalBody.h"

using namespace Physics;

void PhysicalBody :: ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time, const CollisionPoint &CP_point)
{
    xVector3 N_arm = CP_point.P_collision - P_center_Trfm;
    xFLOAT   S_arm = N_arm.length();
    N_arm /= S_arm;

    xFLOAT   W_cos_arm = /*acos( */fabs(xVector3::DotProduct( N_arm, xVector3::Normalize(NW_accel) ) )/* ) * PI_inv * 2.f*/;
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
        if (Collisions.size())
        {
            std::vector<CollisionPoint>::iterator iter_cur = Collisions.begin(),
                                                  iter_end = Collisions.end();

            xPoint3 P_mean_support; P_mean_support.zero();
            xFLOAT  W_sum = 0.f;

            for (; iter_cur != iter_end; ++iter_cur)
            {
                xFLOAT W_cos = xVector3::DotProduct(iter_cur->V_response.normalize(), NW_velocity_new);
                if (W_cos < 0.f)
                {
                    W_sum += W_cos;
                    P_mean_support += iter_cur->P_collision * W_cos;
                }
            }
            if (W_sum < 0.f)
            {
                P_mean_support /= W_sum;
                W_sum /= Collisions.size();
                ApplyAcceleration(NW_velocity_new * W_sum, T_time, CollisionPoint(P_mean_support));
            }
        }
    }

    if (!FL_stationary)
    {
        if (!Collisions.size())
        {
            if (T_time != 0)
            {
                //drag
                xFLOAT V_vel = NW_velocity.length();
                xFLOAT W_air_drag = air_drag(S_radius, V_vel*V_vel) * T_time / M_mass;
                if (V_vel > W_air_drag)
                    NW_velocity_new += NW_velocity * (1.f - W_air_drag / V_vel);
            }
            else
                NW_velocity_new += NW_velocity;
        }
        NW_velocity = NW_velocity_new;
        if (!NW_velocity.isZero())
        {
            if (T_time != 0)
            {
                MX_LocalToWorld.postTranslateT(NW_velocity * T_time);
                FL_modified = true;
            }
            NW_velocity_new.zero();
        }
    }

    if  (FL_modified) P_center_Trfm = MX_LocalToWorld.preTransformP(P_center);

    if (!FL_stationary)
    {
        if (!Collisions.size())
        {
            if (T_time != 0)
            {
                //drag
                xFLOAT V_omega = QT_velocity.angularVelocity().length();
                xFLOAT W_air_drag = air_drag(S_radius, V_omega*V_omega * S_radius*S_radius * 0.25f)
                    * T_time / M_mass;
                if (V_omega > W_air_drag)
                    QT_velocity_new = xQuaternion::product(
                                            xQuaternion::interpolateFull(QT_velocity, 1.f - W_air_drag / V_omega),
                                            QT_velocity_new);
            }
            else
                QT_velocity_new = xQuaternion::product(QT_velocity, QT_velocity_new);
        }
        QT_velocity = QT_velocity_new;
        if (QT_velocity.w != 1.f)
        {
            if (T_time != 0)
            {
                MX_LocalToWorld *= xMatrixFromQuaternion(xQuaternion::interpolateFull(QT_velocity, T_time))
                    .preTranslateT(-P_center_Trfm).postTranslateT(P_center_Trfm);
                FL_modified = true;
            }
            QT_velocity_new.zeroQ();
        }
    }

    if (IsModified()) BVHierarchy.invalidateTransformation();
}
