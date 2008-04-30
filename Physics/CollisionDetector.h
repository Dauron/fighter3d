#ifndef __incl_Physics_CollisionDetector_h
#define __incl_Physics_CollisionDetector_h

#include "../World/ModelObj.h"

class CollisionDetector
{
#define det3(x1, y1, z1, x2, y2, z2, x3, y3, z3)    (x1)*((y2)*(z3)-(z2)*(y3))+(y1)*((z2)*(x3)-(x2)*(z3))+(z1)*((x2)*(y3)-(y2)*(x3));
#define determinant(vA, vB, vC)                     (vA->x-vC->x)*(vB->y-vC->y)-(vA->y-vC->y)*(vB->x-vC->x)
#define sgn(a)                                      ((a) > 0 ? 1 : ( (a) < 0 ? -1 : 0 ));

    ModelObj *model1;
    ModelObj *model2;

    CollisionWithModel *collidedModel1;
    CollisionWithModel *collidedModel2;

    int  CheckPlanes        (xVector3 *a1, xVector3 *a2, xVector3 *a3,
                             xVector3 *b1, xVector3 *b2, xVector3 *b3);
    bool IntersectTriangles (xVector3 *a1, xVector3 *a2, xVector3 *a3,
                             xVector3 *b1, xVector3 *b2, xVector3 *b3, xVector3 *crossing);

    bool CheckOctreeLevel(CollisionInfo *ci1,              CollisionInfo *ci2,
                          xCollisionHierarchy *ch1,        xCollisionHierarchy *ch2,
                          xCollisionHierarchyBounds *chb1, xCollisionHierarchyBounds *chb2,
                          xWORD cnt1,                      xWORD cnt2,
                          xElement *elem1,                 xElement *elem2);
    // Scan all elements in model2
    bool Collide2(CollisionInfo *ci1, CollisionInfo *&ci2, xElement *elem1, xElement *elem2);
    // Scan all elements in model1
    bool Collide1(CollisionInfo *&ci1, CollisionInfo *&ci2, xElement *elem1, xElement *elem2);

public:

    bool Collide(ModelObj *model1, ModelObj *model2)
    {
        CollisionInfo *ci1 = model1->GetCollisionInfo()-1;
        CollisionInfo *ci2 = model2->GetCollisionInfo()-1;

        float delta = GetTick();

        xRender *r1 = model1->GetRenderer();
        xRender *r2 = model2->GetRenderer();

        this->model1 = model1;
        this->model2 = model2;

        collidedModel1 = collidedModel2 = NULL;

        bool res = false;
        for (xElement *elem1 = r1->xModel->firstP; elem1; elem1 = elem1->nextP)
            res |= Collide1(++ci1, ci2, elem1, r2->xModel->firstP);

        time2 += GetTick() - delta;
        return res;
    }
};

extern int testsLevel0;
extern int testsLevel1;
extern int testsLevel2;
extern int testsLevel3;

#endif
