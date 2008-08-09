#include "CD_RayToMesh.h"

const float CD_RayToMesh::Epsilon = 0.01f;

void CD_RayToMesh::RayCage(xBoxA &refBox, xBoxA &rayBox)
{
    float x = fabs(rayV.x), y = fabs(rayV.y), z = fabs(rayV.z);
    if (x >= y && x >= z)
    {
        rayBox.P_max.x = refBox.P_max.x-rayB.x;
        if (rayBox.P_max.x*rayV.x < 0) rayBox.P_max.x = 0;
        float scale = rayBox.P_max.x / rayV.x;
        rayBox.P_max.x = rayB.x + rayV.x * scale;
        rayBox.P_max.y = rayB.y + rayV.y * scale;
        rayBox.P_max.z = rayB.z + rayV.z * scale;

        rayBox.P_min.x = refBox.P_min.x-rayB.x;
        if (rayBox.P_min.x*rayV.x < 0) rayBox.P_min.x = 0;
        scale = rayBox.P_min.x / rayV.x;
        rayBox.P_min.x = rayB.x + rayV.x * scale;
        rayBox.P_min.y = rayB.y + rayV.y * scale;
        rayBox.P_min.z = rayB.z + rayV.z * scale;
    }
    else if (y >= z)
    {
        rayBox.P_max.y = refBox.P_max.y-rayB.y;
        if (rayBox.P_max.y*rayV.y < 0) rayBox.P_max.y = 0;
        float scale = rayBox.P_max.y / rayV.y;
        rayBox.P_max.x = rayB.x + rayV.x * scale;
        rayBox.P_max.y = rayB.y + rayV.y * scale;
        rayBox.P_max.z = rayB.z + rayV.z * scale;

        rayBox.P_min.y = refBox.P_min.y-rayB.y;
        if (rayBox.P_min.y*rayV.y < 0) rayBox.P_min.y = 0;
        scale = rayBox.P_min.y / rayV.y;
        rayBox.P_min.x = rayB.x + rayV.x * scale;
        rayBox.P_min.y = rayB.y + rayV.y * scale;
        rayBox.P_min.z = rayB.z + rayV.z * scale;
    }
    else
    {
        rayBox.P_max.z = refBox.P_max.z-rayB.z;
        if (rayBox.P_max.z*rayV.z < 0) rayBox.P_max.z = 0;
        float scale = rayBox.P_max.z / rayV.z;
        rayBox.P_max.x = rayB.x + rayV.x * scale;
        rayBox.P_max.y = rayB.y + rayV.y * scale;
        rayBox.P_max.z = rayB.z + rayV.z * scale;

        rayBox.P_min.z = refBox.P_min.z-rayB.z;
        if (rayBox.P_min.z*rayV.z < 0) rayBox.P_min.z = 0;
        scale = rayBox.P_min.z / rayV.z;
        rayBox.P_min.x = rayB.x + rayV.x * scale;
        rayBox.P_min.y = rayB.y + rayV.y * scale;
        rayBox.P_min.z = rayB.z + rayV.z * scale;
    }

    xFLOAT swp;
    if (rayBox.P_max.x < rayBox.P_min.x) { swp = rayBox.P_max.x; rayBox.P_max.x = rayBox.P_min.x; rayBox.P_min.x = swp; }
    if (rayBox.P_max.y < rayBox.P_min.y) { swp = rayBox.P_max.y; rayBox.P_max.y = rayBox.P_min.y; rayBox.P_min.y = swp; }
    if (rayBox.P_max.z < rayBox.P_min.z) { swp = rayBox.P_max.z; rayBox.P_max.z = rayBox.P_min.z; rayBox.P_min.z = swp; }
}

bool CD_RayToMesh::CollideBox(xBoxA &box)
{
    if((rayB.x < box.P_min.x && rayE.x < box.P_min.x) ||
       (rayB.x > box.P_max.x && rayE.x > box.P_max.x) ||
       (rayB.y < box.P_min.y && rayE.y < box.P_min.y) ||
       (rayB.y > box.P_max.y && rayE.y > box.P_max.y) ||
       (rayB.z < box.P_min.z && rayE.z < box.P_min.z) ||
       (rayB.z > box.P_max.z && rayE.z > box.P_max.z))
       return false; // no possible crossing

    xBoxA rayCage;
    RayCage(box, rayCage);
    if ((box.P_min.x - Epsilon > rayCage.P_max.x) ||
        (box.P_max.x + Epsilon < rayCage.P_min.x) ||
        (box.P_min.y - Epsilon > rayCage.P_max.y) ||
        (box.P_max.y + Epsilon < rayCage.P_min.y) ||
        (box.P_min.z - Epsilon > rayCage.P_max.z) ||
        (box.P_max.z + Epsilon < rayCage.P_min.z))
        return false;

    return true;
}

bool CD_RayToMesh::IntersectTriangles(xVector4 *a1, xVector4 *a2, xVector4 *a3, xVector3 *crossing)
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

    xVector4 *swp;
    if (determinant(a1,a2,a3) < 0.f) { swp = a2; a2 = a3; a3 = swp; }

    P1 = determinant(crossing, a1, a2);
    P2 = determinant(crossing, a2, a3);
    float P3 = determinant(crossing, a3, a1);

    if (P1 >= -Epsilon && P2 >= -Epsilon && P3 >= -Epsilon) // inside, on the vertex, on the edge or "empty triangle"
        return true;

    return false;
}

bool CD_RayToMesh::CheckOctreeLevel(xCollisionHierarchyBoundsRoot *ci,
                                    xCollisionData                *pcd,
                                    xCollisionHierarchyBounds     *chb,
                                    xElement                      *elem)
{
    bool res = false;
    xVector3 colPoint;
    xCollisionHierarchy *ch = pcd->kidsP;

    for (int h1 = pcd->kidsC; h1; --h1, ++chb, ++ch)
    {
        if (CollideBox(chb->bounding))
        {
            if (ch->kidsP)
            {
                res |= CheckOctreeLevel(ci, ch, chb->kids, elem);
                continue;
            }

            for (int i1 = ch->facesC; i1; --i1)
            {
                xFace **face1 = ch->facesP + ch->facesC - i1;
                xVector4 *a1 = ci->verticesP + (**face1)[0];
                xVector4 *a2 = ci->verticesP + (**face1)[1];
                xVector4 *a3 = ci->verticesP + (**face1)[2];
                // Exclude elements that can't collide
                xBoxA faceB;
                faceB.P_min.x = min(a1->x, min(a2->x, a3->x));
                faceB.P_min.y = min(a1->y, min(a2->y, a3->y));
                faceB.P_min.z = min(a1->z, min(a2->z, a3->z));
                faceB.P_max.x = max(a1->x, max(a2->x, a3->x));
                faceB.P_max.y = max(a1->y, max(a2->y, a3->y));
                faceB.P_max.z = max(a1->z, max(a2->z, a3->z));
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



bool CD_RayToMesh::CollideElements(xCollisionHierarchyBoundsRoot *&ci, xElement *elem)
{
    bool res = false;

    if (elem->verticesC && ci->kids && CollideBox(ci->bounding))
        res |= CheckOctreeLevel(ci, &elem->collisionData, ci->kids, elem);

    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        res |= CollideElements(++ci, celem);

    return res;
}

bool CD_RayToMesh::Collide(RigidObj *model,
                           xVector3 &rayB, xVector3 &rayE,
                           xVector3 &colPoint, float &colDist)
{
    xCollisionHierarchyBoundsRoot *ci = model->GetCollisionInfo()-1;

    this->rayB = rayB;
    this->rayE = rayE;
    rayV = rayE - rayB;

    notCollided = true;

    bool res = false;
    for (xElement *elem = model->GetModelPh()->kidsP; elem; elem = elem->nextP)
        res |= CollideElements(++ci, elem);

    colPoint = collisionPoint;
    colDist  = collisionDist;

    return res;
}
