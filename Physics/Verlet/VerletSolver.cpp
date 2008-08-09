#include "VerletSolver.h"

const float VerletSolver :: FRICTION_AIR = 0.5f;
const float VerletSolver :: GRAVITY      = 10.f;

void VerletSolver :: SatisfyConstraints()
{
    for (xBYTE pi = I_passes; pi; --pi)
    {
        bool modified = false;
        if (system->C_lengthConst)
        {
            VConstraintLengthEql * iterL = system->C_lengthConst;
            VConstraintLengthEql * endL  = system->C_lengthConst + system->I_particles-1;
            for (; iterL != endL; ++iterL)
                modified |= iterL->Satisfy(system);
        }

        VConstraint **constr = system->C_constraints;
        for (xWORD i = system->I_constraints; i; --i, ++constr)
        {
            modified |= (*constr)->Satisfy(system);
        }

        if (system->C_collisions)
        {
            VConstraintCollisionVector::iterator iter = system->C_collisions->begin(),
                                                  end  = system->C_collisions->end();
            for (; iter != end; ++iter)
                modified |= iter->Satisfy(system);
        }

        if (!modified)
            break;
    }
}

void VerletSolver :: AccumulateForces()
{
    xVector3 *A_iter = system->A_forces;
    for (xWORD i = system->I_particles; i; --i, ++A_iter) *A_iter += A_gravity;
}

void VerletSolver :: Verlet()
{
    xVector3 *P_cur   = system->P_current;
    xVector3 *P_old   = system->P_previous;
    xVector3 *A_iter  = system->A_forces;
    xVector3 *NW_move = system->NW_shift;
    bool     *FL_lock = system->FL_attached;

    xFLOAT    T_step_Sqr = system->T_step*system->T_step;
    xFLOAT    F_friction_Inv = 1.f - (FRICTION_AIR * system->T_step);
    if (system->T_step_Old > 0.f)
        F_friction_Inv *= system->T_step / system->T_step_Old;

    if (A_iter)
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old, ++FL_lock, ++A_iter, ++NW_move)
        {
            *NW_move = F_friction_Inv * *NW_move + *A_iter * T_step_Sqr;
            //*NW_move = /*0.9f * (*P_cur - *P_old) + */ *A_iter * T_step_Sqr;
            *P_old = *P_cur + *NW_move;
            if (*FL_lock && NW_move->lengthSqr() > 0.001f) *FL_lock = false;
        }
    else
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old, ++FL_lock, ++NW_move)
        {
            /* *P_old = *P_cur + 0.9f * (*P_cur - *P_old); */
            //NW_move->zero();
            *NW_move = F_friction_Inv * (*P_cur - *P_old);
            *P_old = *P_cur;
            if (*FL_lock && NW_move->lengthSqr() > 0.001f) *FL_lock = false;
        }

    if (system->C_lengthConst)
    {
        VConstraintLengthEql * iterL = system->C_lengthConst;
        VConstraintLengthEql * endL  = system->C_lengthConst + system->I_particles-1;
        for (; iterL != endL; ++iterL)
        {
            bool &lA = system->FL_attached[iterL->particleA];
            bool &lB = system->FL_attached[iterL->particleA];

            if (lA && !lB)
            {
                if (system->NW_shift[iterL->particleB].lengthSqr() < 0.0001f) lB = true;
            }
            else if (lB && !lA)
            {
                if (system->NW_shift[iterL->particleA].lengthSqr() < 0.0001f) lA = true;
            }
        }
    }

    system->SwapPositions();
}

void VerletSolver :: VerletFull()
{
    xVector3 *P_cur   = system->P_current;
    xVector3 *P_old   = system->P_previous;
    xVector3 *A_iter  = system->A_forces;
    xVector3 *NW_move = system->NW_shift;
    bool     *FL_lock = system->FL_attached;

    xFLOAT    T_step_Sqr = system->T_step*system->T_step;
    xFLOAT    F_friction_Inv = 1.f - (FRICTION_AIR * system->T_step);
    if (system->T_step_Old > 0.f)
        F_friction_Inv *= system->T_step / system->T_step_Old;

    if (A_iter)
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old, ++FL_lock, ++A_iter, ++NW_move)
        {
            *NW_move = F_friction_Inv * (*P_cur - *P_old) + *A_iter * T_step_Sqr;
            *P_old = *P_cur + *NW_move;
            if (*FL_lock && NW_move->lengthSqr() > 0.001f) *FL_lock = false;
        }
    else
        for (xWORD i = system->I_particles; i; --i, ++P_cur, ++P_old, ++FL_lock, ++NW_move)
        {
            *NW_move = F_friction_Inv * (*P_cur - *P_old);
            *P_old = *P_cur + *NW_move;
            if (*FL_lock && NW_move->lengthSqr() > 0.001f) *FL_lock = false;
        }
    /*
    if (system->C_lengthConst)
    {
        VConstraintLengthEql * iterL = system->C_lengthConst;
        VConstraintLengthEql * endL  = system->C_lengthConst + system->I_particles-1;
        for (; iterL != endL; ++iterL)
        {
            bool &lA = system->FL_attached[iterL->particleA];
            bool &lB = system->FL_attached[iterL->particleA];

            if (lA && !lB)
            {
                if (system->NW_shift[iterL->particleB].lengthSqr() < 0.0001f) lB = true;
            }
            else if (lB && !lA)
            {
                if (system->NW_shift[iterL->particleA].lengthSqr() < 0.0001f) lA = true;
            }
        }
    }
    */

    system->SwapPositions();
}