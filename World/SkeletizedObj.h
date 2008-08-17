#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "RigidObj.h"
#include "../Models/lib3dx/xAction.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Multiplayer/NetworkInput.h"

class SkeletizedObj : public RigidObj
{
public:
    xVector3     *NW_VerletVelocity;
    xVector3     *NW_VerletVelocity_new;

    xVector3     *NW_VerletVelocity_total;

    xBYTE         I_bones;

public:

    SkeletizedObj ()
        : RigidObj()
        , verletQuaternions(NULL)
        , ControlType(Control_AI)
        , I_bones(0)
        , NW_VerletVelocity(NULL)
        , NW_VerletVelocity_new(NULL)
    {}

    virtual void Stop()
    {
        RigidObj::Stop();

        if (NW_VerletVelocity)
            for (int i = 0; i < I_bones; ++i)
            {
                NW_VerletVelocity[i].zero();
                NW_VerletVelocity_new[i].zero();
                NW_VerletVelocity_total[i].zero();
            }
    }

    virtual void ApplyDefaults();
    virtual void Initialize ();
    virtual void Finalize ();

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL);
    void AddAnimation(const char *fileName, xDWORD T_start = 0, xDWORD T_end = 0);

    virtual void PreUpdate(float deltaTime);

    virtual void FrameStart();
    virtual void FrameUpdate(float T_time);

    xVector3 MergeCollisions();

    virtual xVector3 GetVelocity(const Physics::Colliders::CollisionPoint &CP_point) const;
    virtual void ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time);
    virtual void ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time, const Physics::Colliders::CollisionPoint &CP_point);

    virtual void ApplyFix(const Physics::Colliders::CollisionPoint &CP_point)
    {
        for (int bi = 0; bi < CP_point.I_Bones; ++bi)
        {
            VConstraintCollision constr;
            constr.particle = CP_point.W_Bones[bi].I_bone;
            constr.planeN   = CP_point.NW_fix;
            constr.planeN.normalize();

            constr.planeD = -xVector3::DotProduct(constr.planeN,
                verletSystem.P_current[constr.particle] + CP_point.NW_fix * CP_point.W_fix );
            collisionConstraints.push_back(constr);

            //verletSystem.P_previous[bi] += CP_point.NW_fix * CP_point.W_fix;
        }
        Modify();
    }

    xActionSet actions;

    xQuaternion  *verletQuaternions;
    xFLOAT        verletWeight;
    xFLOAT        verletTime;
    xFLOAT        verletTimeMaxInv;
    xFLOAT        postHit;

    enum EControlType
    {
        Control_AI           = 0,
        Control_CaptureInput = 1,
        Control_NetworkInput = 2
    } ControlType;

    virtual void UpdateVerletSystem();

protected:
    virtual void CreateVerletSystem();
    virtual void DestroyVerletSystem();

};

#endif
