#ifndef __incl_Physics_PhysicalWorld_h
#define __incl_Physics_PhysicalWorld_h

#include "IPhysicalBody.h"
#include "Colliders/BVHCollider.h"

namespace Physics {
    using namespace Physics::Colliders;

    class PhysicalWorld
    {
    public:
        void Interact(xFLOAT T_time, IPhysicalBody **L_object, xWORD I_object)
        {
            if (!I_object) return;

            IPhysicalBody **Object_1,
                          **Object_2,
                          **Object_2_Last = L_object + I_object,
                          **Object_1_Last = Object_2_Last - 1;

            BVHCollider  Collider;
            CollisionSet cset;

            for (Object_1 = L_object; Object_1 != Object_1_Last; ++Object_1)
                for (Object_2 = Object_1 + 1; Object_2 != Object_2_Last; ++Object_2)
                {
                    IPhysicalBody &pBody1 = **Object_1,
                                  &pBody2 = **Object_2;

                    if (! (pBody1.IsStationary() && pBody2.IsStationary() ) &&
                        Collider.Collide(pBody1.BVHierarchy, pBody2.BVHierarchy,
                                         pBody1.MX_LocalToWorld_Get(),
                                         pBody2.MX_LocalToWorld_Get(),
                                         cset) )
                    {
                        CollisionSet::CollisionVec::iterator collision, ci_end = cset.collisions.end();

                        xFLOAT W_hits_Inv = 1.f / cset.Count();
                        if (!(pBody1.IsStationary() || pBody2.IsStationary()))
                        {
                            xFLOAT M_mass_1 = pBody1.GetMass();
                            xFLOAT M_mass_2 = pBody2.GetMass();
                            xFLOAT W_M1M2_Inv = 1.f / (M_mass_1 + M_mass_2) * W_hits_Inv;

                            xFLOAT W_V1_1 = (M_mass_1 - M_mass_2) * W_M1M2_Inv - W_hits_Inv; // bounce - stop velocity
                            xFLOAT W_V1_2 = 2.f * M_mass_1 * W_M1M2_Inv;
                            xFLOAT W_V2_1 = 2.f * M_mass_2 * W_M1M2_Inv;
                            xFLOAT W_V2_2 = (M_mass_2 - M_mass_1) * W_M1M2_Inv - W_hits_Inv; // bounce - stop velocity

                            xFLOAT W_fix_1 = M_mass_1 * W_M1M2_Inv;
                            xFLOAT W_fix_2 = M_mass_2 * W_M1M2_Inv;

                            for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                            {
                                xVector3 NW_velocity_1 = pBody1.GetVelocity(collision->P_collision_1);
                                xVector3 NW_velocity_2 = pBody2.GetVelocity(collision->P_collision_2);

                                pBody1.ApplyAcceleration(NW_velocity_1 * W_V1_1 + NW_velocity_2 * W_V2_1,
                                    1.f, collision->P_collision_1);
                                pBody1.MX_LocalToWorld_Set().postTranslateT(collision->NW_fix_1 * W_fix_1);

                                pBody2.ApplyAcceleration(NW_velocity_1 * W_V1_2 + NW_velocity_2 * W_V2_2,
                                    1.f, collision->P_collision_2);
                                pBody2.MX_LocalToWorld_Set().postTranslateT(-collision->NW_fix_1 * W_fix_2);
                            }
                        }
                        else
                        if (pBody2.IsStationary())
                        {
                            xFLOAT W_V1_1 = -2.f * W_hits_Inv;
                            for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                            {
                                xVector3 NW_velocity_1 = pBody1.GetVelocity(collision->P_collision_1);
                                pBody1.ApplyAcceleration(NW_velocity_1*W_V1_1, 1.f, collision->P_collision_1);
                                pBody1.MX_LocalToWorld_Set().postTranslateT(collision->NW_fix_1 * W_hits_Inv);
                            }
                        }
                        else
                        if (pBody1.IsStationary())
                        {
                            xFLOAT W_V2_2 = -2.f * W_hits_Inv;
                            for (collision = cset.collisions.begin(); collision != ci_end; ++collision)
                            {
                                xVector3 NW_velocity_2 = pBody2.GetVelocity(collision->P_collision_2);
                                pBody2.ApplyAcceleration(NW_velocity_2*W_V2_2, 1.f, collision->P_collision_2);
                                pBody2.MX_LocalToWorld_Set().postTranslateT(-collision->NW_fix_1 * W_hits_Inv);
                            }
                        }

                        cset.Clear();
                    }
                }

            for (Object_1 = L_object; Object_1 != Object_2_Last; ++Object_1)
                (**Object_1).FrameUpdate(T_time);
        }
    };

} // namespace Physics

#endif
