#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "RigidObj.h"
#include "../Models/lib3dx/xAction.h"
#include "../SourceFiles/ComBoard.h"
#include "../Math/Tracking/ObjectTracker.h"
#include "../Utils/Stat.h"

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

    struct JointInfo {
        std::vector<xVector3> supportPoints;

        xVector3 NW_VerletVelocity;
        xVector3 NW_VerletVelocity_new;
        xVector3 NW_VerletVelocity_total;
        xVector3 F_VerletPower;
        xFLOAT   T_Verlet;

        JointInfo()
        {
            NW_VerletVelocity.zero();
            NW_VerletVelocity_new.zero();
            NW_VerletVelocity_total.zero();
            F_VerletPower.zero();
            T_Verlet = 0.f;
        }
    } *Joints;

    xBYTE         I_bones;
    bool          FL_recovering;
    bool          FL_verlet;

    VerletSystem  verletSystem;
    xQuaternion  *QT_verlet;

    xFLOAT        LifeEnergy;
    xFLOAT        LifeEnergyMax;

public:
    void RegisterStats(StatPage &page);

    SkeletizedObj ()
        : RigidObj()
        , ControlType(Control_AI)
        , Joints(NULL)
        , I_bones(0)
    {}

    virtual void Stop()
    {
        RigidObj::Stop();

        if (Joints)
        {
            JointInfo *joint = Joints;
            for (int bi = 0; bi < I_bones; ++bi, ++joint)
            {
                joint->NW_VerletVelocity.zero();
                joint->NW_VerletVelocity_new.zero();
                joint->NW_VerletVelocity_total.zero();
                joint->F_VerletPower.zero();
            }
        }
    }

    virtual void LoadLine(char *buffer, std::string &dir);

    virtual void ApplyDefaults();
    virtual void Create  ();
    virtual void Destroy ();

    virtual void Create (const char *gr_filename, const char *ph_filename = NULL);

    virtual void FrameStart();
    virtual void Update(float T_time);

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
    void CreateVerletSystem();
    void DestroyVerletSystem();
    void UpdateVerletSystem();
    void UpdateSkews(xQuaternion *QT_bone);

private:
    Math::Tracking::TrackedObject LastTO;

public:
    static SkeletizedObj *camera_controled;
    static SkeletizedObj *network_controled;
};

#endif
