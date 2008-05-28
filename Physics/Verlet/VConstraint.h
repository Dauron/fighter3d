#ifndef __incl_Physics_Verlet_VConstraint_h
#define __incl_Physics_Verlet_VConstraint_h

#include "../../Math/xMath.h"
#include <cstdio>
struct VerletSystem;

struct VConstraint {
    enum VConstraint_Type
    {
        Constraint_LengthEql,
        Constraint_LengthMin,
        Constraint_LengthMax,
        Constraint_Angular,
        Constraint_Plane
    } Type;

    virtual ~VConstraint() {}
    virtual bool Satisfy(VerletSystem *system) = 0;
    virtual void CloneTo(VConstraint *&dst) const = 0;
    virtual void Save( FILE *file )
    {
        fwrite(&this->Type, sizeof(VConstraint_Type), 1, file);
    }
    static  VConstraint *LoadType( FILE *file );
    virtual VConstraint *Load( FILE *file ) = 0;
};

#endif
