#ifndef __incl_Physics_Verlet_VerletSolver_h
#define __incl_Physics_Verlet_VerletSolver_h

#include <vector>
#include "VConstraintLengthEql.h"
#include "VConstraintCollision.h"

typedef std::vector<VConstraintCollision> VConstraintCollisionVector;

struct xSkeleton;
struct VerletSystem
{
    xFLOAT    T_step;
    xFLOAT    T_step_Old;
    xWORD     I_particles;

    xVector3 *P_current;    // current positions
    xVector3 *P_previous;   // previous positions
    xVector3 *A_forces;     // force accumulators
    xFLOAT   *M_weight_Inv; // inverted particle masses

    VConstraintLengthEql        *C_lengthConst;
    VConstraint                **C_constraints;
    xWORD                        I_constraints;
    VConstraintCollisionVector  *C_collisions;

    xMatrix                      MX_ModelToWorld;
    xMatrix                      MX_WorldToModel_T;
    xSkeleton                   *spine;

    VerletSystem()
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

class VerletSolver
{
public:
    xVector3       A_gravity;
    xBYTE          I_passes;

    VerletSystem *system;

    static const float GRAVITY;
    static const float FRICTION_AIR;

public:
    void Init(VerletSystem *system)
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
