#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "RigidObj.h"
#include "../Models/lib3dx/xAction.h"
#include "../SourceFiles/ComBoard.h"
#include "../Math/Tracking/ObjectTracker.h"

struct FightingStyle
{
    std::string Name;
    std::string FileName;
};

class SkeletizedObj : public RigidObj
{
public:
    enum EControlType
    {
        Control_AI            = 0,
        Control_CaptureInput  = 1,
        Control_NetworkInput  = 2,
        Control_ComBoardInput = 3
    } ControlType;

    xActionSet                    actions;
    ComBoard                      comBoard;
    std::vector<FightingStyle>    styles;
    bool                          FL_auto_movement;
    Math::Tracking::ObjectTracker Tracker;

    xVector3     *NW_VerletVelocity;
    xVector3     *NW_VerletVelocity_new;
    xVector3     *NW_VerletVelocity_total;
    xBYTE         I_bones;

    VerletSystem  verletSystem;
    xQuaternion  *QT_verlet;
    xFLOAT        W_verlet;
    xFLOAT        T_verlet;
    xFLOAT        T_verlet_Max;
    xFLOAT        postHit;

public:

    SkeletizedObj ()
        : RigidObj()
        , ControlType(Control_AI)
        , NW_VerletVelocity(NULL)
        , NW_VerletVelocity_new(NULL)
        , I_bones(0)
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

    virtual void LoadLine(char *buffer, std::string &dir);

    virtual void ApplyDefaults();
    virtual void Initialize ();
    virtual void Finalize ();

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL);

    virtual void FrameStart();
    virtual void FrameUpdate(float T_time);

    xVector3 MergeCollisions();

    virtual xVector3 GetVelocity(const Physics::Colliders::CollisionPoint &CP_point) const;
    virtual void ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time);
    virtual void ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time,
                                   const Physics::Colliders::CollisionPoint &CP_point);
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

    virtual void LocationChanged() {
        RigidObj::LocationChanged();
        // verlets
        UpdateVerletSystem();
        verletSystem.SwapPositions();
        UpdateVerletSystem();
        memset(verletSystem.A_forces, 0, sizeof(xVector3)*verletSystem.I_particles);
        memset(verletSystem.NW_shift, 0, sizeof(xVector3)*verletSystem.I_particles);
    }

    virtual Math::Tracking::TrackedObject &GetSubObject(xBYTE ID_sub)
    {
        xBone  &bone = ModelGr->xModelP->Spine.L_bones[ID_sub];
        xMatrix MX_SubObjectToWorld;
        if (ModelGr->instance.MX_bones)
            MX_SubObjectToWorld = xMatrix::Transpose(ModelGr->instance.MX_bones[ID_sub]);
        else
            MX_SubObjectToWorld.identity();

        LastTO = Math::Tracking::TrackedObject(
            MX_SubObjectToWorld, bone.P_end, bone.S_length * 0.5f);
        return LastTO;
    }

protected:
    virtual void CreateVerletSystem();
    virtual void DestroyVerletSystem();
    virtual void UpdateVerletSystem();

private:
    Math::Tracking::TrackedObject LastTO;

public:
    static SkeletizedObj *camera_controled;
    static SkeletizedObj *network_controled;
};

#endif
