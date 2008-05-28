#ifndef __incl_Physics_Verlet_VConstraintLengthEql_h
#define __incl_Physics_Verlet_VConstraintLengthEql_h

#include "VConstraint.h"

struct VConstraintLengthEql : VConstraint
{
    VConstraintLengthEql() { Type = Constraint_LengthEql; }
    virtual ~VConstraintLengthEql() {}

    xWORD  particleA, particleB;
    xFLOAT restLength, restLengthSqr;

    virtual bool Satisfy(VerletSystem *system);
    virtual void CloneTo(VConstraint *&dst) const;

    virtual void          Save( FILE *file );
    virtual VConstraint *Load( FILE *file );
};

#endif
