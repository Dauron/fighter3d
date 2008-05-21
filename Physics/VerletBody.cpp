#include "VerletBody.h"

struct TriangleWeights
{
    xFLOAT3 weight;
};

TriangleWeights CalcPenetrationDepth(const xVector3 triangle[3], const xVector3 &planeP, const xVector3 &planeN)
{
    // Calculate plane (with CrossProduct)
    float W = -(planeP.x*planeN.x + planeP.y*planeN.y + planeP.z*planeN.z);
    float P, Pmax = 0.f;

    xVector3 tr;

    for (int i = 0; i < 3; ++i)
    {
        // Check positions (with DotProducts+W)
        P = triangle[i].x * planeN.x + triangle[i].y * planeN.y + triangle[i].z * planeN.z + W;
        if (P < 0.f)
        {
            tr.xyz[i] = (triangle[i] - planeP).lengthSqr();
            if (P < Pmax) Pmax = P;
        }
        else
            tr.xyz[i] = 0.f;
    }
    xFLOAT sumWeight = 1.f / (tr.x + tr.y + tr.z);

    tr.x = (1.f - tr.x * sumWeight);
    tr.y = (1.f - tr.y * sumWeight);
    tr.z = (1.f - tr.z * sumWeight);
    tr.normalize();
    tr *= -Pmax;

    TriangleWeights res;
    res.weight[0] = tr.x;
    res.weight[1] = tr.y;
    res.weight[2] = tr.z;
    return res;
}

xVector3 CalcCollisionSpeed( const xVector3 triangle[3], const xVector3 &collisionP,
                             const xElement &elem, const xFace &face,
                             const xVerletSystem &system )
{
    xVector3 tr;

    tr.x = (triangle[0] - collisionP).lengthSqr();
    tr.y = (triangle[1] - collisionP).lengthSqr();
    tr.z = (triangle[2] - collisionP).lengthSqr();
    xFLOAT sumWeight = 1.f / (tr.x + tr.y + tr.z);

    tr.x = (1.f - tr.x * sumWeight);
    tr.y = (1.f - tr.y * sumWeight);
    tr.z = (1.f - tr.z * sumWeight);
    tr.normalize();

    xFLOAT *bones  = new xFLOAT[system.particleC];
    memset(bones, 0, system.particleC * sizeof(xFLOAT));
    xDWORD  stride = elem.textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);

    for (int pi = 0; pi < 3; ++pi)
    {
        xVertexSkel *vert = (xVertexSkel*) ( ((xBYTE*)elem.verticesP) + stride * face[pi] );
        for (int b=0; b<4; ++b)
        {
            int   bi = (int) floor(vert->bone[b]);
            float bw = (vert->bone[b] - bi)*10;
            if (bw < 0.01f) break;
            bones[bi] += bw * tr.xyz[pi];
        }
    }

    xVector3 speed; speed.zero();

    for (int bi = 0; bi < system.particleC; ++bi)
        if (bones[bi] > 0.f)
            speed += bones[bi] * ( system.positionP[bi] - system.positionOldP[bi] );

    delete[] bones;

    return speed;
}

void VerletBody :: CalculateCollisions(SkeletizedObj *model)
{
    if (model->locked) return;

    model->collisionConstraints.clear();

    xVector3 *forces = model->verletSystem.accelerationP;
    xVector3 *a = forces;
    for (xWORD i = model->verletSystem.particleC; i; --i, ++a)
    {
        *a = *a * 0.99f; // damping
        a->z -= 10.f;   // gravity
    }

    if (!model->CollidedModels.empty())
    {
        model->verletWeight = 0.5f;
        std::vector<Collisions>::iterator iter, end;

        xSkeleton &spine = model->GetModelGr()->spine;
        xFLOAT   *bones  = new xFLOAT[spine.boneC];

        xVector3 dampPlus; dampPlus.zero();
        xVector3 dampMinus; dampMinus.zero();

        for (int i = model->CollidedModels.size()-1; i >= 0; --i)
        {
            CollisionWithModel &collision = model->CollidedModels[i];
            ModelObj           *model2    = collision.model2;
            iter = collision.collisions.begin();
            end  = collision.collisions.end();
            for (; iter != end; ++iter)
            {
                xVector3 planeN = xVector3::CrossProduct( iter->face2v[1] - iter->face2v[0], iter->face2v[2] - iter->face2v[0] ).normalize();
                TriangleWeights points = CalcPenetrationDepth(iter->face1v, iter->colPoint, planeN);
                xVector3 speed;
                if (model2->Type == ModelObj::Model_Verlet)
                    speed = CalcCollisionSpeed(iter->face2v, iter->colPoint, *iter->elem2, *iter->face2,
                        ((SkeletizedObj*)model2)->verletSystem);
                else
                {
                    xVector3 centerOfTheMassG = (xVector4::Create(model2->modelInstancePh.center, 1.f) * model2->mLocationMatrix).vector3;
                    speed = model2->transVelocity +
                        xVector3::CrossProduct(xQuaternion::angularVelocity(model2->rotatVelocity), centerOfTheMassG-iter->colPoint);
                }

                xElement &elem = *iter->elem1;
                memset(bones, 0, spine.boneC * sizeof(xFLOAT));

                xDWORD stride = elem.textured ? sizeof(xVertexTexSkel) : sizeof(xVertexSkel);

                for (int pi = 0; pi < 3; ++pi)
                {
                    xVertexSkel *vert = (xVertexSkel*) ( ((xBYTE*)elem.verticesP) + stride * (*iter->face1)[pi] );
                    for (int b=0; b<4; ++b)
                    {
                        int   bi = (int) floor(vert->bone[b]);
                        float bw = (vert->bone[b] - bi)*10;
                        if (bw < 0.01f) break;
                        bones[bi] += bw * points.weight[pi];
                    }
                }
                for (int bi = 0; bi < spine.boneC; ++bi)
                    if (bones[bi] > 0.f)
                    {
                        speed = planeN;
                        xVector3 &force = forces[bi];
                        xFLOAT forceN = xVector3::DotProduct(planeN, force);
                        if (forceN < 0.f) // damp force in the direction of collision (Fn = -Fc)
                        {
                            xVector3 damp = planeN * forceN;
                            if (damp.x > 0) dampPlus.x = max (dampPlus.x, damp.x);
                            else            dampMinus.x = min (dampMinus.x, damp.x);
                            if (damp.y > 0) dampPlus.y = max (dampPlus.y, damp.y);
                            else            dampMinus.y = min (dampMinus.y, damp.y);
                            if (damp.z > 0) dampPlus.z = max (dampPlus.z, damp.z);
                            else            dampMinus.z = min (dampMinus.z, damp.z);
                        }
                        force += speed * bones[bi];

                        xVConstraintCollision constr;
                        constr.particle = bi;
                        constr.planeN = planeN;
                        xVector3 planeP = iter->colPoint + planeN * bones[bi];
                        constr.planeD = -(planeP.x*planeN.x + planeP.y*planeN.y + planeP.z*planeN.z);
                        model->collisionConstraints.push_back(constr);
                    }
            }
        }

        a = forces;
        for (xWORD i = model->verletSystem.particleC; i; --i, ++a)
        {
            if (a->x > 0)
                if (a->x > dampPlus.x)  a->x -= dampPlus.x;
                else                    a->x = 0.f;
            else
                if (a->x < dampMinus.x) a->x -= dampMinus.x;
                else                    a->x = 0.f;

            if (a->y > 0)
                if (a->y > dampPlus.y)  a->y -= dampPlus.y;
                else                    a->y = 0.f;
            else
                if (a->y < dampMinus.y) a->y -= dampMinus.y;
                else                    a->y = 0.f;

            if (a->z > 0)
                if (a->z > dampPlus.z)  a->z -= dampPlus.z;
                else                    a->z = 0.f;
            else
                if (a->z < dampMinus.z) a->z -= dampMinus.z;
                else                    a->z = 0.f;
        }

        delete[] bones;
    }
}

void VerletBody :: CalculateMovement(SkeletizedObj *model, float deltaTime)
{
    if (model->locked)
    {
        if (model->verletQuaternions)
        {
            delete[] model->verletQuaternions;
            model->verletQuaternions = NULL;
        }
        model->CollidedModels.clear();
        return;
    }

    //if (model->verletWeight > 0.f)
    {
        xSkeleton &spine = model->GetModelGr()->spine;
        model->verletSystem.constraintsP = spine.constraintsP;
        model->verletSystem.constraintsC = spine.constraintsC;
        model->verletSystem.constraintsLenEql = spine.boneConstrP;
        model->verletSystem.spine = &spine;
        model->verletSystem.locationMatrix   = model->mLocationMatrix;
        model->verletSystem.locationMatrixIT = xMatrix::Invert(model->mLocationMatrix).transpose();

        xVerletSolver engine;
        engine.Init(& model->verletSystem);
        engine.timeStep = deltaTime;
        engine.passesC = 5;
        engine.Verlet();
        engine.SatisfyConstraints();

        spine.CalcQuats(model->verletSystem.positionP, 0, xMatrix::Transpose(model->mLocationMatrix).invert());

        if (!model->verletQuaternions)
            model->verletQuaternions = new xVector4[spine.boneC];
        spine.QuatsToArray(model->verletQuaternions);

        model->mLocationMatrix.postTranslateT(spine.boneP->quaternion.vector3);
        spine.boneP->quaternion.zeroQ();
        model->verletQuaternions[0].zeroQ();

        model->verletWeight -= deltaTime;
    }
    model->CollidedModels.clear();
    model->verletWeight = max(0.0f, model->verletWeight);
}
