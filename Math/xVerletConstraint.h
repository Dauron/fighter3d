#ifndef __incl_Math_xVerletConstraint_h
#define __incl_Math_xVerletConstraint_h

#include "xMath.h"
#include <vector>

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

    xFLOAT   elipseMaxX, elipseMaxY, elipseMinX, elipseMinY; // sines of angles = bounding elipse
    xCHAR    zSignMaxX, zSignMaxY, zSignMinX, zSignMinY;

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
    xWORD     particleC;

    xVector3 *positionP;     // current positions
    xVector3 *positionOldP;  // previous positions
    xVector3 *accelerationP; // force accumulators
    xFLOAT   *weightP;       // force accumulators

    xVConstraintLengthEql *constraintsLenEql;
    xIVConstraint        **constraintsP;
    xWORD                  constraintsC;
    xSkeleton             *spine;
    xMatrix                locationMatrix;
    xMatrix                locationMatrixIT;

    xVConstraintCollisionVector *collisions;

    xVerletSystem()
    {
        positionP     = NULL;
        positionOldP  = NULL;
        accelerationP = NULL;
        weightP       = NULL;
        locationMatrix.identity();
        locationMatrixIT.identity();
    }

    void Init(xWORD numParticles)
    {
        particleC = numParticles;

        positionP     = new xVector3[particleC];
        positionOldP  = new xVector3[particleC];
        accelerationP = new xVector3[particleC];
        weightP       = new xFLOAT[particleC];
        xFLOAT *weight = weightP;
        for (int i = particleC; i; --i, ++weight) *weight = 1.f;

        constraintsC       = 0;
        constraintsP       = NULL;
        constraintsLenEql  = NULL;
        collisions         = NULL;
    }

    void Free()
    {
        particleC   = 0;
        if (positionP)     delete[] positionP;
        if (positionOldP)  delete[] positionOldP;
        if (accelerationP) delete[] accelerationP;
        if (weightP)       delete[] weightP;
        positionP     = NULL;
        positionOldP  = NULL;
        accelerationP = NULL;
        weightP       = NULL;
    }

    void SwapPositions()
    {
        xVector3 *swp = positionP;
        positionP = positionOldP;
        positionOldP = swp;
    }
};

class xVerletSolver
{
public:
    xVector3       gravity;
    xFLOAT         timeStep;
    xBYTE          passesC;

    xVerletSystem *system;

public:
    void Init(xVerletSystem *system)
    {
        passesC      = 1;
        gravity.zero();
        timeStep = 0;

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
};

#endif
