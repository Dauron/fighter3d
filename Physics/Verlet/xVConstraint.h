#ifndef __incl_Physics_Verlet_xVConstraint_h
#define __incl_Physics_Verlet_xVConstraint_h

#include "../../Math/xMath.h"
#include <cstdio>
struct xVerletSystem;

struct xVConstraint {
    enum xVConstraint_Type
    {
        Constraint_LengthEql,
        Constraint_LengthMin,
        Constraint_LengthMax,
        Constraint_Angular,
        Constraint_Plane
    } Type;

    virtual ~xVConstraint() {}
    virtual bool Satisfy(xVerletSystem *system) = 0;
    virtual void CloneTo(xVConstraint *&dst) const = 0;
    virtual void Save( FILE *file )
    {
        fwrite(&this->Type, sizeof(xVConstraint_Type), 1, file);
    }
    static  xVConstraint *LoadType( FILE *file );
    virtual xVConstraint *Load( FILE *file ) = 0;
};

#endif
