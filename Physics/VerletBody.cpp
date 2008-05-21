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

    TriangleWeights tr;

    for (int i = 0; i < 3; ++i)
    {
        // Check positions (with DotProducts+W)
        P = triangle[i].x * planeN.x + triangle[i].y * planeN.y + triangle[i].z * planeN.z + W;
        if (P < 0.f)
        {
            tr.weight[i] = (triangle[i] - planeP).lengthSqr();
            if (P < Pmax) Pmax = P;
        }
        else
            tr.weight[i] = 0.f;
    }
    xFLOAT sumWeight = 1.f / (tr.weight[0] + tr.weight[1] + tr.weight[2]);

    Pmax = -Pmax;
    tr.weight[0] = Pmax * (1.f - tr.weight[0] * sumWeight);
    tr.weight[1] = Pmax * (1.f - tr.weight[1] * sumWeight);
    tr.weight[2] = Pmax * (1.f - tr.weight[2] * sumWeight);

    return tr;
}

void VerletBody :: CalculateCollisions(SkeletizedObj *model)
{
    if (model->locked) return;

    model->collisionConstraints.clear();

    if (!model->CollidedModels.empty())
    {
        model->verletWeight = 1.f;
        std::vector<Collisions>::iterator iter, end;

        xSkeleton &spine = model->GetModelGr()->spine;

        xFLOAT   *bones  = new xFLOAT[spine.boneC];
        xVector3 *forces = new xVector3[spine.boneC];
        memset(bones, 0, spine.boneC * sizeof(xFLOAT));
        memset(forces, 0, spine.boneC * sizeof(xVector3));

        for (int i = model->CollidedModels.size()-1; i >= 0; --i)
        {
            iter = model->CollidedModels[i].collisions.begin();
            end  = model->CollidedModels[i].collisions.end();
            for (; iter != end; ++iter)
            {
                xVector3 planeN = xVector3::CrossProduct( iter->face2v[1] - iter->face2v[0], iter->face2v[2] - iter->face2v[0] ).normalize();
                TriangleWeights points = CalcPenetrationDepth(iter->face1v, iter->colPoint, planeN);

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
                        forces[bi] += planeN * bones[bi];
                        /*
                        xVConstraintCollision constr;
                        constr.particle = bi;
                        constr.planeN = planeN;
                        xVector3 planeP = iter->colPoint + planeN * bones[bi];
                        constr.planeD = -(planeP.x*planeN.x + planeP.y*planeN.y + planeP.z*planeN.z);
                        model->collisionConstraints.push_back(constr);
                        */
                    }
            }
        }

        delete[] bones;

        if (model->verletForces) delete[] model->verletForces;
        model->verletForces = forces;
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

    if (model->verletWeight > 0.f)
    {
        xSkeleton &spine = model->GetModelGr()->spine;

        xVerletSolver engine;
        engine.Init(spine.boneC);
        engine.m_numPasses = 10;

        xIKNode  *bone = spine.boneP;
        xVector3 *pos = engine.m_pos, *posOld = engine.m_posOld;
        xMatrix  *mtx = model->modelInstanceGr.bonesM;
        for (int i = spine.boneC; i; --i, ++bone, ++pos, ++posOld, ++mtx)
            *pos = *posOld = model->mLocationMatrix.preTransformP ( mtx->postTransformP(bone->pointE) );
        engine.m_constraintsLenEql = spine.boneConstrP;
        engine.m_numConstraints = spine.constraintsC;
        engine.m_constraints = spine.constraintsP;
        engine.m_a = model->verletForces;
        engine.m_fTimeStep = 0.5f;
        engine.m_numPasses = 20;

        xMatrix test = xMatrix::Invert(model->modelInstanceGr.bonesM[8]);

        engine.Verlet();
        engine.SatisfyConstraints();
        spine.CalcQuats(engine.m_pos, 0, xMatrix::Transpose(model->mLocationMatrix).invert());

        if (!model->verletQuaternions)
            model->verletQuaternions = new xVector4[spine.boneC];
        spine.QuatsToArray(model->verletQuaternions);

        engine.Free();

        model->verletWeight -= deltaTime;
    }
    model->CollidedModels.clear();
    //model->verletWeight = 0.5f;
}
