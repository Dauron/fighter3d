#ifndef __incl_Physics_VerletBody_h
#define __incl_Physics_VerletBody_h

#include "../World/SkeletizedObj.h"

class VerletBody
{
public:
    static void  CalculateCollisions(SkeletizedObj *model);
    static void  CalculateMovement(SkeletizedObj *model, float deltaTime);
};

#endif
