#include "VConstraintCollision.h"
#include "VerletSolver.h"

bool VConstraintCollision :: Satisfy(VerletSystem *system)
{
    xVector3 &p = system->P_current[particle];
    xFLOAT dist = xVector3::DotProduct(planeN, p) + planeD;
    if (dist > 0.f) return false;
    p -= planeN * dist;
    return true;
}

void VConstraintCollision :: CloneTo(VConstraint *&dst) const
{
    VConstraintCollision *res = new VConstraintCollision();
    res->planeN   = planeN;
    res->planeD   = planeD;
    res->particle = particle;
    dst = res;
}

void VConstraintCollision :: Save( FILE *file )
{
    VConstraint::Save(file);
    fwrite(&planeN, sizeof(xVector3), 1, file);
    fwrite(&planeD, sizeof(xFLOAT), 1, file);
    fwrite(&particle, sizeof(xWORD), 1, file);
}

VConstraint * VConstraintCollision :: Load( FILE *file )
{
    fread(&planeN, sizeof(xVector3), 1, file);
    fread(&planeD, sizeof(xFLOAT), 1, file);
    fread(&particle, sizeof(xWORD), 1, file);
    return this;
}
