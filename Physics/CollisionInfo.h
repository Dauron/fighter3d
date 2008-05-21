#ifndef __incl_Physics_CollisionInfo_h
#define __incl_Physics_CollisionInfo_h

#include "../Models/lib3dx/xUtils.h"
#include <vector>

class ModelObj;

struct Collisions
{
    xElement *elem1;
    xElement *elem2;
    xFace    *face1;
    xFace    *face2;

    xVector3  face1v[3];
    xVector3  face2v[3];
    xVector3  colPoint;
};

struct CollisionWithModel
{
    ModelObj               *model2;
    std::vector<Collisions> collisions;
};

#endif
