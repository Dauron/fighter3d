#ifndef __incl_Physics_Verlet_xVConstraintAngular_h
#define __incl_Physics_Verlet_xVConstraintAngular_h

#include "xVConstraint.h"

struct xVConstraintAngular : xVConstraint
{
    xVConstraintAngular() { Type = Constraint_Angular; }
    virtual ~xVConstraintAngular() {}

    xWORD    particleRootB, particleRootE, particle;
    xFLOAT   minZ, maxZ;

    xFLOAT   angleMaxX, angleMaxY, angleMinX, angleMinY;

    virtual bool Satisfy(xVerletSystem *system);
    bool Test(const xVector3 &P_rootB, const xVector3 &P_rootE, const xVector3 &P_curr,
              const xVector3 &N_up, const xVector3 &N_front) const;
    virtual void CloneTo(xVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xVConstraint *Load( FILE *file );
};

#endif
