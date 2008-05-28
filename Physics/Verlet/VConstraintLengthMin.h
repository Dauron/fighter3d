#ifndef __incl_Physics_Verlet_VConstraintLengthMin_h
#define __incl_Physics_Verlet_VConstraintLengthMin_h

#include "VConstraint.h"

struct VConstraintLengthMin : VConstraint
{
    VConstraintLengthMin() { Type = Constraint_LengthMin; }
    virtual ~VConstraintLengthMin() {}

    xWORD  particleA, particleB;
    xFLOAT minLength, minLengthSqr;

    virtual bool Satisfy(VerletSystem *system);
    virtual void CloneTo(VConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual VConstraint *Load( FILE *file );
};

#endif
