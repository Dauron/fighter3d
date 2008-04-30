#include "../World/ModelObj.h"

class RigidBody
{
public:
    static void CalculateMovement(ModelObj *model, float deltaTime)
    {
        xVector3 T = model->transVelocity;
        T = T * 0.5f * deltaTime;
        if (fabs(T.x) > fabs(model->transVelocity.x) ||
            fabs(T.y) > fabs(model->transVelocity.y) ||
            fabs(T.z) > fabs(model->transVelocity.z))
            model->transVelocity.Init(0.f, 0.f, 0.f);
        else
            model->transVelocity -= T;

        xVector4 TR = xQuaternion::interpolateFull(model->rotatVelocity, deltaTime);
        TR.vector3.invert();
        model->rotatVelocity = xQuaternion::product(model->rotatVelocity, TR);

        if (model->physical)
            model->transVelocity.z -= 10.f * deltaTime;

        if (!model->CollidedModels.empty())
        {
            std::vector<Collisions>::iterator iter, end;

            xVector3 collisionNorm; collisionNorm.Init(0.f, 0.f, 0.f);
            xVector3 collisionVelo; collisionVelo.Init(0.f, 0.f, 0.f);
            xVector3 collisionCent; collisionCent.Init(0.f, 0.f, 0.f);
            float    normScale = 0;
            float    massScale = 0;
            for (int i = model->CollidedModels.size()-1; i >= 0; --i)
            {
                //xVector3 normM(0.f, 0.f, 0.f);
                float    normMSc = 0;
                xVector3 centM; centM.Init(0.f, 0.f, 0.f);
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

                float massScaleM = model->CollidedModels[i].model2->mass;
                centM /= numCM /** normMSc*/;
                xVector3 velo = model->CollidedModels[i].model2->transVelocityPrev +
                    xVector3::CrossProduct(
                        xQuaternion::angularVelocity(model->CollidedModels[i].model2->rotatVelocityPrev),
                        model->CollidedModels[i].model2->centerOfTheMass-centM);
                //velo *= 1.f - model->CollidedModels[i].model2->resilience;

                collisionVelo += velo * massScaleM;
                collisionNorm +=  velo.normalize() * normMSc * massScaleM / numCM;
                collisionCent += centM * massScaleM;
                normScale += normMSc;
                massScale += massScaleM;
            }
            if (!collisionNorm.length())
                collisionNorm = -model->transVelocityPrev;
            if (collisionNorm.length())
            {
                collisionNorm.normalize();

                collisionCent /= model->CollidedModels.size() * massScale;
                collisionVelo /= model->CollidedModels.size() * model->mass;

                //cp = collisionCent;
                //com = centerOfTheMass;
                //cno = cp + collisionNorm;

                // angular velocity
                xVector3 vArm  = (model->centerOfTheMass - collisionCent);
                xVector3 vArmN = vArm.normalize();
                float cosColAngle = xVector3::DotProduct(vArmN, collisionNorm);
                float sinColAngle = fabs(sqrt(1.f - cosColAngle*cosColAngle));

                if (sinColAngle > 0.01f)
                {
                    float velo = ( xVector3::CrossProduct(xQuaternion::angularVelocity(model->rotatVelocityPrev), vArm).length()
                        + model->transVelocityPrev.length() ) * model->resilience;
                    velo += collisionVelo.length();
                    float Angle = sinColAngle * 0.5f * velo / vArm.length();
                    if (Angle > 0.1f)
                    {
                        xVector3 vAxis = xVector3::CrossProduct(vArmN, collisionNorm).normalize();
                        vAxis *= sin(Angle);
                        xVector4 q; q.Init(vAxis, cos(Angle));
                        model->rotatVelocity = xQuaternion::product(model->rotatVelocity, q);
                    }
                }

                // straight movement
                if (1.f-sinColAngle)
                {
                    float resilienceVeloL = model->transVelocity.length() * model->resilience;
                    xVector3 resilianceVelo = collisionNorm * resilienceVeloL;
                    //xVector3 resilianceVelo = model->transVelocity * (-model->resilience);
                    model->transVelocity = (resilianceVelo + collisionVelo) * (1.f-sinColAngle);
                    if (model->transVelocity.length() < 0.01f)
                        model->transVelocity.Init(0.f,0.f,0.f);
                }
            }
            model->CollidedModels.clear();
        }

        bool needsRefill = false;
        if (xVector4::Normalize(model->rotatVelocity).vector3.length() > 0.01f)
        {
            xMatrix rotation = xMatrixFromQuaternion( xQuaternion::interpolateFull(model->rotatVelocity, deltaTime));
            model->mLocationMatrix = rotation * model->mLocationMatrix;
            needsRefill = true;
        }
        else
            model->rotatVelocity.Init(0.f, 0.f, 0.f, 1.f);

        if (model->transVelocity.length() > 0.01f)
        {
            xMatrix translation;
            translation.identity();
            translation.row3.vector3 = model->transVelocity * deltaTime;

            model->mLocationMatrix *= translation;
            needsRefill = true;
        }
        else
            model->transVelocity.Init(0.f, 0.f, 0.f);

        if (needsRefill)
            model->CollisionInfo_ReFill();
    }
};
