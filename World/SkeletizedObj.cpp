#include "SkeletizedObj.h"
#include "../AppFramework/System.h"
#include "../Config.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Multiplayer/NetworkInput.h"
#include "ObjectTypes.h"
#include "../Utils/Filesystem.h"
#include "../SourceFiles/InputCodes.h"

SkeletizedObj *SkeletizedObj::camera_controled  = NULL;
SkeletizedObj *SkeletizedObj::network_controled = NULL;

void SkeletizedObj :: ApplyDefaults()
{
    RigidObj::ApplyDefaults();

    FL_stationary   = false;
    FL_physical     = true;
    FL_customBVH    = false;
    FL_phantom      = false;
    W_restitution      = 0.9f;
    W_restitution_self = 0.0f;
    LifeEnergy         = 400.f;
    LifeEnergyMax      = 400.f;
    FL_auto_movement   = true;

    Tracker.Init();
    styles.clear();
    comBoard.Destroy();
    comBoard.PlayerSet = IC_CB_ComboSet0;
}

void SkeletizedObj :: Create ()
{
    RigidObj::Create();
    Type = AI::ObjectType::Human;

    FL_verlet     = false;
    FL_recovering = false;

    I_bones =  ModelGr->xModelP->Spine.I_bones;
    assert (!Joints);
    Joints = new JointInfo[I_bones];
}

void SkeletizedObj :: Create (const char *gr_filename, const char *ph_filename)
{
    RigidObj::Create(gr_filename, ph_filename);
    CreateVerletSystem();
    QT_verlet = new xQuaternion[verletSystem.I_particles];

    comBoard.Load(comBoard.FileName.c_str(), I_bones);
}

void SkeletizedObj :: Destroy ()
{
    RigidObj::Destroy();

    DestroyVerletSystem();
    delete[] QT_verlet;

    actions.Destroy();

    if (Joints) { delete[] Joints; Joints = NULL; }

    comBoard.Destroy();
}


void SkeletizedObj :: CreateVerletSystem()
{
    xModel &model = *ModelGr->xModelP;
    verletSystem.Free();
    verletSystem.Init(model.Spine.I_bones);
    verletSystem.C_collisions = &collisionConstraints;
    xBone    *bone    = model.Spine.L_bones;
    xPoint3  *P_cur   = verletSystem.P_current,
             *P_old   = verletSystem.P_previous;
    xVector3 *A_iter  = verletSystem.A_forces;
    xQuaternion *QT_skew = verletSystem.QT_boneSkew;
    xFLOAT      *M_iter  = verletSystem.M_weight_Inv;
    bool        *FL_lock = verletSystem.FL_attached;
    xMatrix     *MX_bone = ModelGr->instance.MX_bones;

    if (MX_bone)
        for (int i = model.Spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++A_iter, ++M_iter, ++FL_lock,
            ++MX_bone)
        {
            *P_old   = *P_cur = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
            QT_skew->zeroQ();
            *M_iter  = 1 / bone->M_weight;
            *FL_lock = false;
            A_iter->zero();
        }
    else
        for (int i = model.Spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++A_iter, ++M_iter, ++FL_lock)
        {
            *P_old   = *P_cur = MX_LocalToWorld_Get().preTransformP( bone->P_end );
            QT_skew->zeroQ();
            *M_iter  = 1 / bone->M_weight;
            *FL_lock = false;
            A_iter->zero();
        }
}

void SkeletizedObj :: DestroyVerletSystem()
{
    verletSystem.Free();
}

void SkeletizedObj :: UpdateSkews(xQuaternion *QT_bone)
{
    xModel      &model   = *ModelGr->xModelP;
    xBone       *bone    = model.Spine.L_bones;
    xQuaternion *QT_skew = verletSystem.QT_boneSkew;
    if (QT_bone)
        for (int i = model.Spine.I_bones; i; --i, ++bone, ++QT_skew, ++QT_bone)
            *QT_skew = *QT_bone;
    else
        for (int i = model.Spine.I_bones; i; --i, ++QT_skew)
            QT_skew->zeroQ();
}

void SkeletizedObj :: UpdateVerletSystem()
{
    xModel      &model   = *ModelGr->xModelP;
    xBone       *bone    = model.Spine.L_bones;
    xPoint3     *P_old   = verletSystem.P_current;
    xMatrix     *MX_bone = ModelGr->instance.MX_bones;
    for (int i = model.Spine.I_bones; i; --i, ++bone, ++P_old, ++MX_bone)
        *P_old   = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
}

xVector3 SkeletizedObj :: GetVelocity(const Physics::Colliders::CollisionPoint &CP_point) const
{
    if (CP_point.Figure && CP_point.Figure->Type == xIFigure3d::Mesh)
    {
        xVector3 V_velo; V_velo.zero();

        for (int i = 0; i < CP_point.I_Bones; ++i)
            V_velo += CP_point.W_Bones[i].W_bone * Joints[CP_point.W_Bones[i].I_bone].F_VerletPower * 0.1f ;//NW_VerletVelocity_total

        return V_velo + RigidObj::GetVelocity(CP_point);
    }

    return RigidObj::GetVelocity(CP_point);
}

void SkeletizedObj :: ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time)
{
    xVector3 NW_vel = NW_accel * T_time;
    JointInfo *joint = Joints;
    for (int bi = 0; bi < I_bones; ++bi, ++joint)
        joint->NW_VerletVelocity_new += NW_vel;
}

void SkeletizedObj :: ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time,
                                        const Physics::Colliders::CollisionPoint &CP_point)
{
    if (CP_point.Figure && CP_point.Figure->Type == xIFigure3d::Mesh)
    {
        xVector3 NW_vel = NW_accel * T_time;
        JointInfo *joint = Joints;
        for (int i = 0; i < CP_point.I_Bones; ++i, ++joint)
            joint->NW_VerletVelocity_new += CP_point.W_Bones[i].W_bone * NW_vel;
    }
}


void SkeletizedObj :: FrameStart()
{
    RigidObj::FrameStart();
    collisionConstraints.clear();
    memset(verletSystem.FL_attached, 0, sizeof(bool)*I_bones);
    JointInfo *joint = Joints;
    for (int bi = 0; bi < I_bones; ++bi, ++joint)
        joint->NW_VerletVelocity_new.zero();
}

xVector3 SkeletizedObj :: MergeCollisions()
{
    if (!Collisions.size()) return xVector3::Create(0,0,0);

    xVector3 NW_Max; NW_Max.init(EPSILON,EPSILON,EPSILON);
    xDWORD   I_MaxX = 0;
    xDWORD   I_MaxY = 0;
    xDWORD   I_MaxZ = 0;
    xVector3 NW_Min; NW_Min.init(-EPSILON,-EPSILON,-EPSILON);
    xDWORD   I_MinX = 0;
    xDWORD   I_MinY = 0;
    xDWORD   I_MinZ = 0;
    xFLOAT   S_minLen = xFLOAT_HUGE_POSITIVE;
    xFLOAT   S_maxLen = xFLOAT_HUGE_NEGATIVE;

    xVector3 NW_fix;   NW_fix.zero();
    xVector3 V_action; V_action.zero();

    xFLOAT   W_reaction = 0.f;
    xVector3 V_reaction[100];
    memset(V_reaction, 0, sizeof(xVector3)*I_bones);

    SkeletizedObj *Offender = NULL;

    std::vector<Physics::Colliders::CollisionPoint>::iterator
                            iter_cur, iter_end = Collisions.end();
    for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
    {
        for (int i = 0; i < iter_cur->I_Bones; ++i)
        {
            //Physics::Colliders::BoneWeight &bone = iter_cur->W_Bones[i];
            //verletSystem.P_current[bone.I_bone] += iter_cur->NW_fix;
            //verletSystem.W_boneMix[i] = max(0.5f, verletSystem.W_boneMix[i]);
        }
        if (iter_cur->Offender && iter_cur->Offender->Type == AI::ObjectType::Human)
        {
            if (Offender != iter_cur->Offender)
            {
                Offender = ((SkeletizedObj*)iter_cur->Offender);

                ComBoard::Action &action = Offender->comBoard.L_actions[Offender->comBoard.ID_action_cur];
                if (action.W_punch > 0.f && action.T_punch > 0.f && Offender->comBoard.T_progress <= action.T_punch)
                {
                    xFLOAT W_power = action.W_punch * (Offender->comBoard.T_progress - Offender->comBoard.T_enter) / action.T_punch;
                    W_reaction += W_power;
                }
            }
        }
    }
    for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
    {
        if (iter_cur->V_action.lengthSqr() > V_action.lengthSqr())
            V_action =  iter_cur->V_action;

        if (iter_cur->Offender && iter_cur->Offender->Type == AI::ObjectType::Human)
        {
            Offender = ((SkeletizedObj*)iter_cur->Offender);

            for (int cbi = 0; cbi < iter_cur->I_Bones; ++cbi)
            {
                Physics::Colliders::BoneWeight &bone = iter_cur->W_Bones[cbi];
                if (!Offender->FL_recovering)
                    V_reaction[bone.I_bone] += iter_cur->V_reaction * bone.W_bone;
                verletSystem.W_boneMix[bone.I_bone] = max(0.5f, verletSystem.W_boneMix[bone.I_bone]);
            }
        }

        xVector3 NW_tmp = iter_cur->NW_fix;// * iter_cur->W_fix;
        NW_fix += NW_tmp;
        xFLOAT S_len = NW_tmp.lengthSqr();
        S_minLen = min (S_minLen, S_len);
        S_maxLen = max (S_maxLen, S_len);

        if (NW_tmp.x > EPSILON)
        {
            if (NW_tmp.x > NW_Max.x) NW_Max.x = NW_tmp.x;
            ++I_MaxX;
        }
        else
        {
            if (NW_tmp.x < NW_Min.x) NW_Min.x = NW_tmp.x;
            ++I_MinX;
        }
        if (NW_tmp.y > EPSILON)
        {
            if (NW_tmp.y > NW_Max.y) NW_Max.y = NW_tmp.y;
            ++I_MaxY;
        }
        else
        {
            if (NW_tmp.y < NW_Min.y) NW_Min.y = NW_tmp.y;
            ++I_MinY;
        }
        if (NW_tmp.z > EPSILON)
        {
            if (NW_tmp.z > NW_Max.z) NW_Max.z = NW_tmp.z;
            ++I_MaxZ;
        }
        else
        {
            if (NW_tmp.z < NW_Min.z) NW_Min.z = NW_tmp.z;
            ++I_MinZ;
        }
    }

    /*
    if (V_reaction.lengthSqr() > 1500.f && postHit == 0.f)
    {
        T_verlet_Max = T_verlet = V_reaction.lengthSqr() / 1000.f;
        if (Offender)
            Offender->postHit = V_reaction.lengthSqr() / 1000.f;// 5.f;
    }
    */

    if (W_reaction > 0.f)
        LifeEnergy = max (LifeEnergy - W_reaction, 0.f);

    JointInfo *joint = Joints;
    for (int bi = 0; bi < I_bones; ++bi, ++joint)
    {
        xFLOAT V_len = V_reaction[bi].lengthSqr();
        if (V_len > 1.f)
        {
            if (W_reaction > 0.f)
            {
                joint->T_Verlet = max(V_len*0.1f, joint->T_Verlet);
                verletSystem.W_boneMix[bi] = 1.f;
            }
            else
                verletSystem.W_boneMix[bi] = max(0.5f, verletSystem.W_boneMix[bi]);
        }
    }

    xFLOAT I_count_Inv = 1.f / (xFLOAT)Collisions.size();
    NW_fix *= I_count_Inv;
    //if (NW_fix.lengthSqr() >= S_minLen)
    //    return NW_fix;

    if (I_MaxX > 0 || I_MinX > 0)
    {
        xFLOAT W_max = I_MaxX / ((xFLOAT) I_MaxX + I_MinX);
        NW_fix.x = NW_Max.x * W_max + NW_Min.x * (1.f - W_max);
    }
    else
        NW_fix.x = 0.f;
    if (I_MaxY > 0 || I_MinY > 0)
    {
        xFLOAT W_max = I_MaxY / ((xFLOAT) I_MaxY + I_MinY);
        NW_fix.y = NW_Max.y * W_max + NW_Min.y * (1.f - W_max);
    }
    else
        NW_fix.y = 0.f;
    if (I_MaxZ > 0 || I_MinZ > 0)
    {
        xFLOAT W_max = I_MaxZ / ((xFLOAT) I_MaxZ + I_MinZ);
        NW_fix.z = NW_Max.z * W_max + NW_Min.z * (1.f - W_max);
    }
    else
        NW_fix.z = 0.f;

    if (NW_fix.lengthSqr() < 0.000001f)// NW_fix.init(0.f, 0.f, 0.1f); // add random fix if locked
    {
        NW_fix.zero();
        S_minLen = xFLOAT_HUGE_POSITIVE;
        S_maxLen = xFLOAT_HUGE_NEGATIVE;
        std::vector<Physics::Colliders::CollisionPoint>::iterator
                            iter_cur = Collisions.begin(),
                            iter_end = Collisions.end();
        I_MaxX = 0;
        for (; iter_cur != iter_end; ++iter_cur)
        {
            xVector3 NW_tmp = iter_cur->NW_fix;

            xVector3 NW_direction = P_center - iter_cur->P_collision;

            if (xVector3::DotProduct(NW_direction, NW_tmp) > 0)
            {
                NW_fix += NW_tmp;
                ++I_MaxX;
            }
        }
        if (I_MaxX)
            NW_fix /= (xFLOAT) I_MaxX;
    }
    return NW_fix;
}

void SkeletizedObj :: Update(float T_time)
{
    Profile("Update skeletized object");

    //RigidObj::Update(T_time);

    //////////////////////////////////////////////////////// Update joints

    bool FL_locked = false;

    if (!FL_stationary && T_time > EPSILON)
    {
        verletSystem.SwapPositions();
        memcpy(verletSystem.P_current, verletSystem.P_previous, sizeof(xVector3)*I_bones);

        ///////////////////////// If collided
        if (Collisions.size())
        {
            std::vector<Physics::Colliders::CollisionPoint>::iterator
                    iter_cur, iter_end = Collisions.end();

            for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
                ApplyAcceleration(iter_cur->V_reaction, 1.f, *iter_cur);

            JointInfo *joint = Joints;
            for (int bi = 0; bi < I_bones; ++bi, ++joint)
            {
                xFLOAT I_supportPoints = 0;
                xVector3 NW_dump; NW_dump.zero();

                for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
                {
                    bool supports = false;
                    for (int cbi = 0; cbi < iter_cur->I_Bones && !supports; ++cbi)
                        supports = (iter_cur->W_Bones[cbi].I_bone == bi);

                    if (supports)
                    {
                        xVector3 N_fix = xVector3::Normalize(iter_cur->NW_fix);
                        xFLOAT W_cos   = xVector3::DotProduct(N_fix, joint->NW_VerletVelocity_new);
                        if (W_cos < 0.f)
                        {
                            NW_dump -= N_fix * W_cos;
                            ++I_supportPoints;
                        }
                    }
                }

                if (I_supportPoints)
                {
                    joint->NW_VerletVelocity = joint->NW_VerletVelocity_new + NW_dump / I_supportPoints;
                    FL_locked = verletSystem.FL_attached[bi] = true;
                }
                else
                {
                    //// Drag
                    //xFLOAT V_vel = joint->NW_VerletVelocity.length();
                    //xFLOAT W_air_drag = air_drag(S_radius, V_vel*V_vel) * T_time * verletSystem.M_weight_Inv[bi];
                    //if (V_vel > W_air_drag)
                    //    joint->NW_VerletVelocity *= 1.f - W_air_drag / V_vel;
                    joint->NW_VerletVelocity += joint->NW_VerletVelocity_new;
                }

                if (!joint->NW_VerletVelocity.isZero())
                    verletSystem.P_current[bi] += joint->NW_VerletVelocity * T_time;
            }

            xVector3 NW_translation = MergeCollisions() * 0.9f;
            if (NW_translation.lengthSqr() < 0.0001f)
                NW_translation.zero();
            // Update Matrices
            MX_LocalToWorld_Set().postTranslateT(NW_translation);
            UpdateMatrices();
        }
        ///////////////////////// If no collisions
        else
        {
            JointInfo *joint = Joints;
            for (int bi = 0; bi < I_bones; ++bi, ++joint)
            {
                // Drag
                xFLOAT V_vel = joint->NW_VerletVelocity.length();
                xFLOAT W_air_drag = air_drag(S_radius, V_vel*V_vel) * T_time * verletSystem.M_weight_Inv[bi];
                if (V_vel > W_air_drag)
                {
                    joint->NW_VerletVelocity *= 1.f - W_air_drag / V_vel;
                    joint->NW_VerletVelocity += joint->NW_VerletVelocity_new;
                }
                else
                    joint->NW_VerletVelocity = joint->NW_VerletVelocity_new;

                if (!joint->NW_VerletVelocity.isZero())
                    verletSystem.P_current[bi] += joint->NW_VerletVelocity * T_time;
            }

            xVector3 NW_translation = verletSystem.P_current[0]-verletSystem.P_previous[0];
            // Update Matrices
            MX_LocalToWorld_Set().postTranslateT(NW_translation);
            UpdateMatrices();
        }
    }

    xSkeleton &spine = ModelGr->xModelP->Spine;
    verletSystem.C_constraints = spine.C_constraints;
    verletSystem.I_constraints = spine.I_constraints;
    verletSystem.C_lengthConst = spine.C_boneLength;
    verletSystem.Spine = &spine;
    verletSystem.MX_ModelToWorld   = MX_LocalToWorld_Get();
    verletSystem.MX_WorldToModel_T = MX_WorldToLocal;
    verletSystem.MX_WorldToModel_T.transpose();
    verletSystem.T_step = T_time;

    VerletSolver engine;
    engine.Init(verletSystem);
    engine.I_passes = 5;
    //engine.Verlet();
    engine.SatisfyConstraints();

    if (T_time > EPSILON)
    {
        FL_recovering  = false;
        FL_verlet      = false;

        xFLOAT T_time_Inv = 1.f / T_time;
        JointInfo *joint = Joints;
        for (int bi = 0; bi < I_bones; ++bi, ++joint)
        {
            // Get real speed during this frame
            xVector3 V_speed = (verletSystem.P_current[bi] - verletSystem.P_previous[bi]) * T_time_Inv;
            if (V_speed.x > 0.f && joint->NW_VerletVelocity.x > 0.f)
                V_speed.x = min (V_speed.x, joint->NW_VerletVelocity.x);
            else
            if (V_speed.x < 0.f && joint->NW_VerletVelocity.x < 0.f)
                V_speed.x = max (V_speed.x, joint->NW_VerletVelocity.x);
            else
                V_speed.x = 0.f;

            if (V_speed.y > 0.f && joint->NW_VerletVelocity.y > 0.f)
                V_speed.y = min (V_speed.y, joint->NW_VerletVelocity.y);
            else
            if (V_speed.y < 0.f && joint->NW_VerletVelocity.y < 0.f)
                V_speed.y = max (V_speed.y, joint->NW_VerletVelocity.y);
            else
                V_speed.y = 0.f;

            if (V_speed.z > 0.f && joint->NW_VerletVelocity.z > 0.f)
                V_speed.z = min (V_speed.z, joint->NW_VerletVelocity.z);
            else
            if (V_speed.z < 0.f && joint->NW_VerletVelocity.z < 0.f)
                V_speed.z = max (V_speed.z, joint->NW_VerletVelocity.z);
            else
                V_speed.z = 0.f;
            joint->NW_VerletVelocity = V_speed;

            // Make progress with recovering
            if (joint->T_Verlet > T_time)
            {
                if (FL_locked) joint->T_Verlet -= T_time;
                verletSystem.W_boneMix[bi] = 1.f;
                FL_recovering = FL_verlet = true;
            }
            else
            if (joint->T_Verlet > 0.f)
            {
                joint->T_Verlet = 0.f;
                verletSystem.W_boneMix[bi] = 1.f;
                FL_recovering = FL_verlet = true;
            }
            else
            if (verletSystem.W_boneMix[bi] > T_time)
            {
                if (FL_locked) verletSystem.W_boneMix[bi] -= T_time;
                FL_verlet = true;
            }
            else
                verletSystem.W_boneMix[bi] = 0.f;

            // Debug
            //verletSystem.W_boneMix[bi] = 0.8f;
        }

        if (FL_recovering)
            comBoard.ID_action_cur = comBoard.StopAction.ID_action;
    }

    spine.CalcQuats(verletSystem.P_current, verletSystem.QT_boneSkew, 0, verletSystem.MX_WorldToModel_T);
    spine.QuatsToArray(QT_verlet);
    spine.L_bones->QT_rotation.zeroQ();
    QT_verlet[0].zeroQ();

    //////////////////////////////////////////////////////// Track enemy movement

    if (FL_auto_movement && Tracker.Mode != Math::Tracking::ObjectTracker::TRACK_NOTHING &&
        !FL_recovering && LifeEnergy > 0.f && T_time > 0.f)
    {
        xVector3 NW_aim = MX_LocalToWorld_Get().preTransformV(
                              comBoard.GetActionRotation().preTransformV(
                                  xVector3::Create(0.f, -1.1f, 0.f)));
        NW_aim.z = 0.f;
        Tracker.P_destination = P_center_Trfm + NW_aim;
        Tracker.UpdateDestination();
        xVector3 NW_dst = Tracker.P_destination - P_center_Trfm; NW_dst.z = 0.f;

        comBoard.AutoMovement(NW_aim, NW_dst, T_time);
        MX_LocalToWorld_Set().preMultiply(comBoard.MX_shift);
    }
    bool FL_auto_movement_needed = comBoard.AutoAction != ComBoard::AutoHint::HINT_NONE;

    //////////////////////////////////////////////////////// Update Animation

    xQuaternion *bones = NULL, *bones2 = NULL;

    if (LifeEnergy > 0.f)
    {
        if (actions.L_actions.size())
        {
            xDWORD delta = (xDWORD)(T_time*1000);

            actions.Update(delta);
            bones = actions.GetTransformations();

            if (actions.T_progress > 10000) actions.T_progress = 0;
        }

        if ((ControlType == Control_ComBoardInput || FL_auto_movement_needed) && comBoard.L_actions.size())
        {
            if (!FL_recovering)
            {
                comBoard.Update(T_time * 1000, ControlType == Control_ComBoardInput);
                MX_LocalToWorld_Set().preMultiply(comBoard.MX_shift);
            }
            bones2 = comBoard.GetTransformations();
        }
        else
        if (ControlType == Control_CaptureInput)
            bones2 = g_CaptureInput.GetTransformations();
        else
        if (ControlType == Control_NetworkInput)
            bones2 = g_NetworkInput.GetTransformations();
        else
        {
            //bones2 = new xQuaternion[ModelGr->instance.I_bones];
            //for (int i = 0; i < ModelGr->instance.I_bones; ++i)
            //    bones2[i].zeroQ();
        }
    }

    if (!bones && bones2) bones = bones2;
    else
    if (bones2)
    {
        xAnimation::Average(bones2, bones, ModelGr->instance.I_bones, 0.5f, bones);
        delete[] bones2;
    }

    if (bones)
        if (FL_verlet)
            xAnimation::Average(bones, QT_verlet, ModelGr->instance.I_bones, verletSystem.W_boneMix, bones);

    if (bones)
    {
        ModelGr->xModelP->Spine.QuatsFromArray(bones);
        UpdateSkews(bones);
        delete[] bones;
        CalculateSkeleton();
    }
    else
    {
        ModelGr->xModelP->Spine.QuatsFromArray(QT_verlet);
        UpdateSkews(QT_verlet);
        CalculateSkeleton();
    }

    //////////////////////////////////////////////////////// Update Physical Representation

    UpdateMatrices();
    UpdateVerletSystem();

    P_center_Trfm = MX_LocalToWorld_Get().preTransformP(P_center);

    if (T_time > EPSILON)
    {
        xFLOAT T_time_Inv = 1.f / T_time;
        for (int bi = 0; bi < I_bones; ++bi)
            Joints[bi].NW_VerletVelocity_total = (verletSystem.P_current[bi] - verletSystem.P_previous[bi]) * T_time_Inv;
    }
    else
        for (int bi = 0; bi < I_bones; ++bi)
            Joints[bi].NW_VerletVelocity_total.zero();

    for (int bi = 0; bi < I_bones; ++bi)
        if (Joints[bi].NW_VerletVelocity_total.isZero())
            Joints[bi].F_VerletPower.zero();
        else
        {
            xVector3 N_speed = xVector3::Normalize(Joints[bi].NW_VerletVelocity_total);
            xFLOAT   W_power = xVector3::DotProduct(Joints[bi].F_VerletPower, N_speed);
            Joints[bi].F_VerletPower = Joints[bi].NW_VerletVelocity_total;
            if (W_power > 0.f) Joints[bi].F_VerletPower += N_speed*W_power;

        }
}


void SkeletizedObj :: RegisterStats(StatPage &page)
{
    for (int bi = 0; bi < I_bones; ++bi)
    {
        Stat_Float3Ptr *stat_power = new Stat_Float3PtrAndLen();
        if (bi < 10)
            stat_power->Create("Power 0" + itos(bi), Joints[bi].F_VerletPower.xyz);
        else
            stat_power->Create("Power " + itos(bi), Joints[bi].F_VerletPower.xyz);
        page.Add(*stat_power);
    }

    for (int bi = 0; bi < I_bones; ++bi)
    {
        Stat_Float3Ptr *stat_power = new Stat_Float3PtrAndLen();
        if (bi < 10)
            stat_power->Create("Speed 0" + itos(bi), Joints[bi].NW_VerletVelocity_total.xyz);
        else
            stat_power->Create("Speed " + itos(bi), Joints[bi].NW_VerletVelocity_total.xyz);
        page.Add(*stat_power);
    }

    for (int bi = 0; bi < I_bones; ++bi)
    {
        Stat_BoolPtr *stat_locked = new Stat_BoolPtr();
        if (bi < 10)
            stat_locked->Create("Locked 0" + itos(bi), verletSystem.FL_attached[bi]);
        else
            stat_locked->Create("Locked " + itos(bi), verletSystem.FL_attached[bi]);
        page.Add(*stat_locked);
    }

    for (int bi = 0; bi < I_bones; ++bi)
    {
        Stat_FloatPtr *stat_float1 = new Stat_FloatPtr();
        Stat_FloatPtr *stat_float2 = new Stat_FloatPtr();
        if (bi < 10)
        {
            stat_float1->Create("W_Verlet 0" + itos(bi), verletSystem.W_boneMix[bi]);
            stat_float2->Create("T_Verlet 0" + itos(bi), Joints[bi].T_Verlet);
        }
        else
        {
            stat_float1->Create("W_Verlet " + itos(bi), verletSystem.W_boneMix[bi]);
            stat_float2->Create("T_Verlet " + itos(bi), Joints[bi].T_Verlet);
        }
        page.Add(*stat_float1);
        page.Add(*stat_float2);
    }
}

void SkeletizedObj :: LoadLine(char *buffer, std::string &dir)
{
    if (StartsWith(buffer, "animation"))
    {
        const char* params = NULL;
        std::string animFile = Filesystem::GetFullPath(dir + "/" + ReadSubstring(buffer+9, params));

        int start = 0, end = 0;
        if (params)
            sscanf(params, "%d %d", &start, &end);

        if (end <= start)
            actions.AddAnimation(animFile.c_str(), start);
        else
            actions.AddAnimation(animFile.c_str(), start, end);
        return;
    }

    if (StartsWith(buffer, "style"))
    {
        const char* params = NULL;
        FightingStyle style;
        style.Name = ReadSubstring(buffer+5, params);
        if (params)
        {
            style.FileName = Filesystem::GetFullPath(dir + "/" + ReadSubstring(params));
            styles.push_back(style);
            comBoard.FileName = style.FileName;
        }
        return;
    }
    if (StartsWith(buffer, "control"))
    {
        char name[255];
        sscanf(buffer+7, "%s", name);

        if (StartsWith(name, "camera"))
            SkeletizedObj::camera_controled = this;
        else
        if (StartsWith(name, "network"))
            SkeletizedObj::network_controled = this;
        else
        if (StartsWith(name, "comboard"))
        {
            ControlType = Control_ComBoardInput;
            FL_auto_movement = false;
        }
        else
            ControlType = Control_AI;
        return;
    }
    if (StartsWith(buffer, "enemy"))
    {
        int b;
        sscanf(buffer+5, "%d", &b);
        Tracker.Mode      = Math::Tracking::ObjectTracker::TRACK_OBJECT;
        Tracker.ID_object = b;
    }
    if (StartsWith(buffer, "life"))
    {
        xFLOAT f;
        sscanf(buffer+5, "%f", &f);
        LifeEnergy = LifeEnergyMax = f;
    }

    RigidObj::LoadLine(buffer, dir);
}
