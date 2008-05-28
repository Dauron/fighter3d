#include "xVConstraintLengthMax.h"
#include "xVerletSolver.h"

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

void xVConstraintLengthMax :: CloneTo(xVConstraint *&dst) const
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
    xVConstraint::Save(file);
    fwrite(&particleA, sizeof(xWORD), 1, file);
    fwrite(&particleB, sizeof(xWORD), 1, file);
    fwrite(&maxLength, sizeof(xFLOAT), 1, file);
    fwrite(&maxLengthSqr, sizeof(xFLOAT), 1, file);
}

xVConstraint * xVConstraintLengthMax :: Load( FILE *file )
{
    fread(&particleA, sizeof(xWORD), 1, file);
    fread(&particleB, sizeof(xWORD), 1, file);
    fread(&maxLength, sizeof(xFLOAT), 1, file);
    fread(&maxLengthSqr, sizeof(xFLOAT), 1, file);
    return this;
}
