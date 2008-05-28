#ifndef __incl_Physics_Verlet_xVConstraintLengthEql_h
#define __incl_Physics_Verlet_xVConstraintLengthEql_h

#include "xVConstraint.h"

struct xVConstraintLengthEql : xVConstraint
{
    xVConstraintLengthEql() { Type = Constraint_LengthEql; }
    virtual ~xVConstraintLengthEql() {}

    xWORD  particleA, particleB;
    xFLOAT restLength, restLengthSqr;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xVConstraint *&dst) const;

    virtual void          Save( FILE *file );
    virtual xVConstraint *Load( FILE *file );
};

#endif
