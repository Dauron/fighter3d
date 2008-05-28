#ifndef __incl_Physics_Verlet_xVConstraintLengthMax_h
#define __incl_Physics_Verlet_xVConstraintLengthMax_h

#include "xVConstraint.h"

struct xVConstraintLengthMax : xVConstraint
{
    xVConstraintLengthMax() { Type = Constraint_LengthMax; }
    virtual ~xVConstraintLengthMax() {}

    xWORD  particleA, particleB;
    xFLOAT maxLength, maxLengthSqr;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xVConstraint *Load( FILE *file );
};

#endif
