#include "xVConstraintCollision.h"
#include "xVerletSolver.h"

bool xVConstraintCollision :: Satisfy(xVerletSystem *system)
{
    xVector3 &p = system->P_current[particle];
    xFLOAT dist = xVector3::DotProduct(planeN, p) + planeD;
    if (dist > 0.f) return false;
    p -= planeN * dist;
    return true;
}

void xVConstraintCollision :: CloneTo(xVConstraint *&dst) const
{
    xVConstraintCollision *res = new xVConstraintCollision();
    res->planeN   = planeN;
    res->planeD   = planeD;
    res->particle = particle;
    dst = res;
}

void xVConstraintCollision :: Save( FILE *file )
{
    xVConstraint::Save(file);
    fwrite(&planeN, sizeof(xVector3), 1, file);
    fwrite(&planeD, sizeof(xFLOAT), 1, file);
    fwrite(&particle, sizeof(xWORD), 1, file);
}

xVConstraint * xVConstraintCollision :: Load( FILE *file )
{
    fread(&planeN, sizeof(xVector3), 1, file);
    fread(&planeD, sizeof(xFLOAT), 1, file);
    fread(&particle, sizeof(xWORD), 1, file);
    return this;
}
