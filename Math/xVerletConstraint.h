#ifndef __incl_Math_xVerletConstraint_h
#define __incl_Math_xVerletConstraint_h

#include "xMath.h"
#include <vector>

struct xIVConstraint {
    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass) = 0;
};

struct xVConstraintLengthEql : xIVConstraint
{
    xWORD  particleA, particleB;
    xFLOAT restLength, restLengthSqr;

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass)
    {
        xVector3 &p1 = pos[particleA];
        xVector3 &p2 = pos[particleB];
        xVector3 delta  = p1-p2;
        xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        if (abs(restLengthSqr - deltaLengthSqr) < EPSILON2) return false;
        delta *= restLengthSqr/(deltaLengthSqr+restLengthSqr)-0.5f;
        //delta *= -1.f + restLength / sqrt(deltaLengthSqr);

        xFLOAT w1 = invmass[particleA];
        xFLOAT w2 = invmass[particleB];
        if (w1 == 0.f && w2 == 0.f) return false;
        w1 /= (w1+w2);
        w2 = 1.f - w1;

        p1 += w1*delta;
        p2 -= w2*delta;

        return true;
    }
};

struct xVConstraintLengthMin : xIVConstraint
{
    xWORD  particleA, particleB;
    xFLOAT minLength, minLengthSqr;

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass)
    {
        xVector3 &p1 = pos[particleA];
        xVector3 &p2 = pos[particleB];
        xVector3 delta  = p1-p2;
        xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        if (minLengthSqr - deltaLengthSqr > -EPSILON2) return false;
        delta *= minLengthSqr/(deltaLengthSqr+minLengthSqr)-0.5f;

        xFLOAT w1 = invmass[particleA];
        xFLOAT w2 = invmass[particleB];
        if (w1 == 0.f && w2 == 0.f) return false;
        w1 /= (w1+w2);
        w2 = 1.f - w1;

        p1 += w1*delta;
        p2 -= (1.f-w1)*delta;

        return true;
    }
};

struct xVConstraintLengthMax : xIVConstraint
{
    xWORD  particleA, particleB;
    xFLOAT maxLength, maxLengthSqr;

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass)
    {
        xVector3 &p1 = pos[particleA];
        xVector3 &p2 = pos[particleB];
        xVector3 delta  = p1-p2;
        xFLOAT deltaLengthSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z;
        if (deltaLengthSqr - maxLengthSqr < EPSILON2) return false;
        delta *= maxLengthSqr/(deltaLengthSqr+maxLengthSqr)-0.5f;

        xFLOAT w1 = invmass[particleA];
        xFLOAT w2 = invmass[particleB];
        if (w1 == 0.f && w2 == 0.f) return false;
        w1 /= (w1+w2);
        w2 = 1.f - w1;

        p1 += w1*delta;
        p2 -= (1.f-w1)*delta;

        invmass[particleA] = 0.f;
        invmass[particleB] = 0.f;

        return true;
    }
};

struct xVConstraintCollision : xIVConstraint
{
    xVector3 planeN;
    xFLOAT   planeD;
    xWORD    particle;

    virtual bool Satisfy(xVector3 *pos, xFLOAT *invmass)
    {
        xVector3 &p = pos[particle];
        xFLOAT dist = xVector3::DotProduct(planeN, p) + planeD;
        if (dist > -EPSILON2) return false;
        p -= planeN * dist;
        return true;
    }
};

class xVerletSolver
{
public:
    xWORD     m_numParticles;
    xWORD     m_numConstraints;
    xBYTE     m_numPasses;

    xIVConstraint *m_constraints;


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
        m_constraints = NULL;

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
            xIVConstraint *constr = m_constraints;
            for (xWORD i = m_numConstraints; i; --i, ++constr)
                constr->Satisfy(m_pos, m_weight);

            VecConstrCollision::iterator iter = collisionConstraints.begin(), end = collisionConstraints.end();
            for (; iter != end; ++iter)
                iter->Satisfy(m_pos, m_weight);

            VecConstrLength::iterator iterL = lengthConstraints.begin(), endL = lengthConstraints.end();
            for (; iterL != endL; ++iterL)
                iterL->Satisfy(m_pos, m_weight);
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
