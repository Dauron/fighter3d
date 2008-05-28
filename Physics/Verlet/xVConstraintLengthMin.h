#ifndef __incl_Physics_Verlet_xVConstraintLengthMin_h
#define __incl_Physics_Verlet_xVConstraintLengthMin_h

#include "xVConstraint.h"

struct xVConstraintLengthMin : xVConstraint
{
    xVConstraintLengthMin() { Type = Constraint_LengthMin; }
    virtual ~xVConstraintLengthMin() {}

    xWORD  particleA, particleB;
    xFLOAT minLength, minLengthSqr;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xVConstraint *Load( FILE *file );
};

#endif
