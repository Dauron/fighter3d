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
    xVector3 &p1 = system->P_current[particleA];
    xVector3 &p2 = system->P_current[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (fabs(restLengthSqr - deltaLengthSqr) < EPSILON2) return false;
    //delta *= restLengthSqr/(deltaLengthSqr+restLengthSqr)-0.5f;
    delta *= -1.f + restLength / sqrt(deltaLengthSqr);

    xFLOAT w1 = system->M_weight_Inv[particleA];
    xFLOAT w2 = system->M_weight_Inv[particleB];
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
    xVector3 &p1 = system->P_current[particleA];
    xVector3 &p2 = system->P_current[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (minLengthSqr - deltaLengthSqr < 0.f) return false;
    delta *= minLengthSqr/(deltaLengthSqr+minLengthSqr)-0.5f;

    xFLOAT w1 = system->M_weight_Inv[particleA];
    xFLOAT w2 = system->M_weight_Inv[particleB];
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
    xVector3 &p1 = system->P_current[particleA];
    xVector3 &p2 = system->P_current[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (deltaLengthSqr - maxLengthSqr < 0.f) return false;
    delta *= maxLengthSqr/(deltaLengthSqr+maxLengthSqr)-0.5f;

    xFLOAT w1 = system->M_weight_Inv[particleA];
    xFLOAT w2 = system->M_weight_Inv[particleB];
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
    xVector3 &pRootB = system->P_current[particleRootB];
    xVector3 &pRootE = system->P_current[particleRootE];
    xVector3 &p      = system->P_current[particle];

    xVector4 qParent, qCurrent;
    // OPTIMIZE: calc only needed quats
    system->spine->CalcQuats(system->P_current, 0, system->MX_WorldToModel_T);
    xBoneCalculateQuatForVerlet(*system->spine, particle, qParent, qCurrent);
    qCurrent = xQuaternion::product(qParent, qCurrent);

    xIKNode &bone  = system->spine->boneP[particle];
    xVector3 up    = system->MX_ModelToWorld.preTransformV(xQuaternion::rotate(qParent, bone.pointE-bone.pointB)).normalize();
    xVector3 front = system->MX_ModelToWorld.preTransformV(xQuaternion::rotate(qParent, xVector3::Create(0,1,0)));

    xMatrix  MX_WorldToBone = xMatrixFromVectors( front, up, - pRootE ).invert();
    xVector3 position       = MX_WorldToBone.preTransformP( p );
    xVector3 positionNorm   = xVector3::Normalize( position );

    xFLOAT alpha   = atan2(fabs(position.y), fabs(position.x));
    xFLOAT scale   = 2*alpha * PI_inv;
    xFLOAT betaMax;

    if (position.x >= 0 && position.y >= 0)
        betaMax = angleMaxY * scale + angleMaxX * (1.f - scale);
    else if (position.x < 0 && position.y >= 0)
        betaMax = angleMaxY * scale + angleMinX * (1.f - scale);
    else if (position.x >= 0 && position.y < 0)
        betaMax = angleMinY * scale + angleMaxX * (1.f - scale);
    else
        betaMax = angleMinY * scale + angleMinX * (1.f - scale);

    xFLOAT cosMax = cos(betaMax);
    if (positionNorm.z >= cosMax) return false;

    xVector3 positionNew;
    xFLOAT xy     = sin(betaMax);
    positionNew.x = xy*cos(alpha);
    positionNew.y = xy*sin(alpha);
    positionNew.z = cosMax;

    xVector4 quat = xQuaternion::getRotation(positionNorm, positionNew);

    xFLOAT w1 = system->M_weight_Inv[particle];
    xFLOAT w2 = system->M_weight_Inv[particleRootE];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    xVector4 quatP = xQuaternion::interpolate(quat, w1);
    xVector4 quatR = xQuaternion::interpolate(quat, w2);
    xMatrix &MX_BoneToWorld = MX_WorldToBone.invert();
    pRootE = MX_BoneToWorld.preTransformP( xQuaternion::rotate(quatR, -position) + position );
    p      = MX_BoneToWorld.preTransformP( xQuaternion::rotate(quatP, position) );

    return true;
}
bool xVConstraintAngular :: Test(const xVector3 &pRootB, const xVector3 &pRootE, const xVector3 &p,
                                 const xVector3 &up, const xVector3 &front) const
{
    xMatrix transf        = xMatrixFromVectors(front, up, -pRootE).invert();
    xVector3 position     = transf.preTransformP(p);
    xVector3 positionNorm = xVector3::Normalize( position );

    xFLOAT alpha   = atan2(fabs(position.y), fabs(position.x));
    xFLOAT scale   = 2*alpha * PI_inv;
    xFLOAT betaMax;

    if (position.x >= 0 && position.y >= 0)
        betaMax = angleMaxY * scale + angleMaxX * (1.f - scale);
    else if (position.x < 0 && position.y >= 0)
        betaMax = angleMaxY * scale + angleMinX * (1.f - scale);
    else if (position.x >= 0 && position.y < 0)
        betaMax = angleMinY * scale + angleMaxX * (1.f - scale);
    else
        betaMax = angleMinY * scale + angleMinX * (1.f - scale);

    xFLOAT cosMax = cos(betaMax);
    if (positionNorm.z >= cosMax) return false;

    return true;
}

void xVConstraintAngular :: CloneTo(xIVConstraint *&dst) const
{
    xVConstraintAngular *res = new xVConstraintAngular();
    res->particleRootB = particleRootB;
    res->particleRootE = particleRootE;
    res->particle      = particle;
    res->angleMaxX     = angleMaxX;
    res->angleMaxY     = angleMaxY;
    res->angleMinX     = angleMinX;
    res->angleMinY     = angleMinY;
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
    fwrite(&angleMaxX, sizeof(xFLOAT), 1, file);
    fwrite(&angleMaxY, sizeof(xFLOAT), 1, file);
    fwrite(&angleMinX, sizeof(xFLOAT), 1, file);
    fwrite(&angleMinY, sizeof(xFLOAT), 1, file);
    fwrite(&minZ, sizeof(xFLOAT), 1, file);
    fwrite(&maxZ, sizeof(xFLOAT), 1, file);
    fwrite(&upQuat, sizeof(xVector4), 1, file);
}

xIVConstraint * xVConstraintAngular :: Load( FILE *file )
{
    fread(&particleRootB, sizeof(xWORD), 1, file);
    fread(&particleRootE, sizeof(xWORD), 1, file);
    fread(&particle, sizeof(xWORD), 1, file);
    fread(&angleMaxX, sizeof(xFLOAT), 1, file);
    fread(&angleMaxY, sizeof(xFLOAT), 1, file);
    fread(&angleMinX, sizeof(xFLOAT), 1, file);
    fread(&angleMinY, sizeof(xFLOAT), 1, file);
    fread(&minZ, sizeof(xFLOAT), 1, file);
    fread(&maxZ, sizeof(xFLOAT), 1, file);
    fread(&upQuat, sizeof(xVector4), 1, file);
    return this;
}

// COLLISION
bool xVConstraintCollision :: Satisfy(xVerletSystem *system)
{
    xVector3 &p = system->P_current[particle];
    xFLOAT dist = xVector3::DotProduct(planeN, p) + planeD;
    if (dist > 0.f) return false;
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

// VERLET SOLVER
const float xVerletSolver :: FRICTION_AIR = 0.5f;
const float xVerletSolver :: GRAVITY      = 10.f;

void xVerletSolver :: SatisfyConstraints()
{
    for (xBYTE pi = I_passes; pi; --pi)
    {
        if (system->C_collisions)
        {
            xVConstraintCollisionVector::iterator iter = system->C_collisions->begin(),
                                                  end  = system->C_collisions->end();
            for (; iter != end; ++iter)
                iter->Satisfy(system);
        }

        if (system->C_lengthConst)
        {
            xVConstraintLengthEql * iterL = system->C_lengthConst;
            xVConstraintLengthEql * endL  = system->C_lengthConst + system->I_particles-1;
            for (; iterL != endL; ++iterL)
                iterL->Satisfy(system);
        }

        xIVConstraint **constr = system->C_constraints;
        for (xWORD i = system->I_constraints; i; --i, ++constr)
            (*constr)->Satisfy(system);
    }
}

void xVerletSolver :: AccumulateForces()
{
    xVector3 *A_iter = system->A_forces;
    for (xWORD i = system->I_particles; i; --i, ++A_iter) *A_iter += A_gravity;
}

void xVerletSolver :: Verlet()
{
    xVector3 *P_cur  = system->P_current;
    xVector3 *P_old  = system->P_previous;
    xVector3 *A_iter = system->A_forces;
    xFLOAT    T_step_Sqr = system->T_step*system->T_step;

    if (A_iter)
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old, ++A_iter)
            *P_old = *P_cur /*+ 0.9f * (*P_cur - *P_old)*/ + *A_iter * T_step_Sqr;
    else
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old)
            *P_old = *P_cur /*+ 0.9f * (*P_cur - *P_old)*/;

    system->SwapPositions();
}

void xVerletSolver :: VerletFull()
{
    xVector3 *P_cur  = system->P_current;
    xVector3 *P_old  = system->P_previous;
    xVector3 *A_iter = system->A_forces;
    xFLOAT    T_step_Sqr = system->T_step*system->T_step;
    xFLOAT    F_friction_Inv = 1.f - (FRICTION_AIR * system->T_step);
    if (system->T_step_Old > 0.f)
        F_friction_Inv *= system->T_step / system->T_step_Old;

    if (A_iter)
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old, ++A_iter)
            *P_old = *P_cur + F_friction_Inv * (*P_cur - *P_old) + *A_iter * T_step_Sqr;
    else
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old)
            *P_old = *P_cur + F_friction_Inv * (*P_cur - *P_old);

    system->SwapPositions();
}