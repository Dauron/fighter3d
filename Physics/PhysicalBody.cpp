#include "PhysicalBody.h"

using namespace Physics;

void PhysicalBody :: ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time, const CollisionPoint &CP_point)
{
    xVector3 NW_vel = NW_accel * T_time;
    xFLOAT   V_tot  = NW_vel.lengthSqr();
    if (V_tot < EPSILON) return;
    V_tot = sqrt(V_tot);

    xVector3 N_arm = CP_point.P_collision - P_center_Trfm;
    xFLOAT   S_arm = N_arm.length();
/*
    xVector3 NW_rot = xVector3::CrossProduct(NW_vel, N_arm / S_radius);
    xFLOAT   V_rot  = NW_rot.length();
    NW_vel *= 1.f - V_rot / V_tot;

    NW_velocity_new += NW_vel;
    QT_velocity_new = xQuaternion::Product(QT_velocity_new, xQuaternion::AngularVelocity(NW_rot));
*/
    N_arm /= S_arm;

    xPlane  PN_hit; PN_hit.init(xVector3::Normalize(NW_accel), CP_point.P_collision);
    xPoint3 P_cast = PN_hit.castPoint( P_center_Trfm );
    S_arm = 2.f*(CP_point.P_collision - P_cast).length();

    xFLOAT   W_cos_arm = fabs(xVector3::DotProduct( N_arm, NW_vel/V_tot ) );
    W_cos_arm = W_cos_arm > 1.f-EPSILON3
        ? 1.f
        : 1.f - acos( W_cos_arm ) * PI_inv * 4.f;
    /*if (W_cos_arm < 0.f)*/ W_cos_arm = 0;
    if (S_arm < S_radius)
        W_cos_arm = S_arm / S_radius * ( 1.f - W_cos_arm );
    else
        W_cos_arm = 1.f - W_cos_arm;

    // linear velocity
    NW_velocity_new += NW_vel * (1.f - W_cos_arm); // a * (1 -  x * (1 - |cosA|))
    // angular velocity
    xQuaternion QT_now = xQuaternion::AngularVelocity( // a * x * (1 - |cosA|)
                                                       xVector3::CrossProduct(NW_vel, N_arm).normalize()*(V_tot*W_cos_arm));
    QT_velocity_new = xQuaternion::Product(QT_velocity_new, QT_now);
}

void PhysicalBody :: FrameUpdate(xFLOAT T_time)
{
    if (!FL_stationary)
    {
        if (T_time != 0)
        {
            //drag
            xFLOAT V_vel = NW_velocity.length();
            xFLOAT W_air_drag = air_drag(S_radius, V_vel*V_vel) * T_time / M_mass;
            if (V_vel > W_air_drag)
                NW_velocity *= (1.f - W_air_drag / V_vel);
        }

        if (Collisions.size())
        {
            std::vector<CollisionPoint>::iterator iter_cur, iter_end = Collisions.end();

            xPoint3 P_mean_support; P_mean_support.zero();
            xFLOAT  W_sum = 0.f;
            xFLOAT  W_colCount_Inv = 1.f / Collisions.size();

            IPhysicalBody *offender = NULL;
            xVector3       V_reaction;  V_reaction.zero();
            xPoint3        P_collision; P_collision.zero();
            xBYTE          I_collision = 0;

            for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
            {
                if (offender != iter_cur->Offender)
                {
                    if (offender != NULL)
                    {
                        ApplyAcceleration(V_reaction, 1.f, CollisionPoint(offender, P_collision / I_collision));
                        V_reaction.zero();
                        I_collision = 0;
                    }
                    offender = iter_cur->Offender;
                }
                V_reaction  += iter_cur->V_reaction;
                P_collision += iter_cur->P_collision;
                ++I_collision;
            }
            if (offender != NULL)
                ApplyAcceleration(V_reaction, 1.f, CollisionPoint(offender, P_collision / I_collision));

            for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
            {
                iter_cur->W_fix *= W_colCount_Inv;
                ApplyFix(*iter_cur);

                xVector3 N_reaction = xVector3::Normalize(iter_cur->NW_fix);
                xFLOAT W_cos = xVector3::DotProduct(N_reaction, xVector3::Normalize(NW_velocity_new));
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

                xPlane  PN_hit; PN_hit.init(xVector3::Normalize(NW_velocity), P_mean_support);
                xPoint3 P_cast    = PN_hit.castPoint( P_center_Trfm );
                xFLOAT  S_arm     = (P_mean_support - P_cast).length();
                xFLOAT  W_cos_arm = (S_arm < S_radius) ? S_arm / S_radius : 1.f;
                NW_velocity_new += NW_velocity * W_cos_arm;
                xVector3 N_arm = (P_mean_support-P_center_Trfm).normalize();
                xFLOAT  V_speed = NW_velocity.length();
                xQuaternion QT_now = xQuaternion::AngularVelocity( // a * x * (1 - |cosA|)
                    xVector3::CrossProduct(N_arm, NW_velocity).normalize() * (V_speed * (1-W_cos_arm)));
                QT_velocity_new = xQuaternion::Product(QT_velocity_new, QT_now);
                //ApplyAcceleration(NW_velocity_new * W_sum, 1.f, CollisionPoint(NULL, P_mean_support));
            }
        }
        else
            NW_velocity_new += NW_velocity;

        NW_velocity = NW_velocity_new;
        if (!NW_velocity.isZero())
        {
            if (T_time != 0)
                MX_LocalToWorld_Set().postTranslateT(NW_velocity * T_time);
            NW_velocity_new.zero();
        }
    }

    if  (IsModified()) P_center_Trfm = MX_LocalToWorld_Get().preTransformP(P_center);

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
                    QT_velocity_new = xQuaternion::Product(
                                            QT_velocity.interpolateFull(1.f - W_air_drag / V_omega),
                                            QT_velocity_new);
            }
            else
                QT_velocity_new = xQuaternion::Product(QT_velocity, QT_velocity_new);
        }
        QT_velocity = QT_velocity_new;
        if (QT_velocity.w != 1.f)
        {
            if (T_time != 0)
                MX_LocalToWorld_Set() *= xMatrixFromQuaternion(xQuaternion::InterpolateFull(QT_velocity, T_time))
                    .preTranslateT(-P_center_Trfm).postTranslateT(P_center_Trfm);
            QT_velocity_new.zeroQ();
        }
    }

    if (IsModified()) BVHierarchy.invalidateTransformation();
}
