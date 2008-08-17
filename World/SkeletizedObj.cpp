#include "SkeletizedObj.h"
#include "../Physics/SkeletizedBody.h"

void SkeletizedObj :: ApplyDefaults()
{
    RigidObj::ApplyDefaults();
    W_restitution      = 0.9f;
    W_restitution_self = 0.0f;
}

void SkeletizedObj :: Initialize ()
{
    RigidObj::Initialize();
    Type = Model_Verlet;

    verletTime = 0.f; // 1000.f;
    verletWeight = 0.f;
    verletTimeMaxInv = 0.f; // 0.002f;
    postHit = 0.f;

    I_bones =  GetModelPh()->Spine.I_bones;
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
    forceNotStatic = true;
    RigidObj::Initialize(gr_filename, ph_filename);
}

void SkeletizedObj :: Finalize ()
{
    RigidObj::Finalize();
    if (actions.L_actions.size())
    {
        std::vector<xAction>::iterator iterF = actions.L_actions.begin(), iterE = actions.L_actions.end();
        for (; iterF != iterE; ++iterF)
            g_AnimationMgr.DeleteAnimation(iterF->hAnimation);
        actions.L_actions.clear();
    }
    if (NW_VerletVelocity)
    {
        delete[] NW_VerletVelocity;       NW_VerletVelocity     = NULL;
        delete[] NW_VerletVelocity_new;   NW_VerletVelocity_new = NULL;
        delete[] NW_VerletVelocity_total; NW_VerletVelocity_total = NULL;
    }
}

void SkeletizedObj :: AddAnimation(const char *fileName, xDWORD T_start, xDWORD T_end)
{
    actions.L_actions.resize(actions.L_actions.size()+1);
    actions.L_actions.rbegin()->hAnimation = g_AnimationMgr.GetAnimation(fileName);
    actions.L_actions.rbegin()->T_start = T_start;
    actions.L_actions.rbegin()->T_end   = T_end;
}



void SkeletizedObj :: CreateVerletSystem()
{
    xModel &modelInst = *GetModelGr();
    verletSystem.Free();
    verletSystem.Init(modelInst.Spine.I_bones);
    verletSystem.C_collisions = &collisionConstraints;
    xBone    *bone    = modelInst.Spine.L_bones;
    xPoint3  *P_cur   = verletSystem.P_current,
             *P_old   = verletSystem.P_previous;
    xVector3 *A_iter  = verletSystem.A_forces;
    xQuaternion *QT_skew = verletSystem.QT_boneSkew;
    xFLOAT      *M_iter  = verletSystem.M_weight_Inv;
    bool        *FL_lock = verletSystem.FL_attached;
    xMatrix     *MX_bone = modelInstanceGr.MX_bones;
    xQuaternion *QT_bone = modelInstanceGr.QT_bones;

    if (MX_bone)
        for (int i = modelInst.Spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++A_iter, ++M_iter, ++FL_lock,
            ++MX_bone, QT_bone+=2)
        {
            *P_old   = *P_cur = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
            *QT_skew = bone->getSkew(*QT_bone);
            *M_iter  = 1 / bone->M_weight;
            *FL_lock = false;
            A_iter->zero();
        }
    else
        for (int i = modelInst.Spine.I_bones; i; --i, ++bone, ++P_cur, ++P_old, ++QT_skew, ++A_iter, ++M_iter, ++FL_lock)
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
    xModel      &modelInst = *GetModelGr();
    xBone       *bone    = modelInst.Spine.L_bones;
    xPoint3     *P_old   = verletSystem.P_current;
    xQuaternion *QT_skew = verletSystem.QT_boneSkew;
    xMatrix     *MX_bone = modelInstanceGr.MX_bones;
    xQuaternion *QT_bone = modelInstanceGr.QT_bones;
    for (int i = modelInst.Spine.I_bones; i; --i, ++bone, ++P_old, ++QT_skew, ++MX_bone, QT_bone+=2)
    {
        *P_old   = MX_LocalToWorld_Get().preTransformP( MX_bone->postTransformP(bone->P_end) );
        *QT_skew = bone->getSkew(*QT_bone);
    }
}

void SkeletizedObj :: PreUpdate(float deltaTime)
{
    SkeletizedBody::CalculateCollisions(this, deltaTime);
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
    BVHierarchy.invalidateTransformation();
    collisionConstraints.clear();
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

    xVector3 NW_fix; NW_fix.zero();

    std::vector<Physics::Colliders::CollisionPoint>::iterator
                            iter_cur = Collisions.begin(),
                            iter_end = Collisions.end();
    for (; iter_cur != iter_end; ++iter_cur)
    {
        if (iter_cur->V_response.lengthSqr() > 0.1f)
        {
            verletTime = 5.f;
            verletTimeMaxInv = 1.f / verletTime;
        }

        xVector3 NW_tmp = iter_cur->NW_fix * iter_cur->W_fix;
        NW_fix += NW_tmp;
        S_minLen = min (S_minLen, NW_tmp.lengthSqr());

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

    if (NW_fix.lengthSqr() < EPSILON) NW_fix.init(0.f, 0.f, 0.0f); // add random fix if locked
    return NW_fix;
}

void SkeletizedObj :: FrameUpdate(float T_time)
{
    //SkeletizedBody::CalculateMovement(this, T_time);
    //CollidedModels.clear();
    RigidObj::FrameUpdate(T_time);

    //////////////////////////////////////////////////////// Update Verlets

    for (int bi = 0; bi < I_bones; ++bi)
        if (!FL_stationary)
        {
            xFLOAT I_count = 0;

            if (Collisions.size())
            {
                xVector3 NW_dump; NW_dump.zero();

                std::vector<Physics::Colliders::CollisionPoint>::iterator
                            iter_cur = Collisions.begin(),
                            iter_end = Collisions.end();

                for (; iter_cur != iter_end; ++iter_cur)
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
                    NW_VerletVelocity_new[bi] += NW_dump / I_count;
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
        NW_translation = MergeCollisions();
    else
        NW_translation = verletSystem.P_current[0]-verletSystem.P_previous[0];

    UpdateMatrices();

    xSkeleton &spine = GetModelPh()->Spine;
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
    engine.I_passes = 50;
    //engine.Verlet();
    engine.SatisfyConstraints();

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

    if (!verletQuaternions)
        verletQuaternions = new xQuaternion[verletSystem.I_particles];
    spine.QuatsToArray(verletQuaternions);

    if (postHit != 0.f)
        postHit = max(0.f, postHit-T_time);
    if (verletTime != 0.f)
        verletTime = max(0.f, verletTime-T_time);
    if (verletTime != 0.f)
        verletWeight = verletTime * verletTimeMaxInv;
    else
        verletWeight = 0.f;

    MX_LocalToWorld_Set().postTranslateT(NW_translation);
    xMatrix::Invert(MX_LocalToWorld_Get(), MX_WorldToLocal);

    spine.L_bones->QT_rotation.zeroQ();
    verletQuaternions[0].zeroQ();

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
    //if (ControlType == Control_NetworkInput)
    //    bones2 = g_NetworkInput.GetTransformations();

    if (bones2)
        if (bones)
        {
            xAnimation::Combine(bones2, bones, modelInstanceGr.I_bones , bones);
            delete[] bones2;
        }
        else
            bones = bones2;

    if (false && !bones)
    {
        bones = new xQuaternion[modelInstanceGr.I_bones];
        for (int i = 0; i < modelInstanceGr.I_bones; ++i)
            bones[i].zeroQ();
    }

    if (verletQuaternions && bones && verletWeight > 0.f)
        xAnimation::Average(verletQuaternions, bones, modelInstanceGr.I_bones, 1.f-verletWeight, bones);

    if (bones)
    {
        GetModelGr()->Spine.QuatsFromArray(bones);
        delete[] bones;

        CalculateSkeleton();
        CollisionInfo_ReFill();
    }
    else
    if (verletQuaternions)
    {
        GetModelGr()->Spine.QuatsFromArray(verletQuaternions);
        CalculateSkeleton();
        CollisionInfo_ReFill();
    }
    else
        GetModelGr()->Spine.ResetQ();

    //////////////////////////////////////////////////////// Update Physical Representation

    UpdateVerletSystem();

    if (T_time > EPSILON)
    {
        xFLOAT T_time_Inv = 1.f / T_time;
        for (int i = 0; i < I_bones; ++i)
            NW_VerletVelocity_total[i] = (verletSystem.P_current[i] - verletSystem.P_previous[i]) * T_time_Inv;
    }
    else
        for (int i = 0; i < I_bones; ++i)
            NW_VerletVelocity_total[i].zero();

    for (int i = 0; i < modelInstancePh.I_elements; ++i)
        MeshData[i].MX_Bones = modelInstancePh.MX_bones;

    BVHierarchy.invalidateTransformation();
    for (int i = 0; i < modelInstancePh.I_elements; ++i)
        MeshData[i].Transform(xMatrix::Identity());
    GetModelPh()->ReFillBVH(BVHierarchy, MeshData);
}
