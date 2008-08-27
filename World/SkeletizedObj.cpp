#include "SkeletizedObj.h"
#include "../AppFramework/System.h"
#include "../Config.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Multiplayer/NetworkInput.h"
#include "ObjectTypes.h"

void SkeletizedObj :: ApplyDefaults()
{
    RigidObj::ApplyDefaults();
    W_restitution      = 0.9f;
    W_restitution_self = 0.0f;

    Tracker.Init();
}

void SkeletizedObj :: Initialize ()
{
    RigidObj::Initialize();
    Type = AI::ObjectType::Human;

    T_verlet     = 0.f; // 1000.f;
    W_verlet     = 0.f;
    T_verlet_Max = 0.f; // 0.002f;
    postHit = 0.f;

    I_bones =  ModelGr->xModel->Spine.I_bones;
    if (NW_VerletVelocity)
    {
        delete[] NW_VerletVelocity;
        delete[] NW_VerletVelocity_new;
    }
    NW_VerletVelocity       = new xVector3[I_bones];
    NW_VerletVelocity_new   = new xVector3[I_bones];
    NW_VerletVelocity_total = new xVector3[I_bones];
    for (int i = 0; i < I_bones; ++i)
    {
        NW_VerletVelocity[i].zero();
        NW_VerletVelocity_new[i].zero();
        NW_VerletVelocity_total[i].zero();
    }
}

void SkeletizedObj :: Initialize (const char *gr_filename, const char *ph_filename)
{
    RigidObj::Initialize(gr_filename, ph_filename);
    CreateVerletSystem();
    QT_verlet = new xQuaternion[verletSystem.I_particles];

    if (ControlType == Control_ComBoardInput)
    {
        comBoard.Init();
        comBoard.Load("Data/comboard.txt");
    }
}

void SkeletizedObj :: Finalize ()
{
    RigidObj::Finalize();

    DestroyVerletSystem();
    delete[] QT_verlet;

    actions.Free();

    if (NW_VerletVelocity)
    {
        delete[] NW_VerletVelocity;       NW_VerletVelocity     = NULL;
        delete[] NW_VerletVelocity_new;   NW_VerletVelocity_new = NULL;
        delete[] NW_VerletVelocity_total; NW_VerletVelocity_total = NULL;
    }

    comBoard.Init();
}



void SkeletizedObj :: CreateVerletSystem()
{
    xModel &model = *ModelGr->xModel;
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
    xQuaternion *QT_bone = ModelGr->instance.QT_bones;

    if (MX_bone)
        for (int i = model.Spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++A_iter, ++M_iter, ++FL_lock,
            ++MX_bone, QT_bone+=2)
        {
            *P_old   = *P_cur = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
            *QT_skew = bone->getSkew(*QT_bone);
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

void SkeletizedObj :: UpdateVerletSystem()
{
    xModel      &model   = *ModelGr->xModel;
    xBone       *bone    = model.Spine.L_bones;
    xPoint3     *P_old   = verletSystem.P_current;
    xQuaternion *QT_skew = verletSystem.QT_boneSkew;
    xMatrix     *MX_bone = ModelGr->instance.MX_bones;
    xQuaternion *QT_bone = ModelGr->instance.QT_bones;
    for (int i = model.Spine.I_bones; i; --i, ++bone, ++P_old, ++QT_skew, ++MX_bone, QT_bone+=2)
    {
        *P_old   = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
        *QT_skew = bone->getSkew(*QT_bone);
    }
}

xVector3 SkeletizedObj :: GetVelocity(const Physics::Colliders::CollisionPoint &CP_point) const
{
    if (CP_point.Figure && CP_point.Figure->Type == xIFigure3d::Mesh)
    {
        xVector3 V_velo; V_velo.zero();

        for (int i = 0; i < CP_point.I_Bones; ++i)
            V_velo += CP_point.W_Bones[i].W_bone * NW_VerletVelocity_total[CP_point.W_Bones[i].I_bone];

        return V_velo + RigidObj::GetVelocity(CP_point);
    }

    return RigidObj::GetVelocity(CP_point);
}

void SkeletizedObj :: ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time)
{
    xVector3 NW_vel = NW_accel * T_time;
    for (int bi = 0; bi < I_bones; ++bi)
        NW_VerletVelocity_new[bi] += NW_vel;
}

void SkeletizedObj :: ApplyAcceleration(const xVector3 &NW_accel, xFLOAT T_time,
                                        const Physics::Colliders::CollisionPoint &CP_point)
{
    if (CP_point.Figure && CP_point.Figure->Type == xIFigure3d::Mesh)
    {
        xVector3 NW_vel = NW_accel * T_time;
        for (int i = 0; i < CP_point.I_Bones; ++i)
            NW_VerletVelocity_new[CP_point.W_Bones[i].I_bone] += CP_point.W_Bones[i].W_bone * NW_vel;
    }
}

void SkeletizedObj :: FrameStart()
{
    RigidObj::FrameStart();
    collisionConstraints.clear();
    memset(verletSystem.FL_attached, 0, sizeof(bool)*verletSystem.I_particles);
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

    xVector3 NW_fix; NW_fix.zero();
    xVector3 V_reaction; V_reaction.zero();
    xVector3 V_action;   V_action.zero();

    SkeletizedObj *Offender = NULL;

    std::vector<Physics::Colliders::CollisionPoint>::iterator
                            iter_cur = Collisions.begin(),
                            iter_end = Collisions.end();
    for (; iter_cur != iter_end; ++iter_cur)
    {
        if (iter_cur->V_action.lengthSqr() > V_action.lengthSqr())
            V_action =  iter_cur->V_action;
        V_reaction += iter_cur->V_reaction;
        if (iter_cur->Offender && iter_cur->Offender->Type == AI::ObjectType::Human)
            Offender = ((SkeletizedObj*)iter_cur->Offender);

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

    if (V_reaction.lengthSqr() > 5.f && postHit == 0.f)
    {
        T_verlet_Max = T_verlet = 5.f;
        if (Offender)
            Offender->postHit = 5.f;
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

    if (NW_fix.lengthSqr() < 0.000001f) NW_fix.init(0.f, 0.f, 0.1f); // add random fix if locked
    return NW_fix;
}

void SkeletizedObj :: FrameUpdate(float T_time)
{
    float delta = GetTick();

    //RigidObj::FrameUpdate(T_time);

    //////////////////////////////////////////////////////// Update Verlets

    for (int bi = 0; bi < I_bones; ++bi)
        if (!FL_stationary)
        {
            xFLOAT I_count = 0;

            if (Collisions.size())
            {
                xVector3 NW_dump; NW_dump.zero();

                std::vector<Physics::Colliders::CollisionPoint>::iterator
                            iter_cur, iter_end = Collisions.end();

                for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
                    ApplyAcceleration(iter_cur->V_reaction, 1.f, *iter_cur);
                for (iter_cur = Collisions.begin(); iter_cur != iter_end; ++iter_cur)
                {
                    bool supports = false;
                    for (int ci = 0; ci < iter_cur->I_Bones && !supports; ++ci)
                        supports = (iter_cur->W_Bones[ci].I_bone == bi);

                    if (supports)
                    {
                        xVector3 N_fix = xVector3::Normalize(iter_cur->NW_fix);
                        xFLOAT W_cos = xVector3::DotProduct(N_fix, NW_VerletVelocity_new[bi]);
                        if (W_cos < 0.f)
                        {
                            NW_dump -= N_fix * W_cos;
                            ++I_count;
                        }
                    }
                }

                if (I_count)
                {
                    NW_VerletVelocity_new[bi] += NW_dump / I_count;
                    verletSystem.FL_attached[bi] = true;
                }
            }
            if (!I_count)
            {
                /*
                if (T_time != 0)
                {
                    //drag
                    xFLOAT V_vel = NW_VerletVelocity[i].length();
                    xFLOAT W_air_drag = air_drag(S_radius, V_vel*V_vel) * T_time / M_mass;
                    if (V_vel > W_air_drag)
                        NW_VerletVelocity_new[i] += NW_VerletVelocity[i] * (1.f - W_air_drag / V_vel);
                }
                else*/
                    NW_VerletVelocity_new[bi] += NW_VerletVelocity[bi];
            }
            NW_VerletVelocity[bi] = NW_VerletVelocity_new[bi];

            if (!NW_VerletVelocity[bi].isZero())
            {
                if (T_time != 0)
                {
                    verletSystem.P_previous[bi] = verletSystem.P_current[bi] + NW_VerletVelocity[bi] * T_time;
                    Modify();
                }
                else
                    verletSystem.P_previous[bi] = verletSystem.P_current[bi];
                NW_VerletVelocity_new[bi].zero();
            }
            else
                verletSystem.P_previous[bi] = verletSystem.P_current[bi];
        }
    verletSystem.SwapPositions();

    xVector3 NW_translation;
    if (Collisions.size())
    {
        NW_translation = MergeCollisions() * 0.9f;
        if (W_verlet == 1.f || NW_translation.lengthSqr() < 0.0001f)
            NW_translation.zero();
    }
    else
        NW_translation = verletSystem.P_current[0]-verletSystem.P_previous[0];

    // Update Matrices
    MX_LocalToWorld_Set().postTranslateT(NW_translation);
    UpdateMatrices();

    xSkeleton &spine = ModelGr->xModel->Spine;
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

    Performance.CollisionDeterminationMS += GetTick() - delta;
    delta = GetTick();

    if (T_time > EPSILON)
    {
        xFLOAT T_time_Inv = 1.f / T_time;
        for (int i = 0; i < I_bones; ++i)
        {
            xVector3 V_speed = (verletSystem.P_current[i] - verletSystem.P_previous[i]) * T_time_Inv;
            if (V_speed.x > 0.f && NW_VerletVelocity[i].x > 0.f)
                V_speed.x = min (V_speed.x, NW_VerletVelocity[i].x);
            else
            if (V_speed.x < 0.f && NW_VerletVelocity[i].x < 0.f)
                V_speed.x = max (V_speed.x, NW_VerletVelocity[i].x);
            else
                V_speed.x = 0.f;

            if (V_speed.y > 0.f && NW_VerletVelocity[i].y > 0.f)
                V_speed.y = min (V_speed.y, NW_VerletVelocity[i].y);
            else
            if (V_speed.y < 0.f && NW_VerletVelocity[i].y < 0.f)
                V_speed.y = max (V_speed.y, NW_VerletVelocity[i].y);
            else
                V_speed.y = 0.f;

            if (V_speed.z > 0.f && NW_VerletVelocity[i].z > 0.f)
                V_speed.z = min (V_speed.z, NW_VerletVelocity[i].z);
            else
            if (V_speed.z < 0.f && NW_VerletVelocity[i].z < 0.f)
                V_speed.z = max (V_speed.z, NW_VerletVelocity[i].z);
            else
                V_speed.z = 0.f;
            NW_VerletVelocity[i] = V_speed;
        }
    }
    else
        for (int i = 0; i < I_bones; ++i)
            NW_VerletVelocity[i].zero();

    spine.CalcQuats(verletSystem.P_current, verletSystem.QT_boneSkew,
        0, verletSystem.MX_WorldToModel_T, xVector3::Create(0.f,0.f,0.f));
    spine.QuatsToArray(QT_verlet);

    if (postHit != 0.f)
        postHit = max(0.f, postHit-T_time);
    if (T_verlet != 0.f)
        T_verlet = max(0.f, T_verlet-T_time);
    W_verlet = (T_verlet <= 2.f) ? T_verlet * 0.5f : 1.f;

    spine.L_bones->QT_rotation.zeroQ();
    QT_verlet[0].zeroQ();

    //////////////////////////////////////////////////////// Track enemy movement

    if (W_verlet == 0.f && T_time > 0.f)
    {
        xPoint3 P_aim = MX_LocalToWorld_Get().preTransformP(xVector3::Create(0.f, -1.f, 1.f));
        Tracker.UpdateDestination();

        xVector3 NW_aim = P_aim - P_center_Trfm; NW_aim.z = 0.f;
        xVector3 NW_dst = Tracker.P_destination - P_center_Trfm; NW_dst.z = 0.f;

        xFLOAT      W_cos   = xVector3::DotProduct(NW_aim.normalize(), NW_dst.normalize());
        xFLOAT      W_angle = acos(W_cos);
        if ((W_angle > EPSILON3 || W_angle < -EPSILON3) &&
            (W_angle < DEGTORAD(10) && W_angle > -DEGTORAD(10)) )
        {
            //W_angle *= T_time * 0.5f;
            xFLOAT W_scale = T_time * PI * 0.25f;
            W_angle = ((W_scale > W_angle) ? W_angle : W_scale) * 0.5f;
            xFLOAT W_rot_dir = Sign(NW_aim.x*NW_dst.y - NW_aim.y*NW_dst.x);

            xQuaternion QT_rot; QT_rot.init(0.f,0.f,W_rot_dir*sin(W_angle),cos(W_angle));
            MX_LocalToWorld_Set().preMultiply(xMatrixFromQuaternion(QT_rot).transpose());
        }
    }

    //////////////////////////////////////////////////////// Update Animation

    xQuaternion *bones = NULL, *bones2 = NULL;

    if (actions.L_actions.size())
    {
        xDWORD delta = (xDWORD)(T_time*1000);

        actions.Update(delta);
        bones = actions.GetTransformations();

        if (actions.T_progress > 10000) actions.T_progress = 0;
    }


    if (ControlType == Control_CaptureInput)
        bones2 = g_CaptureInput.GetTransformations();
    else
    if (ControlType == Control_NetworkInput)
        bones2 = g_NetworkInput.GetTransformations();
    if (ControlType == Control_ComBoardInput && comBoard.L_actions.size())
    {
        comBoard.Update(T_time * 1000);
        MX_LocalToWorld_Set().preMultiply(comBoard.MX_shift);
        bones2 = comBoard.L_actions[comBoard.ID_action_cur].Anims.GetTransformations();
    }

    if (bones2)
        if (bones)
        {
            xAnimation::Combine(bones2, bones, ModelGr->instance.I_bones , bones);
            delete[] bones2;
        }
        else
            bones = bones2;

    if (false && !bones)
    {
        bones = new xQuaternion[ModelGr->instance.I_bones];
        for (int i = 0; i < ModelGr->instance.I_bones; ++i)
            bones[i].zeroQ();
    }

    if (bones && W_verlet > 0.f)
        xAnimation::Average(QT_verlet, bones, ModelGr->instance.I_bones, 1.f-W_verlet, bones);

    if (bones)
    {
        ModelGr->xModel->Spine.QuatsFromArray(bones);
        delete[] bones;
        CalculateSkeleton();
    }
    else
    {
        ModelGr->xModel->Spine.QuatsFromArray(QT_verlet);
        CalculateSkeleton();
    }
    //else
    //    GetModelGr()->Spine.ResetQ();

    //////////////////////////////////////////////////////// Update Physical Representation

    UpdateMatrices();
    UpdateVerletSystem();

    P_center_Trfm = MX_LocalToWorld_Get().preTransformP(P_center);

    if (T_time > EPSILON)
    {
        xFLOAT T_time_Inv = 1.f / T_time;
        for (int i = 0; i < I_bones; ++i)
            NW_VerletVelocity_total[i] = (verletSystem.P_current[i] - verletSystem.P_previous[i]) * T_time_Inv;
    }
    else
        for (int i = 0; i < I_bones; ++i)
            NW_VerletVelocity_total[i].zero();

    Performance.CollisionDataFillMS += GetTick() - delta;
}
