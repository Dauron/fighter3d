#include "SkeletizedBody.h"
#include "RigidBody.h"

xVector4 GetPenetration(const xPoint3 P_face_1[3], const xPoint3 P_face_2[3])
{
    // Get mean penetration point of face_2 through face_1
    xPlane   PN_plane; PN_plane.init(P_face_1);
    xVector4 P_penetration; P_penetration.zero();
    for (int ip = 0; ip < 3; ++ip)
    {
        xFLOAT S_cur = - PN_plane.distanceToPoint(P_face_2[ip]);
        if (S_cur > 0.f)
        {
            P_penetration.vector3 += P_face_2[ip] * S_cur;
            P_penetration.w += S_cur;
        }
    }
    if (P_penetration.w == 0.f)
        return xVector4::Create((P_face_1[0] + P_face_1[1] + P_face_1[2]) * 0.333333f, 0.f);
    P_penetration.vector3 /= P_penetration.w;
    P_penetration.w       = -PN_plane.distanceToPoint(P_penetration.vector3);
    return P_penetration;
}

void GetBoneWeights(const xPoint3  P_face_1[3], const xFace &face_1,
                    const xVector4 P_penetration, xFLOAT *W_bones,
                    const xElement &elem, const xSkeleton &spine)
{
    // Get weights of points of face_1
    xFLOAT3 W_point;
    bool    FL_onPoint = false;
    for (int ip = 0; ip < 3; ++ip)
    {
        xFLOAT S_distance = (P_face_1[ip] - P_penetration.vector3).lengthSqr();

        if (S_distance)
            W_point[ip] = S_distance;
        else
        {
            memset(W_point, 0, sizeof(W_point));
            W_point[ip] = 1.f;
            FL_onPoint = true;
            break;
        }
    }
    xFLOAT W_sum;
    if (!FL_onPoint)
    {
        W_point[0] = 1.f / W_point[0];
        W_point[1] = 1.f / W_point[1];
        W_point[2] = 1.f / W_point[2];
        W_sum = 1.f / (W_point[0] + W_point[1] + W_point[2]);
        W_point[0] *= W_sum;
        W_point[1] *= W_sum;
        W_point[2] *= W_sum;
    }

    // Get weights of bones
    W_sum = 0.f;
    xDWORD stride = elem.textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);
    for (int ip = 0; ip < 3; ++ip)
    {
        xVertexSkel *vert = (xVertexSkel*) ( ((xBYTE*)elem.verticesP) + stride * face_1[ip] );
        for (int ib=0; ib<4; ++ib)
        {
            int   bi = (int) floor(vert->bone[ib]);
            float bw = (vert->bone[ib] - bi)*10;
            if (bw < 0.01f) break;
            xFLOAT W_influence = W_point[ip] * bw / spine.L_bones[bi].M_weight;
            W_sum += W_influence;
            W_bones[bi] += W_influence;
        }
    }
    if (W_sum != 1.f)
    {
        W_sum = 1.f / W_sum;
        for (int ib=0; ib<spine.I_bones; ++ib)
            if (W_bones[ib])
                W_bones[ib] *= W_sum;
    }
}

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
    W_scale = (W_face.x + W_face.y + W_face.z);
    if (W_scale != 0) W_scale = 1 / W_scale;
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

struct CollisionPoint {
    xVector3 A_force_1;
    xVector3 A_force_2;
    xVector4 P_penetration;
    xVector3 N_collision;
    xWORD    I_counter;
};

void SkeletizedBody :: CalculateCollisions(SkeletizedObj *model, float T_delta)
{
    if (model->locked) return;

    model->collisionConstraints.clear();

    xFLOAT T_step_SqrInv;
    if (T_delta > 0.f)
        T_step_SqrInv = 1.f / (T_delta * T_delta);
    else
        T_step_SqrInv = 0.f;

    xVector3 *A_iter  = model->verletSystem.A_forces;
    bool     *FL_lock = model->verletSystem.FL_attached;
    for (xWORD i = 0; i < model->verletSystem.I_particles; ++i, ++A_iter, ++FL_lock)
        if (! *FL_lock)
        {
            //*A_iter = *A_iter * 0.9f;         // damping
            //A_iter->z -= 10.f;
            //xFLOAT S_zPrev = model->verletSystem.P_current[i].z - model->verletSystem.P_previous[i].z;
            //if (S_zPrev > 0.f) S_zPrev = 0.f;
            //A_iter->z = -10.f +                // gravity
            //    *A_iter * 0.99f;
            A_iter->init(0.f,0.f,-10.f);
        }
        else
            A_iter->zero();

    if (!model->CollidedModels.empty())
    {
        std::vector<Collisions>::iterator C_iter, C_end;

        xSkeleton &spine   = model->GetModelGr()->spine;
        xFLOAT   *W_bones  = new xFLOAT[spine.I_bones];
        xVector3 *A_forces = model->verletSystem.A_forces;
        std::vector<std::vector<xVector3>> A_dampings;
        A_dampings.resize(spine.I_bones);

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
                RigidBody::GetParticleSpeeds(V_speed_2, model2->verletSystem.P_current, model2->verletSystem.P_previous, 1.f);

            CollisionPoint *A_collisions = new CollisionPoint[spine.I_bones];
            memset(A_collisions, 0, sizeof(CollisionPoint)*spine.I_bones);

            C_iter = model_collision.collisions.begin();
            C_end  = model_collision.collisions.end();
            for (; C_iter != C_end; ++C_iter)
            {
                Collisions &collision = *C_iter;
                // Force of the resiliance
                xVector3 A_collision_1 = SkeletizedBody::GetCollisionSpeed(collision.face1v, collision.colPoint,
                            *collision.elem1, *collision.face1, model->verletSystem);
                xVector3 A_collision_2; A_collision_2.zero();
                // Force of the collision
                if (!model2->locked)
                {
                    if (model2->Type != RigidObj::Model_Verlet)
                    {
                        xVector3 P_collision_2 = MX_WorldToModel_2.preTransformP(collision.colPoint) - model2->modelInstancePh.center;
                        RigidBody::Contribution W_particle_2 = RigidBody::GetParticleContribution(P_collision_2);
                        A_collision_2  = V_speed_2[0] * W_particle_2.contrib[0] + V_speed_2[1] * W_particle_2.contrib[1] +
                                         V_speed_2[2] * W_particle_2.contrib[2] + V_speed_2[3] * W_particle_2.contrib[3];
                        A_collision_2 *= model2->mass;
                    }
                    else
                    if (T_step_SqrInv > 0.f)
                        A_collision_2 = SkeletizedBody::GetCollisionSpeed(collision.face2v, collision.colPoint,
                            *collision.elem2, *collision.face2, model2->verletSystem);
                }

                xVector3 N_collision   = xVector3::CrossProduct( collision.face2v[1] - collision.face2v[0], collision.face2v[2] - collision.face2v[0] ).normalize();
                xVector4 P_penetration = GetPenetration(collision.face2v, collision.face1v);
                memset(W_bones, 0, spine.I_bones * sizeof(xFLOAT));
                GetBoneWeights(collision.face1v, *collision.face1, P_penetration, W_bones, *collision.elem1, spine);

                for (int ib=0; ib<spine.I_bones; ++ib)
                    if (W_bones[ib])
                    {
                        CollisionPoint &cp = A_collisions[ib];
                        xFLOAT W_scale = W_bones[ib] / spine.L_bones[ib].M_weight;
                        cp.A_force_1     += A_collision_1 * W_scale;
                        cp.A_force_2     += A_collision_2 * W_scale;
                        if (P_penetration.w > cp.P_penetration.w)
                            cp.P_penetration = P_penetration;
                        cp.N_collision   += N_collision;
                        ++cp.I_counter;
                    }
            }

            // Dampings made by the collision & penetration correction
            //xVector3 NW_collision = N_collision / T_delta;
            A_iter = A_forces;
            //A_dampings.push_back(N_collision);
            for (int ib = 0; ib < model->verletSystem.I_particles; ++ib, ++A_iter)
            {
                CollisionPoint &cp = A_collisions[ib];
                if (!cp.I_counter) continue;

                xFLOAT I_counter_Inv = 1.f / cp.I_counter;
                cp.A_force_1 *= I_counter_Inv;
                cp.A_force_2 *= I_counter_Inv;
                //cp.P_penetration *= I_counter_Inv;
                xVector3 A_total = (cp.A_force_2 - cp.A_force_1)*T_step_SqrInv;
                if (cp.N_collision.lengthSqr() > 0.25f)
                    cp.N_collision.normalize();
                else
                    cp.N_collision = (-A_total).normalize();

                //xFLOAT forceN = xVector3::DotProduct(planeN, *a);
                //if (forceN < 0.f) // damp force in the direction of collision (Fn = -Fc)
                //    magnitude -= forceN;

                //A_dampings[ib].push_back(cp.N_collision);
                A_iter->zero();

                xFLOAT W_collision = xVector3::DotProduct(A_total, cp.N_collision);
                //if (W_collision > 0)
                //    *A_iter += A_total;

                W_collision = A_total.length();
                if (cp.P_penetration.w < 0.02f && W_collision < 10.f)
                    continue;

                if (W_collision < 1.f) // LOCK THE BONE
                    model->verletSystem.FL_attached[ib] = true;

                VConstraintCollision constr;
                constr.particle = ib;
                constr.planeN = cp.N_collision;
                xVector3 P_plane = model->verletSystem.P_current[ib] + cp.N_collision * cp.P_penetration.w;
                constr.planeD = -xVector3::DotProduct(P_plane, cp.N_collision);
                model->collisionConstraints.push_back(constr);

                model->verletSystem.P_current[ib] = P_plane;

                if (W_collision != 0.f)
                {
                    if (model2->Type == RigidObj::Model_Verlet)
                    {
                        xFLOAT T_fix = W_collision*100;
                        if (T_fix > model->verletTime)
                        {
                            if (cp.A_force_1.length() < cp.A_force_2.length())
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
                        xFLOAT T_fix = W_collision * 0.00001f;
                        if (T_fix > model->verletTime)
                        {
                            model->verletTime = T_fix;
                            model->verletTimeMaxInv = 1.f / T_fix;
                        }
                    }
                }
            }

            delete[] A_collisions;
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
                //xFLOAT S_len = V_curr.length();
                //if (S_len > 0.f)
                //{
                //    xFLOAT W_damp = xVector3::DotProduct(N_damp, V_curr / S_len);
                //    if (W_damp < 0.f) // damp force in the direction of collision (Fn = -Fc)
                //        V_curr -= W_damp * S_len * *NW_damp_iter;
                //}
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

    if (!collided)
    {
        bool *FL_lock = model->verletSystem.FL_attached;
        for (int bi = model->verletSystem.I_particles; bi; --bi, ++FL_lock)
            *FL_lock = false;
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
    engine.I_passes = 50;
    engine.Verlet();
    engine.SatisfyConstraints();

    spine.CalcQuats(model->verletSystem.P_current, model->verletSystem.QT_boneSkew,
        0, model->verletSystem.MX_WorldToModel_T, xVector3::Create(0.f,0.f,0.f));
    //model->UpdateVerletSystem();

    if (!model->verletQuaternions)
        model->verletQuaternions = new xQuaternion[spine.I_bones];
    spine.QuatsToArray(model->verletQuaternions);

    if (model->postHit != 0.f)
        model->postHit = max(0.f, model->postHit-T_delta);
    if (model->verletTime != 0.f)
        model->verletTime = max(0.f, model->verletTime-T_delta);
    if (model->verletTime != 0.f)
        model->verletWeight = 0.5f * model->verletTime * model->verletTimeMaxInv;
    else
        model->verletWeight = 0.f;

    if (model->verletWeight == 0)
    {
        xVector3 NW_translation; NW_translation.zero();
        xFLOAT   W_max = 0.f;
        xVector3 *P_cur = model->verletSystem.P_current;
        xVector3 *P_old = model->verletSystem.P_previous;
        for (int i = model->verletSystem.I_particles; i; --i, ++P_cur, ++P_old)
        {
            xVector3 NW_temp = *P_cur - *P_old;
            //NW_translation += NW_temp;
            xFLOAT   W_temp  = NW_temp.lengthSqr();
            if (W_temp > W_max)
            {
                W_max = W_temp;
                NW_translation = NW_temp;
            }
        }
        //NW_translation /= model->verletSystem.I_particles;
        if (W_max > EPSILON)
        {
            NW_translation.x = NW_translation.y = 0.f;
            model->MX_ModelToWorld.postTranslateT(NW_translation);
            xMatrix::Invert(model->MX_ModelToWorld, model->MX_WorldToModel);
        }
    }
    else
    {
        xVector3 NW_translation = model->verletSystem.P_current[0]-model->verletSystem.P_previous[0];
        //if (collided)
        //    model->MX_ModelToWorld.postTranslateT(NW_translation*max(model->verletWeight, 0.1f));
        //else
            model->MX_ModelToWorld.postTranslateT(NW_translation);
        xMatrix::Invert(model->MX_ModelToWorld, model->MX_WorldToModel);
    }
    spine.L_bones->QT_rotation.zeroQ();
    model->verletQuaternions[0].zeroQ();
}
