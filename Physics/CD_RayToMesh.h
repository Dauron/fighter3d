#ifndef __incl_Physics_CDRayToMesh_h
#define __incl_Physics_CDRayToMesh_h

#include "../World/RigidObj.h"
#include "../Math/Figures/xBoxA.h"

using namespace Math::Figures;

class CD_RayToMesh
{
#define determinant(vA, vB, vC)                     (vA->x-vC->x)*(vB->y-vC->y)-(vA->y-vC->y)*(vB->x-vC->x)
    static const float Epsilon;

    xVector3 rayB, rayE, rayV;

    xVector3 collisionPoint;
    float    collisionDist;
    bool     notCollided;

    void RayCage(xBoxA &refBox, xBoxA &rayBox);
    bool CollideBox(xBoxA &box);

    bool IntersectTriangles(xVector4 *a1, xVector4 *a2, xVector4 *a3, xVector3 *crossing);
    bool CheckOctreeLevel(xCollisionHierarchyBoundsRoot *ci,
                          xCollisionData *pcd,
                          xCollisionHierarchyBounds *chb,
                          xElement *elem);
    bool CollideElements(xCollisionHierarchyBoundsRoot *&ci, xElement *elem);

public:

    bool Collide(RigidObj *model,
                 xVector3 &rayB, xVector3 &rayE,
                 xVector3 &colPoint, float &colDist);
};

#endif
