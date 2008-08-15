#ifndef __incl_Physics_PhysicalWorld_h
#define __incl_Physics_PhysicalWorld_h

#include "IPhysicalBody.h"
#include "Colliders/BVHCollider.h"

namespace Physics {
    using namespace Physics::Colliders;

    class PhysicalWorld
    {
    public:
        typedef std::vector<IPhysicalBody*> ObjectVector;

        virtual const xVector3 &Gravity_Get(const IPhysicalBody &object) const
        {
            static xVector3 NW_gravity = xVector3::Create(0.f,0.f, -10.f);
            return NW_gravity;
        }

        virtual const xVector3 &Gravity_GetNormal(const IPhysicalBody &object) const
        {
            static xVector3 NW_gravity = xVector3::Create(0.f,0.f, -1.f);
            return NW_gravity;
        }

        void Interact(xFLOAT T_time, ObjectVector &L_objects)
        {
            if (!L_objects.size()) return;

            ObjectVector::iterator Object_1,
                                   Object_2,
                                   Object_2_Last = L_objects.end(),
                                   Object_1_Last = Object_2_Last - 1;

            BVHCollider  Collider;
            CollisionSet cset;

            for (Object_1 = L_objects.begin(); Object_1 != Object_2_Last; ++Object_1)
                (**Object_1).Collisions.clear();

            for (Object_1 = L_objects.begin(); Object_1 != Object_1_Last; ++Object_1)
                if (!(**Object_1).FL_phantom)
                    for (Object_2 = Object_1 + 1; Object_2 != Object_2_Last; ++Object_2)
                        if (!(**Object_2).FL_phantom)
                        {
                            IPhysicalBody &pBody1 = **Object_1,
                                          &pBody2 = **Object_2;

                            if (! (pBody1.FL_stationary && pBody2.FL_stationary ) &&
                                Collider.Collide(pBody1.BVHierarchy, pBody2.BVHierarchy,
                                                 pBody1.MX_LocalToWorld_Get(),
                                                 pBody2.MX_LocalToWorld_Get(),
                                                 cset) )
                            {
                                CollisionSet::CollisionVec::iterator collision, ci_end = cset.collisions.end();

                                xPoint3  P_mean_support_1;
                                xPoint3  P_mean_support_2;
                                xVector3 NW_best_fix_1;
                                cset.MergeCollisions(P_mean_support_1, P_mean_support_2, NW_best_fix_1);

                                xFLOAT W_hits_Inv = 1.f / cset.Count();
                                if (!(pBody1.FL_stationary || pBody2.FL_stationary))
                                {
                                    xFLOAT M_mass_1 = pBody1.GetMass();
                                    xFLOAT M_mass_2 = pBody2.GetMass();
                                    xFLOAT W_M1M2_Inv = 1.f / (M_mass_1 + M_mass_2);

                                    xFLOAT W_V1_1 = (M_mass_1 - M_mass_2) * W_M1M2_Inv * W_hits_Inv;
                                    xFLOAT W_V1_2 = 2.f * M_mass_1 * W_M1M2_Inv * W_hits_Inv;
                                    xFLOAT W_V2_1 = 2.f * M_mass_2 * W_M1M2_Inv * W_hits_Inv;
                                    xFLOAT W_V2_2 = (M_mass_2 - M_mass_1) * W_M1M2_Inv * W_hits_Inv;

                                    xFLOAT W_fix_1 =  M_mass_1 * W_M1M2_Inv; //* W_hits_Inv;
                                    xFLOAT W_fix_2 = -M_mass_2 * W_M1M2_Inv; //* W_hits_Inv;

                                    pBody1.MX_LocalToWorld_Set().postTranslateT(NW_best_fix_1 * W_fix_1);
                                    pBody2.MX_LocalToWorld_Set().postTranslateT(NW_best_fix_1 * W_fix_2);

                                    for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                                    {
                                        xVector3 NW_velocity_1 = pBody1.GetVelocity(collision->P_collision_1);
                                        xVector3 NW_velocity_2 = pBody2.GetVelocity(collision->P_collision_2);
                                        xVector3 NW_velocity_1n = NW_velocity_1 * W_V1_1 + NW_velocity_2 * W_V2_1;
                                        xVector3 NW_velocity_2n = NW_velocity_1 * W_V1_2 + NW_velocity_2 * W_V2_2;

                                        pBody1.ApplyAcceleration(NW_velocity_1n, 1.f, collision->P_collision_1);
                                        pBody2.ApplyAcceleration(NW_velocity_2n, 1.f, collision->P_collision_2);

                                        //pBody1.MX_LocalToWorld_Set().postTranslateT(collision->NW_fix_1 * W_fix_1);
                                        //pBody2.MX_LocalToWorld_Set().postTranslateT(collision->NW_fix_1 * W_fix_2);

                                        pBody1.Collisions.push_back(HitInfo(collision->P_collision_1, NW_velocity_1n));
                                        pBody2.Collisions.push_back(HitInfo(collision->P_collision_2, NW_velocity_2n));
                                    }
                                }
                                else
                                if (pBody2.FL_stationary)
                                {
                                    xFLOAT W_V1_1 = -1.f * W_hits_Inv;

                                    pBody1.MX_LocalToWorld_Set().postTranslateT(NW_best_fix_1);

                                    for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                                    {
                                        xVector3 NW_velocity_1 = pBody1.GetVelocity(collision->P_collision_1);
                                        xVector3 NW_velocity_1n = NW_velocity_1*W_V1_1;

                                        pBody1.ApplyAcceleration(NW_velocity_1n, 1.f, collision->P_collision_1);

                                        //pBody1.MX_LocalToWorld_Set().postTranslateT(collision->NW_fix_1 * W_hits_Inv);

                                        pBody1.Collisions.push_back(HitInfo(collision->P_collision_1, NW_velocity_1n));
                                    }
                                }
                                else
                                if (pBody1.FL_stationary)
                                {
                                    xFLOAT W_V2_2 = -1.f * W_hits_Inv;

                                    pBody2.MX_LocalToWorld_Set().postTranslateT(-NW_best_fix_1);

                                    for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                                    {
                                        xVector3 NW_velocity_2 = pBody2.GetVelocity(collision->P_collision_2);
                                        xVector3 NW_velocity_2n = NW_velocity_2*W_V2_2;

                                        pBody2.ApplyAcceleration(NW_velocity_2n, 1.f, collision->P_collision_2);

                                        //pBody2.MX_LocalToWorld_Set().postTranslateT(collision->NW_fix_1 * (-W_hits_Inv));

                                        pBody2.Collisions.push_back(HitInfo(collision->P_collision_2, NW_velocity_2n));
                                    }
                                }

                                cset.Clear();
                            }
                        }

            std::vector<HitInfo>::iterator iter_cur, iter_end;

            for (Object_1 = L_objects.begin(); Object_1 != Object_2_Last; ++Object_1)
            {
                IPhysicalBody &pBody1 = **Object_1;
                if (pBody1.FL_physical && !pBody1.FL_stationary)
                {
                    pBody1.ApplyAcceleration(Gravity_Get(pBody1), T_time);

                    if (pBody1.Collisions.size())
                    {
                        xPoint3 P_mean_support; P_mean_support.zero();
                        xFLOAT  W_sum = 0.f;

                        iter_cur = pBody1.Collisions.begin();
                        iter_end = pBody1.Collisions.end();
                        for (; iter_cur != iter_end; ++iter_cur)
                        {
                            xFLOAT W_cos = xVector3::DotProduct(iter_cur->NW_collision.normalize(), Gravity_GetNormal(pBody1));
                            if (W_cos < 0.f)
                            {
                                W_sum += W_cos;
                                P_mean_support += iter_cur->P_collision * W_cos;
                            }
                        }
                        if (W_sum < 0.f)
                        {
                            P_mean_support /= W_sum;
                            W_sum /= pBody1.Collisions.size();
                            pBody1.ApplyAcceleration(Gravity_Get(pBody1)* W_sum, T_time, P_mean_support);
                        }
                    }
                }
                pBody1.FrameUpdate(T_time);
            }
        }
    };

} // namespace Physics

#endif
