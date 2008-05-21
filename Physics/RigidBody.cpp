#include "RigidBody.h"

float RigidBody :: CalcPenetrationDepth(ModelObj *model, xVector3 &planeP, xVector3 &planeN)
{
    // Calculate plane (with CrossProduct)
    float W = -(planeP.x*planeN.x + planeP.y*planeN.y + planeP.z*planeN.z);
    float Pmax = 0.0f;

    CollisionInfo  *ci = model->GetCollisionInfo();
    xWORD           cc = model->GetCollisionInfoC();

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

void RigidBody :: CalculateCollisions(ModelObj *model)
{
    model->penetrationCorrection.zero();
    if (!model->CollidedModels.empty())
    {
        std::vector<Collisions>::iterator iter, end;

        model->collisionNorm.zero();
        model->collisionVelo.zero();
        model->collisionCent.zero();
        float    normScale = 0;
        float    massScale = 0;
        for (int i = model->CollidedModels.size()-1; i >= 0; --i)
        {
            //xVector3 normM(0.f, 0.f, 0.f);
            float    normMSc = 0;
            xVector3 centM; centM.zero();
            xWORD    numCM = model->CollidedModels[i].collisions.size();

            iter = model->CollidedModels[i].collisions.begin();
            end  = model->CollidedModels[i].collisions.end();
            for (; iter != end; ++iter)
            {
                xVector3 norm1 = xVector3::CrossProduct( iter->face1v[1] - iter->face1v[0], iter->face1v[2] - iter->face1v[0] ).normalize();
                xVector3 norm2 = xVector3::CrossProduct( iter->face2v[1] - iter->face2v[0], iter->face2v[2] - iter->face2v[0] ).normalize();
                float scale = (fabs(xVector3::DotProduct(norm1, norm2)) + 0.01f);
                normMSc += scale;
                centM += /*scale **/ iter->colPoint;
            }

            ModelObj *model2 = model->CollidedModels[i].model2;
            float massScaleM = model2->mass;
            centM /= numCM /** normMSc*/;
            xVector3 centerOfTheMassG = (xVector4::Create(model2->centerOfTheMass, 1.f) * model2->mLocationMatrix).vector3;
            xVector3 velo = model2->transVelocity +
                xVector3::CrossProduct(xQuaternion::angularVelocity(model2->rotatVelocity), centerOfTheMassG-centM);
            //velo *= 1.f - model->CollidedModels[i].model2->resilience;

            model->collisionVelo += velo * massScaleM;
            model->collisionNorm += velo.normalize() * normMSc * massScaleM / numCM;
            model->collisionCent += centM * massScaleM;
            normScale += normMSc;
            massScale += massScaleM;
        }
        if (!model->collisionNorm.length())
            model->collisionNorm = -model->transVelocity;
        if (model->collisionNorm.length())
        {
            model->collisionCent /= model->CollidedModels.size() * massScale;
            model->collisionVelo /= model->CollidedModels.size() * model->mass;

            xVector3 centerOfTheMassG = (xVector4::Create(model->centerOfTheMass, 1.f)*model->mLocationMatrix).vector3;
            xVector3 vArm             = centerOfTheMassG - model->collisionCent;

            model->collisionNorm.normalize();
            if (xVector3::DotProduct(vArm, model->collisionNorm) < 0.f)
                model->collisionNorm.invert();

            float pDepth = CalcPenetrationDepth(model, model->collisionCent, model->collisionNorm);
            float pcL = (pDepth * massScale / (massScale + model->CollidedModels.size() * model->mass));
            model->penetrationCorrection = model->collisionNorm * pcL;
        }
    }
}

void RigidBody :: CalculateMovement(ModelObj *model, float deltaTime)
{
    xVector3 T = model->transVelocity;
    T = T * 0.5f * deltaTime;
    if (fabs(T.x) > fabs(model->transVelocity.x) ||
        fabs(T.y) > fabs(model->transVelocity.y) ||
        fabs(T.z) > fabs(model->transVelocity.z))
        model->transVelocity.zero();
    else
        model->transVelocity -= T;

    if (deltaTime < 2.f)
    {
        xVector4 TR = xQuaternion::interpolateFull(model->rotatVelocity, deltaTime);
        TR.vector3.invert();
        model->rotatVelocity = xQuaternion::product(model->rotatVelocity, TR);
    }
    else
        model->rotatVelocity.zeroQ();

    float pcL = 0.f;

    if (!model->CollidedModels.empty())
    {
        if (model->collisionNorm.length())
        {
            pcL = model->penetrationCorrection.length();
            xVector3 centerOfTheMassG     = (xVector4::Create(model->centerOfTheMass, 1.f)*model->mLocationMatrix).vector3;
            //xVector3 centerOfTheMassGprev = (xVector4::Create(model->centerOfTheMass, 1.f)*model->mLocationMatrixPrev).vector3;
            //float tvL = (centerOfTheMassG-centerOfTheMassGprev).length();
            //if (tvL-pcL > 0.f && tvL > EPSILON) model->transVelocity *= (tvL-pcL)/tvL;
            //else               model->transVelocity.zero();

            //model->cp  = model->collisionCent;
            //model->com = centerOfTheMassG;
            //model->cno = model->cp + model->collisionNorm;

            // angular velocity
            xVector3 vArm  = centerOfTheMassG - model->collisionCent;
            xVector3 vArmN = xVector3::Normalize(vArm);
            float cosColAngle = xVector3::DotProduct(vArmN, model->collisionNorm);
            float sinColAngle = fabs(sqrt(1.f - cosColAngle*cosColAngle));

            if (sinColAngle > 0.01f)
            {
                float velo = ( xVector3::CrossProduct(xQuaternion::angularVelocity(model->rotatVelocity), vArm).length()
                    + model->transVelocity.length() + model->penetrationCorrection.length() ) * model->resilience;
                velo += model->collisionVelo.length();
                float Angle = sinColAngle * 0.5f * velo / vArm.length();
                if (Angle > 0.01f)
                {
                    xVector3 vAxis = xVector3::CrossProduct(vArmN, model->collisionNorm).normalize();
                    //model->cro = centerOfTheMassG+vAxis;
                    vAxis *= sin(Angle);
                    //xVector4 q; q.init(vAxis, cos(Angle));
                    //model->rotatVelocity = xQuaternion::product(model->rotatVelocity, q);
                    model->rotatVelocity.init(vAxis, cos(Angle));
                }
            }

            // straight movement
            if (1.f-sinColAngle)
            {
                float resilienceVeloL = model->transVelocity.length() * model->resilience;
                xVector3 resilianceVelo = model->collisionNorm * resilienceVeloL;
                //xVector3 resilianceVelo = model->transVelocity * (-model->resilience);
                model->transVelocity = (resilianceVelo + model->collisionVelo) * (1.f-sinColAngle);
                if (model->transVelocity.length() < 0.01f)
                    model->transVelocity.zero();
            }
        }
        model->CollidedModels.clear();
    }

    model->mLocationMatrixPrev = model->mLocationMatrix;

    bool needsRefill = false;
    if (xVector4::Normalize(model->rotatVelocity).vector3.length() > 0.01f)
    {
        xMatrix translation; translation.identity();
        translation.row3.vector3 = -model->centerOfTheMass;
        xMatrix rotation = translation * xMatrixFromQuaternion(xQuaternion::interpolateFull(model->rotatVelocity, deltaTime));
        translation.row3.vector3.invert();
        model->mLocationMatrix = rotation * translation * model->mLocationMatrix;
        needsRefill = true;
    }
    else
        model->rotatVelocity.zeroQ();

    if (pcL > 0.001f)
    {
        xMatrix translation;
        translation.identity();
        translation.row3.vector3 = model->penetrationCorrection;
        model->mLocationMatrix *= translation;
        needsRefill = true;
    }
    if (model->transVelocity.length() > 10.f * deltaTime)
    {
        xMatrix translation;
        translation.identity();
        translation.row3.vector3 = model->transVelocity * deltaTime;

        model->mLocationMatrix *= translation;
        needsRefill = true;
    }
    else
        model->transVelocity.zero();

    if (needsRefill)
        model->CollisionInfo_ReFill();

    if (model->physical)
        model->transVelocity.z -= 10.f * deltaTime;
}
