#ifndef __incl_Physics_RayTrCollisionDetector_h
#define __incl_Physics_RayTrCollisionDetector_h

#include "../World/ModelObj.h"

class RayTrCollisionDetector
{
#define determinant(vA, vB, vC)                     (vA->x-vC->x)*(vB->y-vC->y)-(vA->y-vC->y)*(vB->x-vC->x)
    static const float Epsilon;

    xVector3 rayB, rayE, rayV;

    xVector3 collisionPoint;
    float    collisionDist;
    bool     notCollided;

    void RayCage(xBox &refBox, xBox &rayBox);
    bool CollideBox(xBox &box);

    bool IntersectTriangles(xVector3 *a1, xVector3 *a2, xVector3 *a3, xVector3 *crossing);
    bool CheckOctreeLevel(CollisionInfo *ci,
                          xCollisionHierarchy *ch,
                          xCollisionHierarchyBounds *chb,
                          xWORD cnt,
                          xElement *elem);
    bool CollideElements(CollisionInfo *&ci, xElement *elem);

public:

    bool Collide(ModelObj *model,
                 xVector3 &rayB, xVector3 &rayE,
                 xVector3 &colPoint, float &colDist);
};

#endif
