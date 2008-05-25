#ifndef __incl_Physics_VerletBody_h
#define __incl_Physics_VerletBody_h

#include "../World/SkeletizedObj.h"

class VerletBody
{
public:
    static void  CalculateCollisions(SkeletizedObj *model, float T_delta);
    static void  CalculateMovement(SkeletizedObj *model, float T_delta);

    static xVector3 GetCollisionSpeed( const xVector3 P_face[3], const xVector3 &P_collision,
                             const xElement &elem, const xFace &face,
                             const xVerletSystem &system );
};


struct FaceWeights
{
    xFLOAT3  S_vert;
    xFLOAT   S_max;
    xVector3 P_max;
};
FaceWeights CalcPenetrationDepth(const xVector3 P_face[3], const xVector3 &P_plane, const xVector3 &N_plane);

#endif
