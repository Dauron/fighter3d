#include "SkeletizedBody.h"
#include "RigidBody.h"

FaceWeights CalcPenetrationDepth(const xVector3 P_face[3], const xVector3 &P_plane, const xVector3 &N_plane)
{
    // Calculate plane (with CrossProduct)
    float W = -(P_plane.x*N_plane.x + P_plane.y*N_plane.y + P_plane.z*N_plane.z);
    float S_cur, S_max = 0.f;

    xVector3 W_face;
    FaceWeights W_res;
    W_res.P_max.zero();
    xFLOAT W_points_max = 0;

    for (int i = 0; i < 3; ++i)
    {
        // Check positions (with DotProducts+W)
        S_cur = -(P_face[i].x * N_plane.x + P_face[i].y * N_plane.y + P_face[i].z * N_plane.z + W);
        if (S_cur > 0.f)
        {
            W_face.xyz[i] = (P_face[i] - P_plane).lengthSqr();
            W_points_max += S_cur;
            W_res.P_max += S_cur * P_face[i];
            if (S_cur > S_max) S_max = S_cur;
        }
        else
            W_face.xyz[i] = 0.f;
    }
    if (W_points_max) W_res.P_max /= W_points_max;
    else              W_res.P_max = P_plane;

    xFLOAT W_scale = (W_face.x + W_face.y + W_face.z);
    if (W_face.x > 0.f) W_face.x = W_scale / W_face.x;
    if (W_face.y > 0.f) W_face.y = W_scale / W_face.y;
    if (W_face.z > 0.f) W_face.z = W_scale / W_face.z;
    W_scale = 1 / (W_face.x + W_face.y + W_face.z);
    W_face.x *= W_scale;
    W_face.y *= W_scale;
    W_face.z *= W_scale;

    W_res.S_max = S_max;
    W_res.S_vert[0] = W_face.x * S_max;
    W_res.S_vert[1] = W_face.y * S_max;
    W_res.S_vert[2] = W_face.z * S_max;
    return W_res;
}

xVector3 SkeletizedBody :: GetCollisionSpeed( const xVector3 P_face[3], const xVector3 &P_collision,
                             const xElement &elem, const xFace &face, const VerletSystem &system )
{
    xVector3 W_face;
    W_face.x = (P_face[0] - P_collision).lengthSqr();
    W_face.y = (P_face[1] - P_collision).lengthSqr();
    W_face.z = (P_face[2] - P_collision).lengthSqr();
    xFLOAT W_scale = W_face.x + W_face.y + W_face.z;
    if (W_face.x > 0.f) W_face.x = W_scale / W_face.x;
    if (W_face.y > 0.f) W_face.y = W_scale / W_face.y;
    if (W_face.z > 0.f) W_face.z = W_scale / W_face.z;
    W_scale = 1 / (W_face.x + W_face.y + W_face.z);
    W_face.x *= W_scale;
    W_face.y *= W_scale;
    W_face.z *= W_scale;

    xFLOAT *W_bone = new xFLOAT[system.I_particles];
    memset(W_bone, 0, system.I_particles * sizeof(xFLOAT));
    xDWORD stride = elem.textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);

    for (int pi = 0; pi < 3; ++pi)
    {
        xVertexSkel *vert = (xVertexSkel*) ( ((xBYTE*)elem.verticesP) + stride * face[pi] );
        for (int b=0; b<4; ++b)
        {
            int   bi = (int) floor(vert->bone[b]);
            float bw = (vert->bone[b] - bi)*10;
            if (bw < 0.01f) break;
            W_bone[bi] += bw * W_face.xyz[pi];
        }
    }

    xVector3 V_speed; V_speed.zero();

    for (int bi = 0; bi < system.I_particles; ++bi)
        if (W_bone[bi] > 0.f)
            V_speed += (W_bone[bi] / system.M_weight_Inv[bi]) * ( system.P_current[bi] - system.P_previous[bi] );

    delete[] W_bone;

    return V_speed;
}

void SkeletizedBody :: CalculateCollisions(SkeletizedObj *model, float T_delta)
{
    if (model->locked) return;

    model->collisionConstraints.clear();

    xVector3 *A_iter = model->verletSystem.A_forces;
    for (xWORD i = model->verletSystem.I_particles; i; --i, ++A_iter)
    {
        *A_iter = *A_iter * 0.99f; // damping
        A_iter->z -= 10.f;         // gravity
    }

    if (!model->CollidedModels.empty())
    {
        std::vector<Collisions>::iterator C_iter, C_end;

        xSkeleton &spine   = model->GetModelGr()->spine;
        xFLOAT   *W_bones  = new xFLOAT[spine.I_bones];
        xVector3 *A_forces = model->verletSystem.A_forces;
        std::vector<std::vector<xVector3>> A_dampings;
        A_dampings.resize(spine.I_bones);

        xFLOAT T_step_SqrInv;
        if (model->verletSystem.T_step > 0.f)
            T_step_SqrInv = 1.f / (model->verletSystem.T_step * model->verletSystem.T_step);
        else
            T_step_SqrInv = 0.f;

        for (int i = model->CollidedModels.size()-1; i >= 0; --i)
        {
            CollisionWithModel model_collision = model->CollidedModels[i];
            RigidObj *model2 = model_collision.model2;
            xMatrix  &MX_WorldToModel_2 = model2->MX_WorldToModel;

            xVector3 V_speed_2[4];
            if (model2->Type != RigidObj::Model_Verlet && !model2->locked)
                RigidBody::GetParticleSpeeds(V_speed_2, model2->verletSystem.P_current, model2->verletSystem.P_previous, T_step_SqrInv);

            C_iter = model_collision.collisions.begin();
            C_end  = model_collision.collisions.end();
            for (; C_iter != C_end; ++C_iter)
            {
                Collisions &collision = *C_iter;
                // Force of the resiliance
                xVector3 A_collision_1 = SkeletizedBody::GetCollisionSpeed(collision.face1v, collision.colPoint,
                            *collision.elem1, *collision.face1, model->verletSystem) * T_step_SqrInv;
                xVector3 A_collision_2; A_collision_2.zero();
                xVector3 A_collision = A_collision_1;
                // Force of the collision
                if (!model2->locked)
                {
                    if (model2->Type != RigidObj::Model_Verlet)
                    {
                        xVector3     P_collision_2 = MX_WorldToModel_2.preTransformP(collision.colPoint) - model2->modelInstancePh.center;
                        RigidBody::Contribution W_particle_2  = RigidBody::GetParticleContribution(P_collision_2);
                        A_collision_2  = V_speed_2[0] * W_particle_2.contrib[0] + V_speed_2[1] * W_particle_2.contrib[1] +
                                         V_speed_2[2] * W_particle_2.contrib[2] + V_speed_2[3] * W_particle_2.contrib[3];
                        A_collision -= A_collision_2;
                    }
                    else
                    if (T_step_SqrInv > 0.f)
                    {
                        A_collision_2 = SkeletizedBody::GetCollisionSpeed(collision.face2v, collision.colPoint,
                            *collision.elem2, *collision.face2, model2->verletSystem) * T_step_SqrInv;
                        A_collision -= A_collision_2;
                        //A_collision_2 = MX_WorldToModel_1.preTransformV(A_collision_2 * (model2->mass / model->mass));
                    }
                    //A_total += A_collision_2;
                }

                xFLOAT W_collision = A_collision.length() * 0.0001f;
                if (W_collision != 0.f)
                {
                    if (model2->Type == RigidObj::Model_Verlet)
                    {
                        xFLOAT T_fix = W_collision * 10.f;
                        if (T_fix > model->verletTime)
                        {
                            if (A_collision_1.length() < A_collision_2.length())
                            {
                                if (model->postHit == 0.f)
                                {
                                    model->verletTime = T_fix;
                                    model->verletTimeMaxInv = 2.f / T_fix;
                                }
                            }
                            else
                                model->postHit = 5.f;
                        }
                    }
                    else
                    {
                        xFLOAT T_fix = W_collision * 0.01f;
                        if (T_fix > model->verletTime)
                        {
                            model->verletTime = T_fix;
                            model->verletTimeMaxInv = 1.f / T_fix;
                        }
                    }
                }
                // Collision normal
                xVector3 N_collision = -xVector3::Normalize(A_collision);
                //if (N_collision.isZero())
                    N_collision = xVector3::CrossProduct( collision.face2v[1] - collision.face2v[0], collision.face2v[2] - collision.face2v[0] ).normalize();
                // Collision depth
                FaceWeights W_penetration = ::CalcPenetrationDepth(collision.face1v, collision.colPoint, N_collision);
                // Bone contribution
                xElement &elem = *collision.elem1;
                memset(W_bones, 0, spine.I_bones * sizeof(xFLOAT));
                xDWORD stride = elem.textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
                xFLOAT massSum = 0.f;
                for (int pi = 0; pi < 3; ++pi)
                {
                    xVertexSkel *vert = (xVertexSkel*) ( ((xBYTE*)elem.verticesP) + stride * (*collision.face1)[pi] );
                    for (int b=0; b<4; ++b)
                    {
                        int   bi = (int) floor(vert->bone[b]);
                        float bw = (vert->bone[b] - bi)*10;
                        if (bw < 0.01f) break;
                        massSum += model->verletSystem.M_weight_Inv[bi];
                        W_bones[bi] += bw * W_penetration.S_vert[pi] * model->verletSystem.M_weight_Inv[bi];
                    }
                }
                massSum = 1.f / massSum;
                for (int b=0; b < spine.I_bones; ++b)
                    W_bones[b] *= massSum;

                // Dampings made by the collision & penetration correction
                xVector3 NW_collision = N_collision / T_delta;
                A_iter = A_forces;
                //A_dampings.push_back(N_collision);
                for (int bi = 0; bi < model->verletSystem.I_particles; ++bi, ++A_iter)
                {
                    //xFLOAT forceN = xVector3::DotProduct(planeN, *a);
                    //if (forceN < 0.f) // damp force in the direction of collision (Fn = -Fc)
                    //    magnitude -= forceN;
                    if (W_bones[bi] != 0.f)
                    {
                        A_dampings[bi].push_back(N_collision);
                    }

                    //if (model2->Type == RigidObj::Model_Verlet)
                    //    *A_iter += 40.f * W_bones[bi] * NW_collision;
                    //else
                        *A_iter += W_bones[bi] * NW_collision;

                    VConstraintCollision constr;
                    constr.particle = bi;
                    constr.planeN = N_collision;
                    xVector3 P_plane = model->verletSystem.P_current[bi] + N_collision * W_bones[bi];
                    constr.planeD = -(P_plane.x*N_collision.x + P_plane.y*N_collision.y + P_plane.z*N_collision.z);
                    model->collisionConstraints.push_back(constr);

                    //model->verletSystem.P_current[bi] = P_plane;
                }
            }
        }

        /*
        std::vector<xVector3>::iterator NW_damp_iter, NW_damp_end = A_dampings.end();
        for (NW_damp_iter = A_dampings.begin(); NW_damp_iter != NW_damp_end; ++NW_damp_iter)
        {
            xVector3 N_damp = xVector3::Normalize(*NW_damp_iter);
            A_iter = A_forces;
            for (int bi = 0; bi < model->verletSystem.I_particles; ++bi, ++A_iter)
            {
                xFLOAT S_len = A_iter->length();
                if (S_len > 0.f)
                {
                    xFLOAT W_damp = xVector3::DotProduct(N_damp, *A_iter / S_len);
                    if (W_damp < 0.f) // damp force in the direction of collision (Fn = -Fc)
                        *A_iter -= W_damp * S_len * *NW_damp_iter;
                }
            }
        }
        A_dampings.clear();
        */

        A_iter = A_forces;
        for (int bi = 0; bi < model->verletSystem.I_particles; ++bi, ++A_iter)
        {
            /*
            xVector3 V_prev, V_curr;
            if (T_step_SqrInv > 0.f)
                V_prev = (model->verletSystem.P_current[bi] - model->verletSystem.P_previous[bi]) * T_step_SqrInv;
            else
                V_prev.zero();
            V_curr = V_prev;
            */
            std::vector<xVector3>::iterator NW_damp_iter, NW_damp_end = A_dampings[bi].end();
            for (NW_damp_iter = A_dampings[bi].begin(); NW_damp_iter != NW_damp_end; ++NW_damp_iter)
            {
                xVector3 N_damp = xVector3::Normalize(*NW_damp_iter);
                xFLOAT S_len = A_iter->length();
                if (S_len > 0.f)
                {
                    xFLOAT W_damp = xVector3::DotProduct(N_damp, *A_iter / S_len);
                    if (W_damp < 0.f) // damp force in the direction of collision (Fn = -Fc)
                        *A_iter -= W_damp * S_len * *NW_damp_iter;
                }
                /*xFLOAT S_len = V_curr.length();
                if (S_len > 0.f)
                {
                    xFLOAT W_damp = xVector3::DotProduct(N_damp, V_curr / S_len);
                    if (W_damp < 0.f) // damp force in the direction of collision (Fn = -Fc)
                        V_curr -= W_damp * S_len * *NW_damp_iter;
                }*/
            }
            //*A_iter += V_curr - V_prev;
            A_dampings[bi].clear();
        }
        A_dampings.clear();

        delete[] W_bones;
    }
}

void SkeletizedBody :: CalculateMovement(SkeletizedObj *model, float T_delta)
{
    bool collided = model->CollidedModels.size() > 0;

    model->CollidedModels.clear();
    if (model->locked)
    {
        if (model->verletQuaternions)
        {
            delete[] model->verletQuaternions;
            model->verletQuaternions = NULL;
        }
        return;
    }

    xSkeleton &spine = model->GetModelGr()->spine;
    model->verletSystem.C_constraints = spine.C_constraints;
    model->verletSystem.I_constraints = spine.I_constraints;
    model->verletSystem.C_lengthConst = spine.C_boneLength;
    model->verletSystem.spine = &spine;
    model->verletSystem.MX_ModelToWorld   = model->MX_ModelToWorld;
    model->verletSystem.MX_WorldToModel_T = model->MX_WorldToModel;
    model->verletSystem.MX_WorldToModel_T.transpose();
    model->verletSystem.T_step = T_delta;

    VerletSolver engine;
    engine.Init(& model->verletSystem);
    engine.I_passes = 10;
    engine.VerletFull();
    engine.SatisfyConstraints();

    spine.CalcQuats(model->verletSystem.P_current, 0, model->verletSystem.MX_WorldToModel_T);

    if (!model->verletQuaternions)
        model->verletQuaternions = new xVector4[spine.I_bones];
    spine.QuatsToArray(model->verletQuaternions);

    if (model->postHit != 0.f)
        model->postHit = max(0.f, model->postHit-T_delta);
    if (model->verletTime != 0.f)
        model->verletTime = max(0.f, model->verletTime-T_delta);
    if (model->verletTime != 0.f)
        model->verletWeight = 0.5f * model->verletTime * model->verletTimeMaxInv;
    else
        model->verletWeight = 0.f;

    model->MX_ModelToWorld.postTranslateT((model->verletSystem.P_current[0]-model->verletSystem.P_previous[0])*model->verletWeight);
    xMatrix::Invert(model->MX_ModelToWorld, model->MX_WorldToModel);
    spine.L_bones->QT_rotation.zeroQ();
    model->verletQuaternions[0].zeroQ();
}
