#ifndef __incl_Physics_VerletBody_h
#define __incl_Physics_VerletBody_h

#include "../World/SkeletizedObj.h"

class VerletBody
{
public:
    static void  CalculateCollisions(SkeletizedObj *model, float deltaTime);
    static void  CalculateMovement(SkeletizedObj *model, float deltaTime);

    static xVector3 CalcCollisionSpeed( const xVector3 triangle[3], const xVector3 &collisionP,
                             const xElement &elem, const xFace &face,
                             const xVerletSystem &system );
};


struct TriangleWeights
{
    xFLOAT3 weight;
    xFLOAT  Pmax;
};
TriangleWeights CalcPenetrationDepth(const xVector3 triangle[3], const xVector3 &planeP, const xVector3 &planeN);

#endif
