#ifndef __incl_Physics_RigidBody_h
#define __incl_Physics_RigidBody_h

#include "../World/RigidObj.h"

class RigidBody
{
public:
    struct Contribution
    {
        xFLOAT contrib[4];
    };

    static Contribution GetParticleContribution(const xVector3 &N_plane_2,
                                                const xVector3 *P_current,
                                                const xVector3 &P_collision,
                                                const xVector3 &P_collision_Local);
    static Contribution GetParticleContribution(const xVector3 &P_collision_Local);
    static void GetParticleSpeeds(xVector3 *V_speed, const xVector3 *P_current, const xVector3 *P_previous,
                             xFLOAT T_step_SqrInv)
    {
        if (T_step_SqrInv > 0.f)
        {
            V_speed[0] = (P_current[0] - P_previous[0]) * T_step_SqrInv;
            V_speed[1] = (P_current[1] - P_previous[1]) * T_step_SqrInv;
            V_speed[2] = (P_current[2] - P_previous[2]) * T_step_SqrInv;
            V_speed[3] = (P_current[3] - P_previous[3]) * T_step_SqrInv;
        }
        else
            memset (V_speed, 0, sizeof(xVector3)*4);
    }

    static void  CalculateCollisions(RigidObj *model, float T_delta);
    static void  CalculateMovement(RigidObj *model, float T_delta);
};

#endif
