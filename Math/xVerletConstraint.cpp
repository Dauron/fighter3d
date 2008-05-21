#include "xVerletConstraint.h"

xIVConstraint * xIVConstraint :: LoadType( FILE *file )
{
    xIVConstraint_Type type;
    xIVConstraint *res = NULL;
    fread(&type, sizeof(xIVConstraint_Type), 1, file);
    switch (type)
    {
        case Constraint_LengthEql:
            res = (new xVConstraintLengthEql())->Load(file);
            break;
        case Constraint_LengthMin:
            res = (new xVConstraintLengthMin())->Load(file);
            break;
        case Constraint_LengthMax:
            res = (new xVConstraintLengthMax())->Load(file);
            break;
        case Constraint_Angular:
            res = (new xVConstraintAngular())->Load(file);
            break;
        case Constraint_Plane:
            res = (new xVConstraintCollision())->Load(file);
            break;
    }
    return res;
}

bool xVConstraintLengthEql :: Satisfy(xVerletSystem *system)
{
    xVector3 &p1 = system->positionP[particleA];
    xVector3 &p2 = system->positionP[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (fabs(restLengthSqr - deltaLengthSqr) < EPSILON2) return false;
    delta *= restLengthSqr/(deltaLengthSqr+restLengthSqr)-0.5f;
    //delta *= -1.f + restLength / sqrt(deltaLengthSqr);

    xFLOAT w1 = system->weightP[particleA];
    xFLOAT w2 = system->weightP[particleB];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    p1 += w1*delta;
    p2 -= w2*delta;

    return true;
}

void xVConstraintLengthEql :: CloneTo(xIVConstraint *&dst) const
{
    xVConstraintLengthEql *res = new xVConstraintLengthEql();
    res->particleA     = particleA;
    res->particleB     = particleB;
    res->restLength    = restLength;
    res->restLengthSqr = restLengthSqr;
    dst = res;
}

void xVConstraintLengthEql :: Save( FILE *file )
{
    xIVConstraint::Save(file);
    fwrite(&particleA, sizeof(xWORD), 1, file);
    fwrite(&particleB, sizeof(xWORD), 1, file);
    fwrite(&restLength, sizeof(xFLOAT), 1, file);
    fwrite(&restLengthSqr, sizeof(xFLOAT), 1, file);
}

xIVConstraint * xVConstraintLengthEql :: Load( FILE *file )
{
    fread(&particleA, sizeof(xWORD), 1, file);
    fread(&particleB, sizeof(xWORD), 1, file);
    fread(&restLength, sizeof(xFLOAT), 1, file);
    fread(&restLengthSqr, sizeof(xFLOAT), 1, file);
    return this;
}

bool xVConstraintLengthMin :: Satisfy(xVerletSystem *system)
{
    xVector3 &p1 = system->positionP[particleA];
    xVector3 &p2 = system->positionP[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (minLengthSqr - deltaLengthSqr < 0.f) return false;
    delta *= minLengthSqr/(deltaLengthSqr+minLengthSqr)-0.5f;

    xFLOAT w1 = system->weightP[particleA];
    xFLOAT w2 = system->weightP[particleB];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    p1 += w1*delta;
    p2 -= (1.f-w1)*delta;

    return true;
}

void xVConstraintLengthMin :: CloneTo(xIVConstraint *&dst) const
{
    xVConstraintLengthMin *res = new xVConstraintLengthMin();
    res->particleA    = particleA;
    res->particleB    = particleB;
    res->minLength    = minLength;
    res->minLengthSqr = minLengthSqr;
    dst = res;
}

void xVConstraintLengthMin :: Save( FILE *file )
{
    xIVConstraint::Save(file);
    fwrite(&particleA, sizeof(xWORD), 1, file);
    fwrite(&particleB, sizeof(xWORD), 1, file);
    fwrite(&minLength, sizeof(xFLOAT), 1, file);
    fwrite(&minLengthSqr, sizeof(xFLOAT), 1, file);
}

xIVConstraint * xVConstraintLengthMin :: Load( FILE *file )
{
    fread(&particleA, sizeof(xWORD), 1, file);
    fread(&particleB, sizeof(xWORD), 1, file);
    fread(&minLength, sizeof(xFLOAT), 1, file);
    fread(&minLengthSqr, sizeof(xFLOAT), 1, file);
    return this;
}

bool xVConstraintLengthMax :: Satisfy(xVerletSystem *system)
{
    xVector3 &p1 = system->positionP[particleA];
    xVector3 &p2 = system->positionP[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (deltaLengthSqr - maxLengthSqr < 0.f) return false;
    delta *= maxLengthSqr/(deltaLengthSqr+maxLengthSqr)-0.5f;

    xFLOAT w1 = system->weightP[particleA];
    xFLOAT w2 = system->weightP[particleB];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    p1 += w1*delta;
    p2 -= (1.f-w1)*delta;

    return true;
}

void xVConstraintLengthMax :: CloneTo(xIVConstraint *&dst) const
{
    xVConstraintLengthMax *res = new xVConstraintLengthMax();
    res->particleA    = particleA;
    res->particleB    = particleB;
    res->maxLength    = maxLength;
    res->maxLengthSqr = maxLengthSqr;
    dst = res;
}

void xVConstraintLengthMax :: Save( FILE *file )
{
    xIVConstraint::Save(file);
    fwrite(&particleA, sizeof(xWORD), 1, file);
    fwrite(&particleB, sizeof(xWORD), 1, file);
    fwrite(&maxLength, sizeof(xFLOAT), 1, file);
    fwrite(&maxLengthSqr, sizeof(xFLOAT), 1, file);
}

xIVConstraint * xVConstraintLengthMax :: Load( FILE *file )
{
    fread(&particleA, sizeof(xWORD), 1, file);
    fread(&particleB, sizeof(xWORD), 1, file);
    fread(&maxLength, sizeof(xFLOAT), 1, file);
    fread(&maxLengthSqr, sizeof(xFLOAT), 1, file);
    return this;
}
// ANGULAR
#include "../Models/lib3dx/xSkeleton.h"

bool xVConstraintAngular :: Satisfy(xVerletSystem *system)
{
    xVector3 &pRootB = system->positionP[particleRootB];
    xVector3 &pRootE = system->positionP[particleRootE];
    xVector3 &p      = system->positionP[particle];

    xVector4 qParent, qCurrent;
    // OPTIMIZE: calc only needed quats
    system->spine->CalcQuats(system->positionP, 0, system->locationMatrixIT);
    xBoneCalculateQuatForVerlet(*system->spine, particle, qParent, qCurrent);
    qCurrent = xQuaternion::product(qParent, qCurrent);

    xIKNode &bone  = system->spine->boneP[particle];
    xVector3 up    = system->locationMatrix.preTransformV(xQuaternion::rotate(qParent, bone.pointE-bone.pointB)).normalize();
    xVector3 front = system->locationMatrix.preTransformV(xQuaternion::rotate(qParent, xVector3::Create(0,1,0)));

    xMatrix  transf   = xMatrixFromVectors(front, up, pRootE);
    xVector3 position = transf.preTransformP(p);
    xVector3 positionNorm = xVector3::Normalize( position );
    xFLOAT cosCur = xVector3::DotProduct(xVector3::Create(0,0,1), positionNorm);

    xFLOAT lenXYinv = position.x*position.x + position.y*position.y;
    xFLOAT sinT, cosT;
    if (lenXYinv != 0.f)
    {
        lenXYinv = 1 / sqrt(lenXYinv);
        sinT = position.y * lenXYinv;
        cosT = position.x * lenXYinv;
    }
    else
    {
        sinT = 0.f;
        cosT = 1.f;
    }
    xFLOAT zSignX, zSignY, Xm, Ym;
    if (cosT >= 0 && sinT >= 0)
    {
        Xm = elipseMaxX;
        Ym = elipseMaxY;
        zSignX = zSignMaxX;
        zSignY = zSignMaxY;
    }
    else if (cosT < 0 && sinT >= 0)
    {
        Xm = elipseMinX;
        Ym = elipseMaxY;
        zSignX = zSignMinX;
        zSignY = zSignMaxY;
    }
    else if (cosT >= 0 && sinT < 0)
    {
        Xm = elipseMaxX;
        Ym = elipseMinY;
        zSignX = zSignMaxX;
        zSignY = zSignMinY;
    }
    else
    {
        Xm = elipseMinX;
        Ym = elipseMinY;
        zSignX = zSignMinX;
        zSignY = zSignMinY;
    }

    xFLOAT fullX = Xm;
    xFLOAT fullY = Ym;
    Xm *= cosT;
    Ym *= sinT;

    xVector3 positionNew;
    positionNew.x = Xm; positionNew.y = Ym;
    if (zSignX*zSignY > 0)
        positionNew.z = zSignX * sqrt(max(1 - Xm*Xm - Ym*Ym, 0.f));
    else
    {
        xFLOAT ZmX = cosT >= 0 ? elipseMaxX : elipseMinX;
               ZmX = zSignX * sqrt(max(1 - ZmX*ZmX, 0.f));
        xFLOAT ZmY = sinT >= 0 ? elipseMaxY : elipseMinY;
               ZmY = zSignY * sqrt(max(1 - ZmY*ZmY, 0.f));
        positionNew.z = cosT*ZmX + (1-cosT)*ZmY;
        positionNew.normalize();
    }

    xFLOAT cosMax = xVector3::DotProduct(xVector3::Create(0,0,1), positionNew);
    if (cosCur >= cosMax) return false;
/*
    if (( (zSignX < 0 && positionNorm.z >= 0.f) ||
          (zSignX < 0 || positionNorm.z >= 0.f)
           zSignX * fabs(positionNorm.x) <= zSignX*XmP) &&
        ( (zSignY < 0 && positionNorm.z >= 0.f) || zSignY * fabs(positionNorm.y) <= zSignY*YmP) )
            return false;
*/

    if (fullX > fullY)
    {
        xFLOAT scale = fabs(fullY / fullX);
        xFLOAT maxX = min(fullX, positionNorm.x);
        Xm = maxX - (maxX - Xm)*scale;
        Ym = Sign(positionNorm.y) * sqrt(max(fullY*fullY - scale*scale*Xm*Xm, 0.f));
        Ym = Ym;
    }
    else
    if (fullY != 0)
    {
        xFLOAT scale = fabs(fullX / fullY);
        xFLOAT maxY = min(fullY, positionNorm.y);
        Ym = maxY - (maxY - Ym)*scale;
        Xm = Sign(positionNorm.x) * sqrt(max(fullX*fullX - scale*scale*Ym*Ym, 0.f));
    }

    positionNew.x = Xm; positionNew.y = Ym;
    if (zSignX*zSignY > 0)
        positionNew.z = zSignX * sqrt(max(1 - Xm*Xm - Ym*Ym, 0.f));
    else
    {
        xFLOAT ZmX = cosT >= 0 ? elipseMaxX : elipseMinX;
               ZmX = zSignX * sqrt(max(1 - ZmX*ZmX, 0.f));
        xFLOAT ZmY = sinT >= 0 ? elipseMaxY : elipseMinY;
               ZmY = zSignY * sqrt(max(1 - ZmY*ZmY, 0.f));
        positionNew.z = cosT*ZmX + (1-cosT)*ZmY;
        positionNew.normalize();
    }

    xVector4 quat = xQuaternion::getRotation(positionNorm, positionNew);

    xFLOAT w1 = system->weightP[particle];
    xFLOAT w2 = system->weightP[particleRootE];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    xVector4 quatP = xQuaternion::interpolate(quat, w1);
    xVector4 quatR = xQuaternion::interpolate(quat, w2);
    transf.invert();
    pRootE = transf.preTransformP( xQuaternion::rotate(quatR, -position) + position );
    p      = transf.preTransformP( xQuaternion::rotate(quatP, position) );

    return true;
}
bool xVConstraintAngular :: Test(const xVector3 &pRootB, const xVector3 &pRootE, const xVector3 &p,
                                 const xVector3 &up, const xVector3 &front) const
{
    xMatrix transf    = xMatrixFromVectors(front, up, pRootE);
    xVector3 position = transf.preTransformP(p);
    xVector3 positionNorm = xVector3::Normalize( position );

    xFLOAT lenXYinv = position.x*position.x + position.y*position.y;
    xFLOAT sinT, cosT;
    if (lenXYinv != 0.f)
    {
        lenXYinv = 1 / sqrt(lenXYinv);
        sinT = position.y * lenXYinv;
        cosT = position.x * lenXYinv;
    }
    else
    {
        sinT = 0.f;
        cosT = 1.f;
    }
    xFLOAT zSignX, zSignY, Xm, Ym;
    if (cosT >= 0 && sinT >= 0)
    {
        Xm = elipseMaxX;
        Ym = elipseMaxY;
        zSignX = zSignMaxX;
        zSignY = zSignMaxY;
    }
    else if (cosT < 0 && sinT >= 0)
    {
        Xm = elipseMinX;
        Ym = elipseMaxY;
        zSignX = zSignMinX;
        zSignY = zSignMaxY;
    }
    else if (cosT >= 0 && sinT < 0)
    {
        Xm = elipseMaxX;
        Ym = elipseMinY;
        zSignX = zSignMaxX;
        zSignY = zSignMinY;
    }
    else
    {
        Xm = elipseMinX;
        Ym = elipseMinY;
        zSignX = zSignMinX;
        zSignY = zSignMinY;
    }

    Xm *= cosT;
    Ym *= sinT;

    xVector3 positionNew;
    positionNew.x = Xm; positionNew.y = Ym;
    if (zSignX*zSignY > 0)
        positionNew.z = zSignX * sqrt(max(1 - Xm*Xm - Ym*Ym, 0.f));
    else
    {
        xFLOAT ZmX = cosT >= 0 ? elipseMaxX : elipseMinX;
               ZmX = zSignX * sqrt(max(1 - ZmX*ZmX, 0.f));
        xFLOAT ZmY = sinT >= 0 ? elipseMaxY : elipseMinY;
               ZmY = zSignY * sqrt(max(1 - ZmY*ZmY, 0.f));
        positionNew.z = cosT*ZmX + (1-cosT)*ZmY;
        positionNew.normalize();
    }

    xFLOAT cosMax = xVector3::DotProduct(xVector3::Create(0,0,1), positionNew);
    xFLOAT cosCur = xVector3::DotProduct(xVector3::Create(0,0,1), positionNorm);
    if (cosCur >= cosMax) return false;

    return true;
}

void xVConstraintAngular :: CloneTo(xIVConstraint *&dst) const
{
    xVConstraintAngular *res = new xVConstraintAngular();
    res->particleRootB = particleRootB;
    res->particleRootE = particleRootE;
    res->particle      = particle;
    res->elipseMaxX    = elipseMaxX;
    res->elipseMaxY    = elipseMaxY;
    res->elipseMinX    = elipseMinX;
    res->elipseMinY    = elipseMinY;
    res->zSignMaxX     = zSignMaxX;
    res->zSignMaxY     = zSignMaxY;
    res->zSignMinX     = zSignMinX;
    res->zSignMinY     = zSignMinY;
    res->minZ          = minZ;
    res->maxZ          = maxZ;
    res->upQuat        = upQuat;
    dst = res;
}

void xVConstraintAngular :: Save( FILE *file )
{
    xIVConstraint::Save(file);
    fwrite(&particleRootB, sizeof(xWORD), 1, file);
    fwrite(&particleRootE, sizeof(xWORD), 1, file);
    fwrite(&particle, sizeof(xWORD), 1, file);
    fwrite(&elipseMaxX, sizeof(xFLOAT), 1, file);
    fwrite(&elipseMaxY, sizeof(xFLOAT), 1, file);
    fwrite(&elipseMinX, sizeof(xFLOAT), 1, file);
    fwrite(&elipseMinY, sizeof(xFLOAT), 1, file);
    fwrite(&zSignMaxX, sizeof(xCHAR), 1, file);
    fwrite(&zSignMaxY, sizeof(xCHAR), 1, file);
    fwrite(&zSignMinX, sizeof(xCHAR), 1, file);
    fwrite(&zSignMinY, sizeof(xCHAR), 1, file);
    fwrite(&minZ, sizeof(xFLOAT), 1, file);
    fwrite(&maxZ, sizeof(xFLOAT), 1, file);
    fwrite(&upQuat, sizeof(xVector4), 1, file);
}

xIVConstraint * xVConstraintAngular :: Load( FILE *file )
{
    fread(&particleRootB, sizeof(xWORD), 1, file);
    fread(&particleRootE, sizeof(xWORD), 1, file);
    fread(&particle, sizeof(xWORD), 1, file);
    fread(&elipseMaxX, sizeof(xFLOAT), 1, file);
    fread(&elipseMaxY, sizeof(xFLOAT), 1, file);
    fread(&elipseMinX, sizeof(xFLOAT), 1, file);
    fread(&elipseMinY, sizeof(xFLOAT), 1, file);
    fread(&zSignMaxX, sizeof(xCHAR), 1, file);
    fread(&zSignMaxY, sizeof(xCHAR), 1, file);
    fread(&zSignMinX, sizeof(xCHAR), 1, file);
    fread(&zSignMinY, sizeof(xCHAR), 1, file);
    fread(&minZ, sizeof(xFLOAT), 1, file);
    fread(&maxZ, sizeof(xFLOAT), 1, file);
    fread(&upQuat, sizeof(xVector4), 1, file);
    return this;
}

// COLLISION
bool xVConstraintCollision :: Satisfy(xVerletSystem *system)
{
    xVector3 &p = system->positionP[particle];
    xFLOAT dist = xVector3::DotProduct(planeN, p) + planeD;
    if (dist > -EPSILON2) return false;
    p -= planeN * dist;
    return true;
}

void xVConstraintCollision :: CloneTo(xIVConstraint *&dst) const
{
    xVConstraintCollision *res = new xVConstraintCollision();
    res->planeN   = planeN;
    res->planeD   = planeD;
    res->particle = particle;
    dst = res;
}

void xVConstraintCollision :: Save( FILE *file )
{
    xIVConstraint::Save(file);
    fwrite(&planeN, sizeof(xVector3), 1, file);
    fwrite(&planeD, sizeof(xFLOAT), 1, file);
    fwrite(&particle, sizeof(xWORD), 1, file);
}

xIVConstraint * xVConstraintCollision :: Load( FILE *file )
{
    fread(&planeN, sizeof(xVector3), 1, file);
    fread(&planeD, sizeof(xFLOAT), 1, file);
    fread(&particle, sizeof(xWORD), 1, file);
    return this;
}

void xVerletSolver :: SatisfyConstraints()
{
    for (xBYTE pass_n = passesC; pass_n; --pass_n)
    {
/*
        if (system->collisions)
        {
            xVConstraintCollisionVector::iterator iter = system->collisions->begin(),
                                                  end  = system->collisions->end();
            for (; iter != end; ++iter)
                iter->Satisfy(system);
        }

        if (system->constraintsLenEql)
        {
            xVConstraintLengthEql * iterL = system->constraintsLenEql;
            xVConstraintLengthEql * endL  = system->constraintsLenEql + system->particleC-1;
            for (; iterL != endL; ++iterL)
                iterL->Satisfy(system);
        }
*/
        xIVConstraint **constr = system->constraintsP;
        for (xWORD i = system->constraintsC; i; --i, ++constr)
            (*constr)->Satisfy(system);
    }
}

void xVerletSolver :: AccumulateForces()
{
    xVector3 *a = system->accelerationP;
    for (xWORD i = system->particleC; i; --i, ++a) *a = gravity;
}

void xVerletSolver :: Verlet()
{
    xVector3 *pC = system->positionP;
    xVector3 *pO = system->positionOldP;
    xVector3 *a  = system->accelerationP;
    xFLOAT    timeStepSqr = timeStep*timeStep;

    if (system->accelerationP)
        for (xWORD i = system->particleC; i; --i, ++pC, ++pO, ++a)
            *pO = *pC + /*2 * *pC - *pO + */*a * timeStepSqr;
    else
        for (xWORD i = system->particleC; i; --i, ++pC, ++pO, ++a)
            *pO = 2 * *pC - *pO;

    system->SwapPositions();
}