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
        case Constraint_Plane:
            res = (new xVConstraintCollision())->Load(file);
            break;
    }
    return res;
}

bool xVConstraintLengthEql :: Satisfy(xVector3 *pos, xFLOAT *invmass)
{
    xVector3 &p1 = pos[particleA];
    xVector3 &p2 = pos[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (fabs(restLengthSqr - deltaLengthSqr) < EPSILON2) return false;
    delta *= restLengthSqr/(deltaLengthSqr+restLengthSqr)-0.5f;
    //delta *= -1.f + restLength / sqrt(deltaLengthSqr);

    xFLOAT w1 = invmass[particleA];
    xFLOAT w2 = invmass[particleB];
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

bool xVConstraintLengthMin :: Satisfy(xVector3 *pos, xFLOAT *invmass)
{
    xVector3 &p1 = pos[particleA];
    xVector3 &p2 = pos[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (minLengthSqr - deltaLengthSqr < 0.f) return false;
    delta *= minLengthSqr/(deltaLengthSqr+minLengthSqr)-0.5f;

    xFLOAT w1 = invmass[particleA];
    xFLOAT w2 = invmass[particleB];
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

bool xVConstraintLengthMax :: Satisfy(xVector3 *pos, xFLOAT *invmass)
{
    xVector3 &p1 = pos[particleA];
    xVector3 &p2 = pos[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (deltaLengthSqr - maxLengthSqr < 0.f) return false;
    delta *= maxLengthSqr/(deltaLengthSqr+maxLengthSqr)-0.5f;

    xFLOAT w1 = invmass[particleA];
    xFLOAT w2 = invmass[particleB];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    p1 += w1*delta;
    p2 -= (1.f-w1)*delta;

    invmass[particleA] = 0.f;
    invmass[particleB] = 0.f;

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

bool xVConstraintCollision :: Satisfy(xVector3 *pos, xFLOAT *invmass)
{
    xVector3 &p = pos[particle];
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
