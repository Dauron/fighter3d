#include "xVConstraintLengthEql.h"
#include "xVerletSolver.h"

bool xVConstraintLengthEql :: Satisfy(xVerletSystem *system)
{
    xVector3 &p1 = system->P_current[particleA];
    xVector3 &p2 = system->P_current[particleB];
    xVector3 delta  = p1-p2;
    xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
    if (fabs(restLengthSqr - deltaLengthSqr) < EPSILON2) return false;
    delta *= restLengthSqr/(deltaLengthSqr+restLengthSqr)-0.5f;
    //delta *= -1.f + restLength / sqrt(deltaLengthSqr);

    xFLOAT w1 = system->M_weight_Inv[particleA];
    xFLOAT w2 = system->M_weight_Inv[particleB];
    if (w1 == 0.f && w2 == 0.f) return false;
    w1 /= (w1+w2);
    w2 = 1.f - w1;

    p1 += w1*delta;
    p2 -= w2*delta;

    return true;
}

void xVConstraintLengthEql :: CloneTo(xVConstraint *&dst) const
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
    xVConstraint::Save(file);
    fwrite(&particleA, sizeof(xWORD), 1, file);
    fwrite(&particleB, sizeof(xWORD), 1, file);
    fwrite(&restLength, sizeof(xFLOAT), 1, file);
    fwrite(&restLengthSqr, sizeof(xFLOAT), 1, file);
}

xVConstraint * xVConstraintLengthEql :: Load( FILE *file )
{
    fread(&particleA, sizeof(xWORD), 1, file);
    fread(&particleB, sizeof(xWORD), 1, file);
    fread(&restLength, sizeof(xFLOAT), 1, file);
    fread(&restLengthSqr, sizeof(xFLOAT), 1, file);
    return this;
}
