#include "RayTrCollisionDetector.h"

const float RayTrCollisionDetector::Epsilon = 0.01f;

void RayTrCollisionDetector::RayCage(xBox &refBox, xBox &rayBox)
{
    float x = fabs(rayV.x), y = fabs(rayV.y), z = fabs(rayV.z);
    if (x >= y && x >= z)
    {
        rayBox.max.x = refBox.max.x-rayB.x;
        if (rayBox.max.x*rayV.x < 0) rayBox.max.x = 0;
        float scale = rayBox.max.x / rayV.x;
        rayBox.max.x = rayB.x + rayV.x * scale;
        rayBox.max.y = rayB.y + rayV.y * scale;
        rayBox.max.z = rayB.z + rayV.z * scale;

        rayBox.min.x = refBox.min.x-rayB.x;
        if (rayBox.min.x*rayV.x < 0) rayBox.min.x = 0;
        scale = rayBox.min.x / rayV.x;
        rayBox.min.x = rayB.x + rayV.x * scale;
        rayBox.min.y = rayB.y + rayV.y * scale;
        rayBox.min.z = rayB.z + rayV.z * scale;
    }
    else if (y >= z)
    {
        rayBox.max.y = refBox.max.y-rayB.y;
        if (rayBox.max.y*rayV.y < 0) rayBox.max.y = 0;
        float scale = rayBox.max.y / rayV.y;
        rayBox.max.x = rayB.x + rayV.x * scale;
        rayBox.max.y = rayB.y + rayV.y * scale;
        rayBox.max.z = rayB.z + rayV.z * scale;

        rayBox.min.y = refBox.min.y-rayB.y;
        if (rayBox.min.y*rayV.y < 0) rayBox.min.y = 0;
        scale = rayBox.min.y / rayV.y;
        rayBox.min.x = rayB.x + rayV.x * scale;
        rayBox.min.y = rayB.y + rayV.y * scale;
        rayBox.min.z = rayB.z + rayV.z * scale;
    }
    else
    {
        rayBox.max.z = refBox.max.z-rayB.z;
        if (rayBox.max.z*rayV.z < 0) rayBox.max.z = 0;
        float scale = rayBox.max.z / rayV.z;
        rayBox.max.x = rayB.x + rayV.x * scale;
        rayBox.max.y = rayB.y + rayV.y * scale;
        rayBox.max.z = rayB.z + rayV.z * scale;

        rayBox.min.z = refBox.min.z-rayB.z;
        if (rayBox.min.z*rayV.z < 0) rayBox.min.z = 0;
        scale = rayBox.min.z / rayV.z;
        rayBox.min.x = rayB.x + rayV.x * scale;
        rayBox.min.y = rayB.y + rayV.y * scale;
        rayBox.min.z = rayB.z + rayV.z * scale;
    }

    xFLOAT swp;
    if (rayBox.max.x < rayBox.min.x) { swp = rayBox.max.x; rayBox.max.x = rayBox.min.x; rayBox.min.x = swp; }
    if (rayBox.max.y < rayBox.min.y) { swp = rayBox.max.y; rayBox.max.y = rayBox.min.y; rayBox.min.y = swp; }
    if (rayBox.max.z < rayBox.min.z) { swp = rayBox.max.z; rayBox.max.z = rayBox.min.z; rayBox.min.z = swp; }
}

bool RayTrCollisionDetector::CollideBox(xBox &box)
{
    if((rayB.x < box.min.x && rayE.x < box.min.x) ||
       (rayB.x > box.max.x && rayE.x > box.max.x) ||
       (rayB.y < box.min.y && rayE.y < box.min.y) ||
       (rayB.y > box.max.y && rayE.y > box.max.y) ||
       (rayB.z < box.min.z && rayE.z < box.min.z) ||
       (rayB.z > box.max.z && rayE.z > box.max.z))
       return false; // no possible crossing

    xBox rayCage;
    RayCage(box, rayCage);
    if ((box.min.x - Epsilon > rayCage.max.x) ||
        (box.max.x + Epsilon < rayCage.min.x) ||
        (box.min.y - Epsilon > rayCage.max.y) ||
        (box.max.y + Epsilon < rayCage.min.y) ||
        (box.min.z - Epsilon > rayCage.max.z) ||
        (box.max.z + Epsilon < rayCage.min.z))
        return false;

    return true;
}

bool RayTrCollisionDetector::IntersectTriangles(xVector3 *a1, xVector3 *a2, xVector3 *a3, xVector3 *crossing)
{
    // Calculate plane (with CrossProduct)
    float p1x = a2->x - a1->x, p1y = a2->y - a1->y, p1z = a2->z - a1->z;
    float p2x = a3->x - a1->x, p2y = a3->y - a1->y, p2z = a3->z - a1->z;
    float X = p1y*p2z - p1z*p2y, Y = p1z*p2x - p1x*p2z, Z = p1x*p2y - p1y*p2x;
    float scale = sqrt(X*X+Y*Y+Z*Z);
    X /= scale; Y /= scale; Z /= scale;
    float W = -(a1->x*X + a1->y*Y + a1->z*Z);
    // Check positions (with DotProducts+W)
    float P1 = rayB.x * X + rayB.y * Y + rayB.z * Z + W;
    float P2 = rayE.x * X + rayE.y * Y + rayE.z * Z + W;

    if ((P1 > 0.f && P2 > 0.f) || (P1 < 0.f && P2 < 0.f)) // no intersection
        return false;
    else
    if (P1 == 0.f && P2 == 0.f)                           // coplanar
        return false; // we don't treat 2d collisions as collisions, they just touch, but do not collide

    // Cast ray points to the plane
    p1x = rayB.x - X * P1; p1y = rayB.y - Y * P1; p1z = rayB.z - Z * P1;
    p2x = rayE.x - X * P2; p2y = rayE.y - Y * P2; p2z = rayE.z - Z * P2;
    // Calculate new ray direction
    p2x -= p1x; p2y -= p1y; p2z -= p1z;
    // Calculate crossing point
    float ipol = fabs(P1)/(fabs(P1)+fabs(P2));
    crossing->x = p1x + ipol*p2x;
    crossing->y = p1y + ipol*p2y;
    crossing->z = p1z + ipol*p2z;

    xVector3 *swp;
    if (determinant(a1,a2,a3) < 0.f) { swp = a2; a2 = a3; a3 = swp; }

    P1 = determinant(crossing, a1, a2);
    P2 = determinant(crossing, a2, a3);
    float P3 = determinant(crossing, a3, a1);

    if (P1 >= -Epsilon && P2 >= -Epsilon && P3 >= -Epsilon) // inside, on the vertex, on the edge or "empty triangle"
        return true;

    return false;
}

bool RayTrCollisionDetector::CheckOctreeLevel(CollisionInfo             *ci,
                                              xCollisionHierarchy       *ch,
                                              xCollisionHierarchyBounds *chb,
                                              xWORD                      cnt,
                                              xElement                  *elem)
{
    bool res = false;
    xVector3 colPoint;

    for (int h1 = cnt; h1; --h1, ++chb, ++ch)
    {
        if (CollideBox(chb->bounding))
        {
            if (ch->kidsP)
            {
                res |= CheckOctreeLevel(ci, ch->kidsP, chb->kids, ch->kidsC, elem);
                continue;
            }

            for (int i1 = ch->facesC; i1; --i1)
            {
                xWORD3 **face1 = ch->facesP + ch->facesC - i1;
                xVector3 *a1 = &(ci->verticesP + (**face1)[0])->vector3;
                xVector3 *a2 = &(ci->verticesP + (**face1)[1])->vector3;
                xVector3 *a3 = &(ci->verticesP + (**face1)[2])->vector3;
                // Exclude elements that can't collide
                xBox faceB;
                faceB.min.x = min(a1->x, min(a2->x, a3->x));
                faceB.min.y = min(a1->y, min(a2->y, a3->y));
                faceB.min.z = min(a1->z, min(a2->z, a3->z));
                faceB.max.x = max(a1->x, max(a2->x, a3->x));
                faceB.max.y = max(a1->y, max(a2->y, a3->y));
                faceB.max.z = max(a1->z, max(a2->z, a3->z));
                if (!CollideBox(faceB))
                    continue;

                if (IntersectTriangles(a1,a2,a3,&colPoint))
                {
                    float cDist = (colPoint - rayB).length();
                    if (notCollided || cDist < collisionDist) {
                        collisionPoint = colPoint;
                        collisionDist = (colPoint - rayB).length();
                    }
                    notCollided = false;
                    res = true;
                }
            }
        }
    }
    return res;
}



bool RayTrCollisionDetector::CollideElements(CollisionInfo *&ci, xElement *elem)
{
    bool res = false;

    if (elem->verticesC && ci->collisionHP && CollideBox(ci->bounding))
        res |= CheckOctreeLevel(ci, elem->collisionData.hierarchyP, ci->collisionHP,
            elem->collisionData.hierarchyC, elem);

    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        res |= CollideElements(++ci, celem);

    return res;
}

bool RayTrCollisionDetector::Collide(ModelObj *model,
                                     xVector3 &rayB, xVector3 &rayE,
                                     xVector3 &colPoint, float &colDist)
{
    CollisionInfo *ci = model->GetCollisionInfo()-1;
    xRender *r = model->GetRenderer();

    this->rayB = rayB;
    this->rayE = rayE;
    rayV = rayE - rayB;

    notCollided = true;

    bool res = false;
    for (xElement *elem = r->xModel->firstP; elem; elem = elem->nextP)
        res |= CollideElements(++ci, elem);

    colPoint = collisionPoint;
    colDist  = collisionDist;

    return res;
}
