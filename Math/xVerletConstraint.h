#ifndef __incl_Math_xVerletConstraint_h
#define __incl_Math_xVerletConstraint_h

#include "xMath.h"
#include <vector>

/*

 T_     time
 S_     distance
 V_     velocity
 A_     acceleration
 M_     mass

 P_     point, position
 N_     normal vector
 NW_    vector (scaled normal)
 MX_    matrix

 W_     weight, scale, amount

 I_     integer counter

 C_     constraint

*/

struct xVerletSystem;

struct xIVConstraint {
    enum xIVConstraint_Type
    {
        Constraint_LengthEql,
        Constraint_LengthMin,
        Constraint_LengthMax,
        Constraint_Angular,
        Constraint_Plane
    } Type;

    virtual ~xIVConstraint() {}
    virtual bool Satisfy(xVerletSystem *system) = 0;
    virtual void CloneTo(xIVConstraint *&dst) const = 0;
    virtual void Save( FILE *file )
    {
        fwrite(&this->Type, sizeof(xIVConstraint_Type), 1, file);
    }
    static xIVConstraint *LoadType( FILE *file );
    virtual xIVConstraint *Load( FILE *file ) = 0;
};

struct xVConstraintLengthEql : xIVConstraint
{
    xVConstraintLengthEql() { Type = Constraint_LengthEql; }
    virtual ~xVConstraintLengthEql() {}

    xWORD  particleA, particleB;
    xFLOAT restLength, restLengthSqr;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xIVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xIVConstraint *Load( FILE *file );
};

struct xVConstraintLengthMin : xIVConstraint
{
    xVConstraintLengthMin() { Type = Constraint_LengthMin; }
    virtual ~xVConstraintLengthMin() {}

    xWORD  particleA, particleB;
    xFLOAT minLength, minLengthSqr;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xIVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xIVConstraint *Load( FILE *file );
};

struct xVConstraintLengthMax : xIVConstraint
{
    xVConstraintLengthMax() { Type = Constraint_LengthMax; }
    virtual ~xVConstraintLengthMax() {}

    xWORD  particleA, particleB;
    xFLOAT maxLength, maxLengthSqr;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xIVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xIVConstraint *Load( FILE *file );
};


struct xVConstraintAngular : xIVConstraint
{
    xVConstraintAngular() { Type = Constraint_Angular; }
    virtual ~xVConstraintAngular() {}

    xWORD    particleRootB, particleRootE;
    xWORD    particle;
    xVector4 upQuat;
    xFLOAT   minZ, maxZ;

    xFLOAT   angleMaxX, angleMaxY, angleMinX, angleMinY;

    virtual bool Satisfy(xVerletSystem *system);
    bool Test(const xVector3 &pRootB, const xVector3 &pRootE, const xVector3 &p,
        const xVector3 &up, const xVector3 &front) const;
    virtual void CloneTo(xIVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xIVConstraint *Load( FILE *file );
};


struct xVConstraintCollision : xIVConstraint
{
    xVConstraintCollision() { Type = Constraint_Plane; }
    virtual ~xVConstraintCollision() {}

    xVector3 planeN;
    xFLOAT   planeD;
    xWORD    particle;

    virtual bool Satisfy(xVerletSystem *system);
    virtual void CloneTo(xIVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xIVConstraint *Load( FILE *file );
};


typedef std::vector<xVConstraintCollision> xVConstraintCollisionVector;

struct xSkeleton;
struct xVerletSystem
{
    xFLOAT    T_step;
    xFLOAT    T_step_Old;
    xWORD     I_particles;

    xVector3 *P_current;    // current positions
    xVector3 *P_previous;   // previous positions
    xVector3 *A_forces;     // force accumulators
    xFLOAT   *M_weight_Inv; // inverted particle masses

    xVConstraintLengthEql       *C_lengthConst;
    xIVConstraint              **C_constraints;
    xWORD                        I_constraints;
    xVConstraintCollisionVector *C_collisions;

    xMatrix                      MX_ModelToWorld;
    xMatrix                      MX_WorldToModel_T;
    xSkeleton                   *spine;

    xVerletSystem()
    {
        P_current    = NULL;
        P_previous   = NULL;
        A_forces     = NULL;
        M_weight_Inv = NULL;
        MX_ModelToWorld.identity();
        MX_WorldToModel_T.identity();
    }

    void Init(xWORD numParticles)
    {
        I_particles = numParticles;

        P_current    = new xVector3[I_particles];
        P_previous   = new xVector3[I_particles];
        A_forces     = new xVector3[I_particles];
        M_weight_Inv = new xFLOAT  [I_particles];
        xFLOAT *M_iter = M_weight_Inv;
        for (xWORD i = I_particles; i; --i, ++M_iter) *M_iter = 1.f;

        I_constraints  = 0;
        C_constraints  = NULL;
        C_lengthConst  = NULL;
        C_collisions   = NULL;

        T_step           = 0;
        T_step_Old       = 0;
    }

    void Free()
    {
        I_particles = 0;
        if (P_current)    delete[] P_current;
        if (P_previous)   delete[] P_previous;
        if (A_forces)     delete[] A_forces;
        if (M_weight_Inv) delete[] M_weight_Inv;
        P_current    = NULL;
        P_previous   = NULL;
        A_forces     = NULL;
        M_weight_Inv = NULL;
        I_constraints = 0;
        C_constraints = NULL;
        C_lengthConst = NULL;
        C_collisions  = NULL;
    }

    void SwapPositions()
    {
        xVector3 *swp = P_current;
        P_current = P_previous;
        P_previous = swp;
        T_step_Old = T_step;
    }
};

class xVerletSolver
{
public:
    xVector3       A_gravity;
    xBYTE          I_passes;

    xVerletSystem *system;

    static const float GRAVITY;
    static const float FRICTION_AIR;

public:
    void Init(xVerletSystem *system)
    {
        I_passes = 1;
        A_gravity.zero();

        this->system = system;
    }

    void TimeStep()
    {
        AccumulateForces();
        Verlet();
        SatisfyConstraints();
    }

    void SatisfyConstraints();
    void AccumulateForces();
    void Verlet();
    void VerletFull();
};

#endif
