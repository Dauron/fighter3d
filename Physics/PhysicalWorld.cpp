#include "PhysicalWorld.h"

using namespace Physics;

void PhysicalWorld :: Interact(xFLOAT T_time, ObjectVector &L_objects)
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
                        Collider.Collide(&pBody1, &pBody2,
                                         pBody1.BVHierarchy, pBody2.BVHierarchy,
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

                            xFLOAT W_V1_1 = (M_mass_1 - M_mass_2) * W_M1M2_Inv * W_hits_Inv * pBody1.W_restitution_self;
                            xFLOAT W_V1_2 = 2.f * M_mass_1 * W_M1M2_Inv * W_hits_Inv * pBody1.W_restitution;
                            xFLOAT W_V2_1 = 2.f * M_mass_2 * W_M1M2_Inv * W_hits_Inv * pBody2.W_restitution;
                            xFLOAT W_V2_2 = (M_mass_2 - M_mass_1) * W_M1M2_Inv * W_hits_Inv * pBody2.W_restitution_self;

                            xFLOAT W_fix_1 = M_mass_1 * W_M1M2_Inv; //* W_hits_Inv;
                            xFLOAT W_fix_2 = M_mass_2 * W_M1M2_Inv; //* W_hits_Inv;

                            //pBody1.MX_LocalToWorld_Set().postTranslateT(NW_best_fix_1 * W_fix_1);
                            //pBody2.MX_LocalToWorld_Set().postTranslateT(NW_best_fix_1 * -W_fix_2);

                            for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                            {
                                CollisionPoint &cp1 = collision->CPoint1_Get();
                                CollisionPoint &cp2 = collision->CPoint2_Get();
                                cp1.V_action = pBody1.GetVelocity(cp1);
                                cp2.V_action = pBody2.GetVelocity(cp2);
                                cp1.V_reaction = cp1.V_action * W_V1_1 + cp2.V_action * W_V2_1;
                                cp2.V_reaction = cp1.V_action * W_V1_2 + cp2.V_action * W_V2_2;
                                cp1.W_fix      = W_fix_1;
                                cp2.W_fix      = W_fix_2;

                                pBody1.ApplyAcceleration(cp1.V_reaction, 1.f, cp1);
                                pBody1.Collisions.push_back(cp1);

                                pBody2.ApplyAcceleration(cp2.V_reaction, 1.f, cp2);
                                pBody2.Collisions.push_back(cp2);
                            }
                        }
                        else
                        if (pBody2.FL_stationary)
                        {
                            xFLOAT W_V1_1 = -1.f * W_hits_Inv * pBody1.W_restitution_self;

                            //pBody1.MX_LocalToWorld_Set().postTranslateT(NW_best_fix_1);

                            for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                            {
                                CollisionPoint &cp1 = collision->CPoint1_Get();
                                cp1.V_action   = pBody1.GetVelocity(cp1);
                                cp1.V_reaction = cp1.V_action * W_V1_1;
                                cp1.W_fix      = 1.f;

                                pBody1.ApplyAcceleration(cp1.V_reaction, 1.f, cp1);
                                pBody1.Collisions.push_back(cp1);
                            }
                        }
                        else
                        if (pBody1.FL_stationary)
                        {
                            xFLOAT W_V2_2 = -1.f * W_hits_Inv * pBody2.W_restitution_self;

                            //pBody2.MX_LocalToWorld_Set().postTranslateT(-NW_best_fix_1);

                            for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                            {
                                CollisionPoint &cp2 = collision->CPoint2_Get();
                                cp2.V_action   = pBody2.GetVelocity(cp2);
                                cp2.V_reaction = cp2.V_action * W_V2_2;
                                cp2.W_fix      = 1.f;

                                pBody2.ApplyAcceleration(cp2.V_reaction, 1.f, cp2);
                                pBody2.Collisions.push_back(cp2);
                            }
                        }

                        cset.Clear();
                    }
                }

    for (Object_1 = L_objects.begin(); Object_1 != Object_2_Last; ++Object_1)
    {
        IPhysicalBody &pBody1 = **Object_1;
        if (pBody1.FL_physical && !pBody1.FL_stationary)
            pBody1.ApplyAcceleration(Gravity_Get(pBody1), T_time);
        pBody1.FrameUpdate(T_time);
    }
}
