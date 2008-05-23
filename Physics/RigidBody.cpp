#include "RigidBody.h"
#include "VerletBody.h"

float RigidBody :: CalcPenetrationDepth(ModelObj *model, xVector3 &planeP, xVector3 &planeN)
{
    if (model->locked) return 0.f;

    // Calculate plane (with CrossProduct)
    float W = -(planeP.x*planeN.x + planeP.y*planeN.y + planeP.z*planeN.z);
    float Pmax = 0.f;

    xCollisionHierarchyBoundsRoot *ci = model->GetCollisionInfo();
    xWORD                          cc = model->GetCollisionInfoC();

    for (; cc; --cc, ++ci)
    {
        xVector4 *vec = ci->verticesP;
        xDWORD    vc  = ci->verticesC;
        for (; vc; --vc, ++vec)
        {
            // Check positions (with DotProducts+W)
            float P = vec->x * planeN.x + vec->y * planeN.y + vec->z * planeN.z + W;
            if (P < Pmax) Pmax = P;
        }
    }

    return -Pmax;
}


void RigidBody :: CalculateCollisions(ModelObj *model, float deltaTime)
{
    if (model->locked) return;

    model->penetrationCorrection.zero();
    model->collisionConstraints.clear();

    xVector3 *a = model->verletSystem.accelerationP;
    if (model->physical)
        for (xWORD i = model->verletSystem.particleC; i; --i, ++a)
            a->z -= 10.f;

    if (!model->CollidedModels.empty())
    {
        std::vector<xVector3> damps[4];

        std::vector<Collisions>::iterator iter, end;
        xMatrix transf = model->mLocationMatrix; transf.invert();
        xVector3 *forces = model->verletSystem.accelerationP;

        xVector3 velos1[4], velos2[4];
        if (model->verletSystem.timeStep > 0.f)
        {
            xFLOAT t2I = 1 / (model->verletSystem.timeStep * model->verletSystem.timeStep);
            velos1[0] = (model->verletSystem.positionP[0] - model->verletSystem.positionOldP[0]) * t2I;
            velos1[1] = (model->verletSystem.positionP[1] - model->verletSystem.positionOldP[1]) * t2I;
            velos1[2] = (model->verletSystem.positionP[2] - model->verletSystem.positionOldP[2]) * t2I;
            velos1[3] = (model->verletSystem.positionP[3] - model->verletSystem.positionOldP[3]) * t2I;
        }
        else
            memset (velos1, 0, sizeof(xVector3)*4);

        for (int i = model->CollidedModels.size()-1; i >= 0; --i)
        {
            CollisionWithModel &collision = model->CollidedModels[i];
            ModelObj           *model2    = collision.model2;

            if (model2->Type != ModelObj::Model_Verlet)
                if (model->verletSystem.timeStep > 0.f)
                {
                    xFLOAT t2I = 1 / (model->verletSystem.timeStep * model->verletSystem.timeStep);
                    velos2[0] = (model2->verletSystem.positionP[0] - model2->verletSystem.positionOldP[0]) * t2I;
                    velos2[1] = (model2->verletSystem.positionP[1] - model2->verletSystem.positionOldP[1]) * t2I;
                    velos2[2] = (model2->verletSystem.positionP[2] - model2->verletSystem.positionOldP[2]) * t2I;
                    velos2[3] = (model2->verletSystem.positionP[3] - model2->verletSystem.positionOldP[3]) * t2I;
                }
                else
                    memset (velos2, 0, sizeof(xVector3)*4);

            xVector3 fTotal; fTotal.zero();
            xVector3 cPoint; cPoint.zero();

            iter = collision.collisions.begin();
            end  = collision.collisions.end();
            for (; iter != end; ++iter)
            {
                xVector3 force; force.zero();
                xFLOAT scale = 0, len[4];
                for (int bi = 0; bi < model->verletSystem.particleC; ++bi)
                    scale += len[bi] = (iter->colPoint - model->verletSystem.positionP[bi]).length();
                scale = 1 / scale;
                for (int bi = 0; bi < model->verletSystem.particleC; ++bi)
                    force += velos1[bi] * len[bi] * scale;
                force = transf.preTransformV(force);
                fTotal -= force * model->resilience;
                if (!model2->locked)
                {
                    force.zero();
                    if (model2->Type != ModelObj::Model_Verlet)
                    {
                        scale = 0;
                        for (int bi = 0; bi < model2->verletSystem.particleC; ++bi)
                            scale += len[bi] = (iter->colPoint - model2->verletSystem.positionP[bi]).length();
                        scale = 1 / scale;
                        for (int bi = 0; bi < model2->verletSystem.particleC; ++bi)
                            force += velos2[bi] * len[bi] * scale;
                        force = transf.preTransformV(force * (model2->mass / model->mass));
                    }
                    else
                    {
                        force = VerletBody::CalcCollisionSpeed(iter->face2v, iter->colPoint, *iter->elem2, *iter->face2,
                            model2->verletSystem);
                        force = transf.preTransformV(force * (model2->mass / model->mass));
                    }
                    fTotal += force;
                }
                cPoint += transf.preTransformP(iter->colPoint);

                xVector3 planeN = xVector3::CrossProduct( iter->face2v[1] - iter->face2v[0], iter->face2v[2] - iter->face2v[0] ).normalize();
                xFLOAT   Pmax   = ::CalcPenetrationDepth(iter->face1v, iter->colPoint, planeN).Pmax;

                a = forces;
                for (int bi = 0; bi < model->verletSystem.particleC; ++bi, ++a)
                {
                    xFLOAT   veloN = xVector3::DotProduct(planeN, velos1[bi]);
                    damps[bi].push_back(planeN);

                    xFLOAT magnitude = 0.f;//Pmax / deltaTime;
                    xFLOAT lenA = a->length();
                    if (lenA > 0.f)
                    {
                        xFLOAT forceN = xVector3::DotProduct(planeN, *a / lenA);
                        if (forceN < 0.f) // damp force in the direction of collision (Fn = -Fc)
                            magnitude -= forceN * lenA;
                    }
                    *a += magnitude * planeN;
                }
            }

            xFLOAT radius = model->modelInstancePh.elementInstanceP[0].bsRadius * 0.5f; // TODO: use real radius
            xFLOAT collCI = 1.f / collision.collisions.size();
            fTotal *= collCI;
            cPoint *= collCI;
            cPoint -= model->modelInstancePh.center;
            cPoint /= radius;
            xVector3 s = xVector3::Create(fTotal.x * (sqrt(max(1-cPoint.y*cPoint.y-cPoint.z*cPoint.z, 0.f))),
                                          fTotal.y * (sqrt(max(1-cPoint.x*cPoint.x-cPoint.z*cPoint.z, 0.f))),
                                          fTotal.z * (sqrt(max(1-cPoint.x*cPoint.x-cPoint.y*cPoint.y, 0.f))));
            forces[0] += s;
            forces[1] += s + xVector3::Create(0, fTotal.y * cPoint.x, fTotal.z * cPoint.x);
            forces[2] += s + xVector3::Create(fTotal.x * cPoint.y, 0, fTotal.z * cPoint.y);
            forces[3] += s + xVector3::Create(fTotal.x * cPoint.z, fTotal.y * cPoint.z, 0);
        }

        a = forces;
        for (int bi = 0; bi < model->verletSystem.particleC; ++bi, ++a)
        {
            xVector3 prevVelocity = velos1[bi];
            std::vector<xVector3>::iterator damp, last = damps[bi].end();
            for (damp = damps[bi].begin(); damp != last; ++damp)
            {
                xFLOAT len = a->length();
                if (len > 0.f)
                {
                    xFLOAT forceN = xVector3::DotProduct(*damp, *a / len);
                    if (forceN < 0.f) // damp force in the direction of collision (Fn = -Fc)
                        *a -= forceN * len * *damp;
                }
                len = velos1[bi].length();
                if (len > 0.f)
                {
                    xFLOAT forceN = xVector3::DotProduct(*damp, velos1[bi] / len);
                    if (forceN < 0.f) // damp force in the direction of collision (Fn = -Fc)
                        velos1[bi] -= forceN * len * *damp;
                }
            }
            *a += velos1[bi] - prevVelocity;

            damps[bi].clear();
        }
    }
}

void RigidBody :: CalculateMovement(ModelObj *model, float deltaTime)
{
    model->CollidedModels.clear();
    if (model->locked) return;

    model->verletSystem.timeStep = deltaTime;
    xVerletSolver engine;
    engine.Init(& model->verletSystem);
    engine.passesC = 5;
    engine.VerletFull();
    engine.SatisfyConstraints();

    model->mLocationMatrixPrev  = model->mLocationMatrix;
    model->mLocationMatrix = xMatrixFromVectors(model->verletSystem.positionP[2]-model->verletSystem.positionP[0],
        model->verletSystem.positionP[3]-model->verletSystem.positionP[0], -model->verletSystem.positionP[0]);

    bool needsRefill = model->mLocationMatrix != model->mLocationMatrixPrev;

    xVector3 *a = model->verletSystem.accelerationP;
    for (xWORD i = model->verletSystem.particleC; i; --i, ++a)
        a->zero();

    if (needsRefill)
    {
        model->modelInstanceGr.location = model->modelInstancePh.location;
        model->CollisionInfo_ReFill();
        model->InvalidateShadowRenderData();
    }
}
