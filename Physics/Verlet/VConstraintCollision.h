#ifndef __incl_Physics_Verlet_VConstraintCollision_h
#define __incl_Physics_Verlet_VConstraintCollision_h

#include "VConstraint.h"

struct VConstraintCollision : VConstraint
{
    VConstraintCollision() { Type = Constraint_Plane; }
    virtual ~VConstraintCollision() {}

    xVector3 planeN;
    xFLOAT   planeD;
    xWORD    particle;

    virtual bool Satisfy(VerletSystem *system);
    virtual void CloneTo(VConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual VConstraint *Load( FILE *file );
};

#endif
