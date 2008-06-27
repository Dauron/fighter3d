#include "RigidBody.h"
#include "SkeletizedBody.h"

RigidBody::Contribution RigidBody :: GetParticleContribution(const xVector3 &N_plane_2,
                                                             const xVector3 *P_current,
                                                             const xVector3 &P_collision,
                                                             const xVector3 &P_collision_Local)
{
    xVector4 W_part;
    W_part.x = xVector3::CrossProduct(N_plane_2, (P_current[1] - P_current[0])*P_collision_Local.x).lengthSqr();
    W_part.y = xVector3::CrossProduct(N_plane_2, (P_current[2] - P_current[0])*P_collision_Local.y).lengthSqr();
    W_part.z = xVector3::CrossProduct(N_plane_2, (P_current[3] - P_current[0])*P_collision_Local.z).lengthSqr();
    W_part.w = 1-xVector3::CrossProduct(N_plane_2, (P_collision - P_current[0]).normalize()).lengthSqr();
    W_part.w *= W_part.w;
    W_part.w *= W_part.w;
    W_part.w *= W_part.w;
    W_part /= W_part.x + W_part.y + W_part.z + W_part.w;

    xMatrix  &MX_WorldToX = xMatrixFromVectors(P_current[1] - P_current[0], N_plane_2).invert();
    xMatrix  &MX_WorldToY = xMatrixFromVectors(P_current[2] - P_current[0], N_plane_2).invert();
    xMatrix  &MX_WorldToZ = xMatrixFromVectors(P_current[3] - P_current[0], N_plane_2).invert();
    xVector3 &P_collision_Rel = P_collision - P_current[0];
    if (MX_WorldToX.preTransformP(P_collision_Rel).y < 0) W_part.x = -W_part.x;
    if (MX_WorldToY.preTransformP(P_collision_Rel).y < 0) W_part.y = -W_part.y;
    if (MX_WorldToZ.preTransformP(P_collision_Rel).y < 0) W_part.z = -W_part.z;

    Contribution W_res;
    W_res.contrib[0] = W_part.w; W_res.contrib[1] = W_part.x; W_res.contrib[2] = W_part.y; W_res.contrib[3] = W_part.z;
    return W_res;
}

RigidBody::Contribution RigidBody :: GetParticleContribution(const xVector3 &P_collision_Local)
{
    xVector4 W_part;

    W_part.w = P_collision_Local.lengthSqr();
    W_part.x = P_collision_Local.y*P_collision_Local.y + P_collision_Local.z*P_collision_Local.z;
    W_part.y = P_collision_Local.x*P_collision_Local.x + P_collision_Local.z*P_collision_Local.z;
    W_part.z = P_collision_Local.x*P_collision_Local.x + P_collision_Local.y*P_collision_Local.y;
    xFLOAT W_scale = W_part.x + W_part.y + W_part.z + W_part.w;
    if (W_part.x > 0.f) W_part.x = W_scale / W_part.x;
    if (W_part.y > 0.f) W_part.y = W_scale / W_part.y;
    if (W_part.z > 0.f) W_part.z = W_scale / W_part.z;
    if (W_part.w > 0.f) W_part.w = W_scale / W_part.w;
    W_part /= W_part.x + W_part.y + W_part.z + W_part.w;
    /*
    W_part.x = P_collision_Local.x;
    W_part.y = P_collision_Local.y;
    W_part.z = P_collision_Local.z;
    W_part.w = 0.f;
    W_part /= fabs(W_part.x) + fabs(W_part.y) + fabs(W_part.z) + fabs(W_part.w);
    */
    Contribution W_res;
    W_res.contrib[0] = W_part.w; W_res.contrib[1] = W_part.x; W_res.contrib[2] = W_part.y; W_res.contrib[3] = W_part.z;
    return W_res;
}


void RigidBody :: CalculateCollisions(RigidObj *model, float T_delta)
{
    if (model->locked) return;

    model->collisionConstraints.clear();

    // add the gravity contribution
    xVector3 *A_iter = model->verletSystem.A_forces;
    if (model->physical)
        for (xWORD i = model->verletSystem.I_particles; i; --i, ++A_iter)
            A_iter->z -= 10.f;

    if (!model->CollidedModels.empty())
    {
        std::vector<Collisions>::iterator C_iter, C_end;

        xMatrix  &MX_WorldToModel_1 = model->MX_WorldToModel;
        xVector3 *A_forces = model->verletSystem.A_forces;
        std::vector<xVector3> A_dampings[4];

        xFLOAT T_step_SqrInv, T_delta_SqrInv;
        if (model->verletSystem.T_step > 0.f)
            T_step_SqrInv = 1.f / (model->verletSystem.T_step * model->verletSystem.T_step);
        else
            T_step_SqrInv = 0.f;
        T_delta_SqrInv = 1.f / (T_delta*T_delta);

        xVector3 V_speed_1[4], V_speed_2[4];
        GetParticleSpeeds(V_speed_1, model->verletSystem.P_current, model->verletSystem.P_previous, T_step_SqrInv);
        xVector3 A_forces_Inner[4];
        A_forces_Inner[0] = A_forces[0];
        A_forces_Inner[1] = A_forces[1];
        A_forces_Inner[2] = A_forces[2];
        A_forces_Inner[3] = A_forces[3];
        memset(model->verletSystem.A_forces, 0, sizeof(xVector3)*model->verletSystem.I_particles);

        for (int i = model->CollidedModels.size()-1; i >= 0; --i)
        {
            CollisionWithModel model_collision = model->CollidedModels[i];
            RigidObj *model2 = model_collision.model2;
            xMatrix  &MX_WorldToModel_2 = model2->MX_WorldToModel;

            if (model2->Type != RigidObj::Model_Verlet && !model2->locked)
                GetParticleSpeeds(V_speed_2, model2->verletSystem.P_current, model2->verletSystem.P_previous, T_step_SqrInv);

            xVector3 P_collision;   P_collision.zero();
            xVector3 P_collision_1; P_collision_1.zero();
            xVector3 P_collision_2; P_collision_2.zero();
            xVector3 N_mean_1;      N_mean_1.zero();
            xVector3 N_mean_2;      N_mean_2.zero();
            xVector3 A_collision_2; A_collision_2.zero();

            C_iter = model_collision.collisions.begin();
            C_end  = model_collision.collisions.end();
            for (; C_iter != C_end; ++C_iter)
            {
                Collisions &collision = *C_iter;
                // Collision planes
                xVector3 N_plane_1 = xVector3::CrossProduct( collision.face1v[1] - collision.face1v[0], collision.face1v[2] - collision.face1v[0] ).normalize();
                xVector3 N_plane_2 = xVector3::CrossProduct( collision.face2v[1] - collision.face2v[0], collision.face2v[2] - collision.face2v[0] ).normalize();
                // Collision depth
                FaceWeights W_face_1 = ::CalcPenetrationDepth(collision.face1v, collision.colPoint, N_plane_2);
                FaceWeights W_face_2 = ::CalcPenetrationDepth(collision.face2v, collision.colPoint, N_plane_1);
                // Center of the collision in model coordinates
                P_collision   += collision.colPoint;
                P_collision_1 += W_face_1.P_max;
                P_collision_2 += W_face_2.P_max;
                N_mean_1 += N_plane_2;
                N_mean_2 += N_plane_1;

                if (!model2->locked && model2->Type == RigidObj::Model_Verlet)
                    A_collision_2 += SkeletizedBody::GetCollisionSpeed(collision.face2v, collision.colPoint,
                        *collision.elem2, *collision.face2, model2->verletSystem);
            }

            // Calculate collsion reaction based on total forces
            xFLOAT I_collCount_Inv = 1.f / model_collision.collisions.size();
            P_collision   *= I_collCount_Inv;
            P_collision_1 *= I_collCount_Inv;
            P_collision_2 *= I_collCount_Inv;
            if (!model2->locked && model2->Type == RigidObj::Model_Verlet)
                A_collision_2 *= I_collCount_Inv * T_step_SqrInv / model->mass;
            N_mean_1.normalize();
            N_mean_2.normalize();

            // Center of the collision in model coordinates
            xVector3 P_collision_1_Loc = MX_WorldToModel_1.preTransformP(P_collision_1) - model->modelInstancePh.center;
            // Force of the resiliance
            Contribution W_particle_1 = GetParticleContribution(N_mean_1, model->verletSystem.P_current,
                                                                P_collision_1, P_collision_1_Loc);
            xVector3 A_collision_1  = V_speed_1[0] +
                                     (V_speed_1[1]-V_speed_1[0]) * W_particle_1.contrib[1] +
                                     (V_speed_1[2]-V_speed_1[0]) * W_particle_1.contrib[2] +
                                     (V_speed_1[3]-V_speed_1[0]) * W_particle_1.contrib[3];
            xVector3 A_collision = A_collision_1;
            xVector3 A_total = -model->resilience * A_collision_1;
            // Force of the collision
            if (!model2->locked)
            {
                if (model2->Type != RigidObj::Model_Verlet)
                {
                    xVector3 P_collision_2_Loc = MX_WorldToModel_2.preTransformP(P_collision_2) - model2->modelInstancePh.center;
                    Contribution W_particle_2 = GetParticleContribution(N_mean_2, model2->verletSystem.P_current,
                                                                        P_collision_2, P_collision_2_Loc);
                    A_collision_2  = V_speed_2[0] +
                                    (V_speed_2[1]-V_speed_2[0]) * W_particle_2.contrib[1] +
                                    (V_speed_2[2]-V_speed_2[0]) * W_particle_2.contrib[2] +
                                    (V_speed_2[3]-V_speed_2[0]) * W_particle_2.contrib[3];
                    A_collision_2 *= (model2->mass / model->mass);
                }
                //A_collision_2 *= (model2->mass / model->mass);
                A_collision -= A_collision_2;
                A_total += A_collision_2;
            }

            xFLOAT   W_plane = -xVector3::DotProduct(N_mean_1, P_collision);
            xFLOAT   S_penetration  = xVector3::DotProduct(N_mean_1, P_collision_1) + W_plane;
            xVector3 NW_penetration = -S_penetration * N_mean_1;

            // Dampings made by the collision & penetration correction
            A_iter = A_forces;
            xFLOAT scale = 1.f / max( max(fabs(W_particle_1.contrib[0]),fabs(W_particle_1.contrib[1])),
                                      max(fabs(W_particle_1.contrib[2]),fabs(W_particle_1.contrib[3])) );
            xVector3 A_damp; A_damp.zero();
            for (int bi = 0; bi < 4; ++bi, ++A_iter)
            {
                model->verletSystem.P_current[bi] += NW_penetration;
                model->verletSystem.P_previous[bi] += NW_penetration;

                W_particle_1.contrib[bi] *= scale;
                if (bi) W_particle_1.contrib[bi] += W_particle_1.contrib[0];

                xVector3 NW_damp = N_mean_1 * Sign(W_particle_1.contrib[bi]);
                A_dampings[bi].push_back(NW_damp);

                xFLOAT W_Atotal = fabs(xVector3::DotProduct(N_mean_1, A_forces_Inner[bi]))
                    * 0.25 * min (V_speed_1[bi].length() * 0.1f, 4.f);
                *A_iter += W_particle_1.contrib[bi] * (W_Atotal * N_mean_1 + A_total);
            }
        }

        A_iter = A_forces;
        for (int bi = 0; bi < 4; ++bi, ++A_iter)
        {
            xVector3 V_prev = V_speed_1[bi];
            std::vector<xVector3>::iterator NW_damp_iter, NW_damp_end = A_dampings[bi].end();
            for (NW_damp_iter = A_dampings[bi].begin(); NW_damp_iter != NW_damp_end; ++NW_damp_iter)
            {
                xVector3 N_damp = xVector3::Normalize(*NW_damp_iter);
                xFLOAT S_len = V_speed_1[bi].length();
                if (S_len > 0.f)
                {
                    xFLOAT W_damp = xVector3::DotProduct(N_damp, V_speed_1[bi] / S_len);
                    if (W_damp < 0.f) // damp force in the direction of collision (Fn = -Fc)
                        V_speed_1[bi] -= W_damp * S_len * *NW_damp_iter;
                }
            }
            *A_iter += V_speed_1[bi] - V_prev;

            A_dampings[bi].clear();
        }
    }
}

void RigidBody :: CalculateMovement(RigidObj *model, float T_delta)
{
    model->CollidedModels.clear();
    if (model->locked) return;

    model->verletSystem.T_step = T_delta;
    VerletSolver engine;
    engine.Init(& model->verletSystem);
    engine.I_passes = 1;
    engine.VerletFull();
    engine.SatisfyConstraints();

    model->MX_ModelToWorld_prev = model->MX_ModelToWorld;
    model->MX_ModelToWorld = xMatrixFromVectors(model->verletSystem.P_current[2]-model->verletSystem.P_current[0],
        model->verletSystem.P_current[3]-model->verletSystem.P_current[0]);
    xVector3 P_center = model->MX_ModelToWorld.preTransformV(model->modelInstancePh.center);
    model->MX_ModelToWorld.postTranslateT(model->verletSystem.P_current[0]-P_center);
    xMatrix::Invert(model->MX_ModelToWorld, model->MX_WorldToModel);

    bool needsRefill = model->MX_ModelToWorld != model->MX_ModelToWorld_prev;

    memset(model->verletSystem.A_forces, 0, sizeof(xVector3)*model->verletSystem.I_particles);

    if (needsRefill)
    {
        model->modelInstanceGr.location = model->modelInstancePh.location;
        model->CollisionInfo_ReFill();
        model->InvalidateShadowRenderData();
    }
}
