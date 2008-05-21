#ifndef __incl_Physics_RigidBody_h
#define __incl_Physics_RigidBody_h

#include "../World/ModelObj.h"

class RigidBody
{
public:
    static float CalcPenetrationDepth(ModelObj *model, xVector3 &planeP, xVector3 &planeN);
    static void  CalculateCollisions(ModelObj *model);
    static void  CalculateMovement(ModelObj *model, float deltaTime);

    static const float GRAVITY;
    static const float FRICTION_AIR;
};

#endif
