#ifndef __incl_Physics_Verlet_VConstraintLengthMax_h
#define __incl_Physics_Verlet_VConstraintLengthMax_h

#include "VConstraint.h"

struct VConstraintLengthMax : VConstraint
{
    VConstraintLengthMax() { Type = Constraint_LengthMax; }
    virtual ~VConstraintLengthMax() {}

    xWORD  particleA, particleB;
    xFLOAT maxLength, maxLengthSqr;

    virtual bool Satisfy(VerletSystem *system);
    virtual void CloneTo(VConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual VConstraint *Load( FILE *file );
};

#endif
