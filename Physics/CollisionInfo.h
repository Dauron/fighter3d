#ifndef __incl_Physics_CollisionInfo_h_old
#define __incl_Physics_CollisionInfo_h_old

#include "../Models/lib3dx/xUtils.h"
#include <vector>

class RigidObj;

struct Collisions
{
    xElement *elem1;
    xElement *elem2;
    xFace    *ID_face_1;
    xFace    *ID_face_2;

    xVector3  face1v[3];
    xVector3  face2v[3];
    xVector3  colPoint;
};

struct CollisionWithModel
{
    RigidObj               *model2;
    std::vector<Collisions> collisions;
};

#endif
