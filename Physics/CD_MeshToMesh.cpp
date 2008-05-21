#include "CD_MeshToMesh.h"

int  CD_MeshToMesh:: CheckPlanes(xVector4 *a1, xVector4 *a2, xVector4 *a3,
                                 xVector4 *b1, xVector4 *b2, xVector4 *b3)
{
    // Calculate plane (with CrossProduct)
    float p1x = a2->x - a1->x, p1y = a2->y - a1->y, p1z = a2->z - a1->z;
    float p2x = a3->x - a1->x, p2y = a3->y - a1->y, p2z = a3->z - a1->z;
    float X = p1y*p2z - p1z*p2y, Y = p1z*p2x - p1x*p2z, Z = p1x*p2y - p1y*p2x;
    float W = -(a1->x*X + a1->y*Y + a1->z*Z);
    // Check positions (with DotProducts+W)
    float P1 = b1->x * X + b1->y * Y + b1->z * Z + W;
    float P2 = b2->x * X + b2->y * Y + b2->z * Z + W;
    float P3 = b3->x * X + b3->y * Y + b3->z * Z + W;
/*
    xVector4 plane;
    plane.vector3 = xVector3::CrossProduct(*a2-*a1,*a3-*a1);
    plane.w = -xVector3::DotProduct(*a1, plane.vector3);

    float P1 = xVector3::DotProduct(*b1, plane.vector3) + plane.w;
    float P2 = xVector3::DotProduct(*b2, plane.vector3) + plane.w;
    float P3 = xVector3::DotProduct(*b3, plane.vector3) + plane.w;
*/
    if ((P1 > 0.f && P2 > 0.f && P3 > 0.f) || (P1 < 0.f && P2 < 0.f && P3 < 0.f)) // no intersection
        return 0;
    else
    if (P1 == 0.f && P2 == 0.f && P3 == 0.f)                                      // coplanar
        return -1;

    // possible intersection
    if (P2*P3 > 0.f || (P2 == 0.f && P3 == 0.f))
        return 1;
    if (P1*P3 > 0.f || (P1 == 0.f && P3 == 0.f))
        return 2;
    return 3;
}

bool CD_MeshToMesh:: IntersectTriangles(xVector4 *a1, xVector4 *a2, xVector4 *a3,
                                        xVector4 *b1, xVector4 *b2, xVector4 *b3, xVector3 *crossing)
{
    int r = CheckPlanes(a1, a2, a3, b1, b2, b3);
    if (!r) return false;

    xVector4 *swp;
    if (r > 0)
    {
        if (r == 2) { swp = b1; b1 = b2; b2 = b3; b3 = swp; }
        else
        if (r == 3) { swp = b1; b1 = b3; b3 = b2; b2 = swp; }

        r = CheckPlanes(b1, b2, b3, a1, a2, a3);
        if (!r) return false;
    }

    if (r == -1) // coplanar
    {
        return false; // we don't treat 2d collisions as collisions, they just touch, but do not collide

        if (determinant(a1,a2,a3) < 0.f) { swp = a2; a2 = a3; a3 = swp; }
        if (determinant(b1,b2,b3) < 0.f) { swp = b2; b2 = b3; b3 = swp; }

        float P1 = determinant(a1, b1, b2);
        float P2 = determinant(a1, b2, b3);
        float P3 = determinant(a1, b3, b1);

        if (P1 >= 0.f && P2 >= 0.f && P3 >= 0.f) // inside, on the vertex, on the edge or "empty triangle"
            return true;

        while (determinant(a1, b1, b2) < 0 || determinant(a1, b1, b3) < 0)
        { xVector4 *swp = b1; b1 = b2; b2 = b3; b3 = swp; }

        if (determinant(a1, b2, b3) > 0) // Region R1
        {
            if (determinant(b3, b1, a2) >= 0)    // I
            {
                if (determinant(b3, a1, a2) < 0)  // IIa
                    return false;
                else
                {
                    if (determinant(a1, b1, a2) >= 0) // IIIa
                        return true;
                    else
                    {
                        if (determinant(a1, b1, a3) < 0) // IV a
                            return false;
                        else
                        {
                            if (determinant(a2, a3, b1) >= 0) // V
                                return true;
                            else
                                return false;
                        }
                    }
                }
            }
            else
            {
                if (determinant(b3, b1, a3) < 0) // IIb
                    return false;
                else
                {
                    if (determinant(a2, a3, b3) > 0) // IIIb
                        return false;
                    else
                    {
                        if (determinant(a1, b1, a3) > 0) // IVb
                            return false;
                        else
                            return true;
                    }
                }
            }
        }
        else                             // Region R2
        {
            if (determinant(b3, b1, a2) >= 0)        // I
            {
                if (determinant(b2, b3, a2) >= 0)    // IIa
                {
                    if (determinant(a1, b1, a2) >= 0) // IIIa
                    {
                        if (determinant(a1, b2, a2) > 0) // IVa
                            return false;
                        else
                            return true;
                    }
                    else
                    {
                        if (determinant(a1, b1, a3) > 0) // IVb
                            return false;
                        else
                        {
                            if (determinant(b3, b1, a3) > 0) // Va
                                return false;
                            else
                                return true;
                        }
                    }
                }
                else
                {
                    if (determinant(a1, b2, a2) > 0) // IIIb
                        return false;
                    else
                    {
                        if (determinant(b2, b3, a3) > 0) // IVc
                            return false;
                        else
                        {
                            if (determinant(a2, a3, b2) > 0) // Vb
                                return false;
                            else
                                return true;
                        }
                    }
                }
            }
            else
            {
                if (determinant(b3, b1, a3) > 0) // IIb
                    return false;
                else
                {
                    if (determinant(a2, a3, b3) >= 0) // IIIc
                    {
                        if (determinant(a3, a1, b1) > 0) // IVd
                            return false;
                        else
                            return true;
                    }
                    else
                    {
                        if (determinant(a2, a3, b2) > 0) // VIe
                            return false;
                        else
                        {
                            if (determinant(b2, b3, a3) > 0) // Vc
                                return false;
                            else
                                return true;
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (r == 2) { swp = a1; a1 = a2; a2 = a3; a3 = swp; }
        else
        if (r == 3) { swp = a1; a1 = a3; a3 = a2; a2 = swp; }

        // Calculate plane (with CrossProduct)
        float p1x = a2->x - a1->x, p1y = a2->y - a1->y, p1z = a2->z - a1->z;
        float p2x = b1->x - a1->x, p2y = b1->y - a1->y, p2z = b1->z - a1->z;
        float X1 = p1y*p2z - p1z*p2y, Y1 = p1z*p2x - p1x*p2z, Z1 = p1x*p2y - p1y*p2x;
        float W1 = -(a1->x*X1 + a1->y*Y1 + a1->z*Z1);
        // Check positions (with DotProduct+W)
        float P1a = b2->x * X1 + b2->y * Y1 + b2->z * Z1 + W1;
        float P1b = b3->x * X1 + b3->y * Y1 + b3->z * Z1 + W1;
        // Calculate plane (with CrossProduct)
        p1x = a3->x - a1->x; p1y = a3->y - a1->y; p1z = a3->z - a1->z;
        float X2 = p1y*p2z - p1z*p2y, Y2 = p1z*p2x - p1x*p2z, Z2 = p1x*p2y - p1y*p2x;
        float W2 = -(a1->x*X2 + a1->y*Y2 + a1->z*Z2);
        // Check positions (with DotProduct+W)
        float P2a = b3->x * X2 + b3->y * Y2 + b3->z * Z2 + W2;
        float P2b = b2->x * X2 + b2->y * Y2 + b2->z * Z2 + W2;

        int p1a = sgn(P1a); int p1b = sgn(P1b);
        int p2a = sgn(P2a); int p2b = sgn(P2b);
        bool res = p1a*p2a < 0.f || p1b*p2b < 0.f;

        if (res)
        {
            if (!p1a) p1a = -1;
            if (!p2a) p2a = -1;
            if (!p1b) p1b = -1;
            if (!p2b) p2b = -1;
            xVector3 p; p.init(0.f, 0.f, 0.f);

            if (p1a*p2b < 0.f || p2a*p1b < 0.f)
            {
                p2x = a2->x - a1->x, p2y = a2->y - a1->y, p2z = a2->z - a1->z;
                X2 = p1y*p2z - p1z*p2y, Y2 = p1z*p2x - p1x*p2z, Z2 = p1x*p2y - p1y*p2x;
                W2 = -(a1->x*X2 + a1->y*Y2 + a1->z*Z2);
                W2 = b1->x * X2 + b1->y * Y2 + b1->z * Z2 + W2;

                if (p1a*p2b < 0.f)
                {
                    X1 = b2->x - b1->x; Y1 = b2->y - b1->y; Z1 = b2->z - b1->z;
                    W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
                    p.x += b1->x - X1*W1; p.y += b1->y - Y1*W1; p.z += b1->z - Z1*W1;
                }
                if (p2a*p1b < 0.f)
                {
                    X1 = b3->x - b1->x; Y1 = b3->y - b1->y; Z1 = b3->z - b1->z;
                    W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
                    p.x += b1->x - X1*W1; p.y += b1->y - Y1*W1; p.z += b1->z - Z1*W1;
                }
            }
            if (p1a*p1b < 0.f || p2a*p2b < 0.f)
            {
                p1x = b3->x - b1->x; p1y = b3->y - b1->y; p1z = b3->z - b1->z;
                p2x = b2->x - b1->x, p2y = b2->y - b1->y, p2z = b2->z - b1->z;
                X2 = p1y*p2z - p1z*p2y, Y2 = p1z*p2x - p1x*p2z, Z2 = p1x*p2y - p1y*p2x;
                W2 = -(b1->x*X2 + b1->y*Y2 + b1->z*Z2);
                W2 = a1->x * X2 + a1->y * Y2 + a1->z * Z2 + W2;

                if (p1a*p1b < 0.f)
                {
                    X1 = a2->x - a1->x; Y1 = a2->y - a1->y; Z1 = a2->z - a1->z;
                    W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
                    p.x += a1->x - X1*W1; p.y += a1->y - Y1*W1; p.z += a1->z - Z1*W1;
                }
                if (p2a*p2b < 0.f)
                {
                    X1 = a3->x - a1->x; Y1 = a3->y - a1->y; Z1 = a3->z - a1->z;
                    W1 = W2 / (X1 * X2 + Y1 * Y2 + Z1 * Z2);
                    p.x += a1->x - X1*W1; p.y += a1->y - Y1*W1; p.z += a1->z - Z1*W1;
                }
            }
            *crossing = p / 2.f;
        }

        return res;
/*
        xVector4 plane;
        plane.vector3 = xVector3::CrossProduct(*a2-*a1,*b1-*a1);
        plane.w = -xVector3::DotProduct(*a1, plane.vector3);
        float P1a = xVector3::DotProduct(*b2, plane.vector3) + plane.w;
        float P1b = xVector3::DotProduct(*b3, plane.vector3) + plane.w;
        plane.vector3 = xVector3::CrossProduct(*a3-*a1,*b1-*a1);
        plane.w = -xVector3::DotProduct(*a1, plane.vector3);
        float P2a = xVector3::DotProduct(*b3, plane.vector3) + plane.w;
        float P2b = xVector3::DotProduct(*b2, plane.vector3) + plane.w;

        return P1a*P2a < 0.f || P1b*P2b < 0.f;
*/
    }
}



bool CD_MeshToMesh:: CheckOctreeLevel(xCollisionHierarchyBoundsRoot *ci1, xCollisionHierarchyBoundsRoot *ci2,
                                      xCollisionHierarchy *ch1,        xCollisionHierarchy *ch2,
                                      xCollisionHierarchyBounds *chb1, xCollisionHierarchyBounds *chb2,
                                      xWORD cnt1,                      xWORD cnt2,
                                      xElement *elem1,                 xElement *elem2)
{
    bool res = false;
    xCollisionHierarchy *pch2 = ch2;
    xCollisionHierarchyBounds *pchb2 = chb2;
    xVector3 colPoint;

    for (int h1 = cnt1; h1; --h1, ++chb1, ++ch1)
    {
        ch2 = pch2; chb2 = pchb2;
        for (int h2 = cnt2; h2; --h2, ++chb2, ++ch2)
        {
            ++Performance.CollidedPreTreeLevels;

            if (chb1->bounding.min.x < chb2->bounding.max.x &&
                chb1->bounding.max.x > chb2->bounding.min.x &&
                chb1->bounding.min.y < chb2->bounding.max.y &&
                chb1->bounding.max.y > chb2->bounding.min.y &&
                chb1->bounding.min.z < chb2->bounding.max.z &&
                chb1->bounding.max.z > chb2->bounding.min.z)
            {
                if (ch1->kidsP && ch2->kidsP)
                {
                    res |= CheckOctreeLevel(ci1, ci2, ch1->kidsP, ch2->kidsP, chb1->kids, chb2->kids,
                                            ch1->kidsC, ch2->kidsC, elem1, elem2);
                    continue;
                }
                if (ch1->kidsP)
                {
                    res |= CheckOctreeLevel(ci1, ci2, ch1->kidsP, ch2, chb1->kids, chb2,
                                            ch1->kidsC, 1, elem1, elem2);
                    continue;
                }
                if (ch2->kidsP)
                {
                    res |= CheckOctreeLevel(ci1, ci2, ch1, ch2->kidsP, chb1, chb2->kids,
                                            1, ch2->kidsC, elem1, elem2);
                    continue;
                }
                ++Performance.CollidedTreeLevels;

                for (int i1 = ch1->facesC; i1; --i1)
                {
                    xFace **face1 = ch1->facesP + ch1->facesC - i1;
                    xVector4 *a1 = ci1->verticesP + (**face1)[0];
                    xVector4 *a2 = ci1->verticesP + (**face1)[1];
                    xVector4 *a3 = ci1->verticesP + (**face1)[2];

                    xBox faceB1;
                    faceB1.min.x = min(a1->x, min(a2->x, a3->x));
                    faceB1.min.y = min(a1->y, min(a2->y, a3->y));
                    faceB1.min.z = min(a1->z, min(a2->z, a3->z));
                    faceB1.max.x = max(a1->x, max(a2->x, a3->x));
                    faceB1.max.y = max(a1->y, max(a2->y, a3->y));
                    faceB1.max.z = max(a1->z, max(a2->z, a3->z));

                    for (int i2 = ch2->facesC; i2; --i2)
                    {
                        ++Performance.CollidedTriangleBounds;

                        xFace **face2 = ch2->facesP + ch2->facesC - i2;
                        xVector4 *b1 = ci2->verticesP + (**face2)[0];
                        xVector4 *b2 = ci2->verticesP + (**face2)[1];
                        xVector4 *b3 = ci2->verticesP + (**face2)[2];
                        // Exclude elements that can't collide
                        xBox faceB2;
                        faceB2.max.x = max(b1->x, max(b2->x, b3->x));
                        if (faceB1.min.x > faceB2.max.x) continue;
                        faceB2.min.x = min(b1->x, min(b2->x, b3->x));
                        if (faceB1.max.x < faceB2.min.x) continue;
                        faceB2.max.y = max(b1->y, max(b2->y, b3->y));
                        if (faceB1.min.y > faceB2.max.y) continue;
                        faceB2.min.y = min(b1->y, min(b2->y, b3->y));
                        if (faceB1.max.y < faceB2.min.y) continue;
                        faceB2.max.z = max(b1->z, max(b2->z, b3->z));
                        if (faceB1.min.z > faceB2.max.z) continue;
                        faceB2.min.z = min(b1->z, min(b2->z, b3->z));
                        if (faceB1.max.z < faceB2.min.z) continue;

                        ++Performance.CollidedTriangles;

                        if (IntersectTriangles(a1,a2,a3,b1,b2,b3,&colPoint))
                        {
                            if (!collidedModel1)
                            {
                                model1->CollidedModels.resize(model1->CollidedModels.size()+1);
                                collidedModel1 = &* (model1->CollidedModels.rbegin());
                                collidedModel1->model2 = model2;
                                model2->CollidedModels.resize(model2->CollidedModels.size()+1);
                                collidedModel2 = &* (model2->CollidedModels.rbegin());
                                collidedModel2->model2 = model1;
                            }

                            Collisions cf;
                            cf.face1 = *face1; cf.face2 = *face2;
                            cf.elem1 = elem1; cf.elem2 = elem2; cf.colPoint = colPoint;
                            cf.face1v[0] = a1->vector3; cf.face1v[1] = a2->vector3; cf.face1v[2] = a3->vector3;
                            cf.face2v[0] = b1->vector3; cf.face2v[1] = b2->vector3; cf.face2v[2] = b3->vector3;
                            collidedModel1->collisions.push_back(cf);
                            cf.face1 = *face2; cf.face2 = *face1;
                            cf.elem1 = elem2; cf.elem2 = elem1;
                            cf.face1v[0] = b1->vector3; cf.face1v[1] = b2->vector3; cf.face1v[2] = b3->vector3;
                            cf.face2v[0] = a1->vector3; cf.face2v[1] = a2->vector3; cf.face2v[2] = a3->vector3;
                            collidedModel2->collisions.push_back(cf);
                            res = true;
                        }
                    }
                }
            }
        }
    }
    return res;
}

// Scan all elements in model2
bool CD_MeshToMesh:: Collide2(xCollisionHierarchyBoundsRoot *ci1, xCollisionHierarchyBoundsRoot *&ci2,
                              xElement *elem1, xElement *elem2)
{
    bool res = false;

    if (elem2->verticesC && ci1->kids && ci2->kids &&
        ci1->bounding.min.x < ci2->bounding.max.x &&
        ci1->bounding.max.x > ci2->bounding.min.x &&
        ci1->bounding.min.y < ci2->bounding.max.y &&
        ci1->bounding.max.y > ci2->bounding.min.y &&
        ci1->bounding.min.z < ci2->bounding.max.z &&
        ci1->bounding.max.z > ci2->bounding.min.z)
        res |= CheckOctreeLevel(ci1, ci2,
            elem1->collisionData.kidsP, elem2->collisionData.kidsP, ci1->kids, ci2->kids,
            elem1->collisionData.kidsC, elem2->collisionData.kidsC, elem1, elem2);

    for (xElement *celem2 = elem2->kidsP; celem2; celem2 = celem2->nextP)
        res |= Collide2(ci1, ++ci2, elem1, celem2);

    return res;
}

// Scan all elements in model1
bool CD_MeshToMesh:: Collide1(xCollisionHierarchyBoundsRoot *&ci1, xCollisionHierarchyBoundsRoot *&ci2,
                              xElement *elem1, xElement *elem2)
{
    xCollisionHierarchyBoundsRoot *cci2 = ci2;
    bool res = false;

    if (elem1->verticesC)
        for (xElement *celem2 = elem2; celem2; celem2 = celem2->nextP)
            res |= Collide2(ci1, ++cci2, elem1, celem2);

    for (xElement *celem1 = elem1->kidsP; celem1; celem1 = celem1->nextP)
        res |= Collide1(++ci1, ci2, celem1, elem2);

    return res;
}
