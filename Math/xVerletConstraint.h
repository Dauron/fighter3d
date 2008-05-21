#ifndef __incl_Math_xVerletConstraint_h
#define __incl_Math_xVerletConstraint_h

#include "xMath.h"
#include <vector>

struct xIVConstraint {
    enum xIVConstraint_Type
    {
        Constraint_LengthEql,
        Constraint_LengthMin,
        Constraint_LengthMax,
        Constraint_Plane
    } Type;

    virtual ~xIVConstraint() {}
    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass) = 0;
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

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass);

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

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass);

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

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass);

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

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass);

    virtual void CloneTo(xIVConstraint *&dst) const;

    virtual void Save( FILE *file );
    virtual xIVConstraint *Load( FILE *file );
};

class xVerletSolver
{
public:
    xWORD     m_numParticles;
    xWORD     m_numConstraints;
    xBYTE     m_numPasses;

    xIVConstraint **m_constraints;


    xVector3 *m_pos;    // current positions
    xVector3 *m_posOld; // previous positions
    xVector3 *m_a;      // force accumulators
    xFLOAT   *m_weight; // force accumulators
    xVector3  m_vGravity;
    xFLOAT    m_fTimeStep;

public:
    typedef std::vector<xVConstraintCollision> VecConstrCollision;
    typedef std::vector<xVConstraintLengthEql> VecConstrLength;

    VecConstrCollision collisionConstraints;
    VecConstrLength    lengthConstraints;

    xVerletSolver()
    {
        m_numParticles = 0;
        m_constraints = NULL;
        m_pos = NULL;
        m_posOld = NULL;
        m_a = NULL;
        m_weight = NULL;
    }

    void Init(xWORD particleC)
    {
        m_numParticles   = particleC;
        m_numConstraints = 0;
        m_numPasses      = 1;
        m_constraints    = NULL;

        m_pos    = new xVector3[particleC];
        m_posOld = new xVector3[particleC];
        m_a      = new xVector3[particleC];
        m_weight = new xFLOAT[particleC];

        xFLOAT *weight = m_weight;
        for (int i = particleC; i; --i, ++weight) *weight = 1.f;

        m_vGravity.zero();
        m_fTimeStep = 0;
        collisionConstraints.clear();
        lengthConstraints.clear();
    }

    void Free()
    {
        m_numParticles   = 0;
        if (m_constraints) delete[] m_constraints;
        if (m_pos) delete[] m_pos;
        if (m_posOld) delete[] m_posOld;
        if (m_a) delete[] m_a;
        if (m_weight) delete[] m_weight;

        collisionConstraints.clear();
        lengthConstraints.clear();
    }

    void TimeStep()
    {
        AccumulateForces();
        Verlet();
        SatisfyConstraints();
    }

    void SatisfyConstraints()
    {
        for (xBYTE pass_n = m_numPasses; pass_n; --pass_n)
        {
            VecConstrCollision::iterator iter = collisionConstraints.begin(), end = collisionConstraints.end();
            for (; iter != end; ++iter)
                iter->Satisfy(m_pos, m_weight);

            VecConstrLength::iterator iterL = lengthConstraints.begin(), endL = lengthConstraints.end();
            for (; iterL != endL; ++iterL)
                iterL->Satisfy(m_pos, m_weight);

            xIVConstraint **constr = m_constraints;
            for (xWORD i = m_numConstraints; i; --i, ++constr)
                (*constr)->Satisfy(m_pos, m_weight);
        }
    }

private:
    void AccumulateForces()
    {
        xVector3 *a = m_a;
        for (xWORD i = m_numParticles; i; --i, ++a) *a = m_vGravity;
    }
    void Verlet()
    {
        xVector3 *pC = m_pos;
        xVector3 *pO = m_posOld;
        xVector3 *a  = m_a;
        xFLOAT    timeStepSqr = m_fTimeStep*m_fTimeStep;

        for (xWORD i = m_numParticles; i; --i, ++pC, ++pO, ++a)
            *pO = 2 * *pC - *pO + *a * timeStepSqr;

        xVector3 *swp = m_pos;
        m_pos = m_posOld;
        m_posOld = swp;
    }
};

#endif
