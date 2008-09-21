#ifndef __incl_Physics_Verlet_VConstraintAngular_h
#define __incl_Physics_Verlet_VConstraintAngular_h

#include "VConstraint.h"

struct VConstraintAngular : VConstraint
{
    VConstraintAngular() { Type = Constraint_Angular; }
    virtual ~VConstraintAngular() {}

    xWORD    particleRootB, particleRootE, particle;
    xFLOAT   minZ, maxZ;

    xFLOAT   angleMaxX, angleMaxY, angleMinX, angleMinY;

    virtual bool Satisfy(VerletSystem *system);
    bool Test(const xVector3 &P_rootB, const xVector3 &P_rootE, const xVector3 &P_curr,
              const xVector3 &N_up, const xVector3 &N_front, const xQuaternion &QT_bone) const;
    virtual void CloneTo(VConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual VConstraint *Load( FILE *file );
};

#endif
