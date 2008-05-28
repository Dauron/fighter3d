#include "VConstraintLengthMin.h"
#include "VerletSolver.h"

bool VConstraintLengthMin :: Satisfy(VerletSystem *system)
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

void VConstraintLengthMin :: CloneTo(VConstraint *&dst) const
{
    VConstraintLengthMin *res = new VConstraintLengthMin();
    res->particleA    = particleA;
    res->particleB    = particleB;
    res->minLength    = minLength;
    res->minLengthSqr = minLengthSqr;
    dst = res;
}

void VConstraintLengthMin :: Save( FILE *file )
{
    VConstraint::Save(file);
    fwrite(&particleA, sizeof(xWORD), 1, file);
    fwrite(&particleB, sizeof(xWORD), 1, file);
    fwrite(&minLength, sizeof(xFLOAT), 1, file);
    fwrite(&minLengthSqr, sizeof(xFLOAT), 1, file);
}

VConstraint * VConstraintLengthMin :: Load( FILE *file )
{
    fread(&particleA, sizeof(xWORD), 1, file);
    fread(&particleB, sizeof(xWORD), 1, file);
    fread(&minLength, sizeof(xFLOAT), 1, file);
    fread(&minLengthSqr, sizeof(xFLOAT), 1, file);
    return this;
}
