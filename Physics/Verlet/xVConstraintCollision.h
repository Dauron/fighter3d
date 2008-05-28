#ifndef __incl_Physics_Verlet_xVConstraintCollision_h
#define __incl_Physics_Verlet_xVConstraintCollision_h

#include "xVConstraint.h"

struct xVConstraintCollision : xVConstraint
{
    xVConstraintCollision() { Type = Constraint_Plane; }
    virtual ~xVConstraintCollision() {}

    xVector3 planeN;
    xFLOAT   planeD;
    xWORD    particle;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xVConstraint *Load( FILE *file );
};

#endif
