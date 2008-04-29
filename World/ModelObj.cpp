#include "ModelObj.h"

void ModelObj::Update(float deltaTime)
{
    if (deltaTime > 0.5f)
    {
        CollidedModels.clear();
        return;
    }

    xVector3 T = transVelocity;
    T = T.normalize() * 1.f * deltaTime;
    if (fabs(T.x) > fabs(transVelocity.x) || fabs(T.y) > fabs(transVelocity.y) || fabs(T.z) > fabs(transVelocity.z))
        transVelocity.x = transVelocity.y = transVelocity.z = 0.f;
    else
        transVelocity -= T;

    xVector4 TR = xQuaternion::interpolateFull(rotatVelocity, deltaTime);
    TR.vector3.invert();
    rotatVelocity = xQuaternion::product(rotatVelocity, TR);

    if (physical)
        transVelocity.z -= 10.f * deltaTime;

    if (!CollidedModels.empty())
    {
        std::vector<Collisions>::iterator iter, end;

        xVector3 collisionNorm; collisionNorm.Init(0.f, 0.f, 0.f);
        xVector3 collisionVelo; collisionVelo.Init(0.f, 0.f, 0.f);
        xVector3 collisionCent; collisionCent.Init(0.f, 0.f, 0.f);
        float    normScale = 0;
        float    massScale = 0;
        for (int i = CollidedModels.size()-1; i >= 0; --i)
        {
            //xVector3 normM(0.f, 0.f, 0.f);
            float    normMSc = 0;
            xVector3 centM; centM.Init(0.f, 0.f, 0.f);
            xWORD    numCM = CollidedModels[i].collisions.size();

            iter = CollidedModels[i].collisions.begin();
            end  = CollidedModels[i].collisions.end();
            for (; iter != end; ++iter)
            {
                xVector3 norm1 = xVector3::CrossProduct( iter->face1v[1] - iter->face1v[0], iter->face1v[2] - iter->face1v[0] ).normalize();
                xVector3 norm2 = xVector3::CrossProduct( iter->face2v[1] - iter->face2v[0], iter->face2v[2] - iter->face2v[0] ).normalize();
                float scale = (fabs(xVector3::DotProduct(norm1, norm2)) + 0.01f);
                normMSc += scale;
                centM += /*scale **/ iter->colPoint;
            }

            float massScaleM = CollidedModels[i].model2->mass;
            centM /= numCM /** normMSc*/;
            xVector3 velo = CollidedModels[i].model2->transVelocityPrev +
                xVector3::CrossProduct(
                    xQuaternion::angularVelocity(CollidedModels[i].model2->rotatVelocityPrev),
                    CollidedModels[i].model2->centerOfTheMass-centM);
            //velo *= 1.f - CollidedModels[i].model2->resilience;

            collisionVelo += velo * massScaleM;
            collisionNorm +=  velo.normalize() * normMSc * massScaleM / numCM;
            collisionCent += centM * massScaleM;
            normScale += normMSc;
            massScale += massScaleM;
        }
        if (!collisionNorm.length())
            collisionNorm = -transVelocityPrev;
        if (collisionNorm.length())
        {
            collisionNorm.normalize();

            collisionCent /= CollidedModels.size() * massScale;
            collisionVelo /= CollidedModels.size() * mass;

            //cp = collisionCent;
            //com = centerOfTheMass;
            //cno = cp + collisionNorm;

            // angular velocity
            xVector3 vArm  = (centerOfTheMass - collisionCent);
            xVector3 vArmN = vArm.normalize();
            float cosColAngle = xVector3::DotProduct(vArmN, collisionNorm);
            float sinColAngle = fabs(sqrt(1.f - cosColAngle*cosColAngle));

            if (sinColAngle > 0.01f)
            {
                float velo = ( xVector3::CrossProduct(xQuaternion::angularVelocity(rotatVelocityPrev), vArm).length()
                    + transVelocityPrev.length() ) * resilience;
                velo += collisionVelo.length();
                float Angle = sinColAngle * 0.5f * velo / vArm.length();
                if (Angle > 0.1f)
                {
                    xVector3 vAxis = xVector3::CrossProduct(vArmN, collisionNorm).normalize();
                    vAxis *= sin(Angle);
                    xVector4 q; q.Init(vAxis, cos(Angle));
                    rotatVelocity = xQuaternion::product(rotatVelocity, q);
                }
            }

            // straight movement
            if (1.f-sinColAngle)
            {
                float resilienceVeloL = transVelocity.length() * resilience;
                xVector3 resilianceVelo = collisionNorm * resilienceVeloL;
                //xVector3 resilianceVelo = transVelocity * (-resilience);
                transVelocity = (resilianceVelo + collisionVelo) * (1.f-sinColAngle);
                if (transVelocity.length() < 0.01f)
                    transVelocity.x = transVelocity.y = transVelocity.z = 0.f;
            }
        }
        CollidedModels.clear();
    }

    bool needsRefill = false;
    if (xVector4::Normalize(rotatVelocity).vector3.length() > 0.01f)
    {
        xMatrix rotation = xMatrixFromQuaternion( xQuaternion::interpolateFull(rotatVelocity, deltaTime));
        mLocationMatrix = rotation * mLocationMatrix;
        needsRefill = true;
    }
    else
    {
        rotatVelocity.x = rotatVelocity.y = rotatVelocity.z = 0.f;
        rotatVelocity.w = 1.f;
    }

    if (transVelocity.length() > 0.01f)
    {
        xMatrix translation;
        translation.identity();
        translation.row3.vector3 = transVelocity * deltaTime;

        mLocationMatrix *= translation;
        needsRefill = true;
    }
    else
        transVelocity.x = transVelocity.y = transVelocity.z = 0.f;

    if (needsRefill)
        CollisionInfo_ReFill();
}

CollisionInfo *ModelObj::GetCollisionInfo()
{
    if (collisionInfo) return collisionInfo;

    idx.clear();
    xRender *rend  = GetRenderer();
    collisionInfoC = xElementCount(rend->xModel);

    collisionInfo = new CollisionInfo[collisionInfoC];

    centerOfTheMass.x = centerOfTheMass.y = centerOfTheMass.z = 0.f;
    xDWORD   verticesC = 0;

    for (xElement *elem = rend->xModel->firstP; elem; elem = elem->nextP)
        verticesC += CollisionInfo_Fill(rend, elem, collisionInfo, true);

    if (verticesC)
        centerOfTheMass /= (float)verticesC;

    return collisionInfo;
}

void ModelObj::CollisionInfo_ReFill()
{
    if (collisionInfo)
    {
        xRender *rend  = GetRenderer();
        centerOfTheMass.x = centerOfTheMass.y = centerOfTheMass.z = 0.f;
        xDWORD   verticesC = 0;
        for (xElement *elem = rend->xModel->firstP; elem; elem = elem->nextP)
            verticesC += CollisionInfo_Fill(rend, elem, collisionInfo, false);
        if (verticesC)
            centerOfTheMass /= (float)verticesC;
    }
}

xDWORD ModelObj::CollisionInfo_Fill(xRender *rend, xElement *elem, CollisionInfo *ci, bool firstTime)
{
    if (firstTime) ci[elem->id].verticesP = NULL;
    xElement_GetSkinnedVertices(elem, rend->bonesM, mLocationMatrix, ci[elem->id].verticesP, false);
    xDWORD verticesC = ci[elem->id].verticesC = elem->verticesC;

    centerOfTheMass += xCenterOfTheMass(ci[elem->id].verticesP, ci[elem->id].verticesC, false);
    ci[elem->id].bounding  = xBoundingBox(ci[elem->id].verticesP, elem->verticesC);

    if (firstTime)
    {
        if (!elem->collisionData.hierarchyP)
            xElement_GetCollisionHierarchy(rend->xModel, elem);
        ci[elem->id].collisionHP = NULL;
    }
    xElement_CalcCollisionHierarchyBox(ci[elem->id].verticesP,
        elem->collisionData.hierarchyP, elem->collisionData.hierarchyC,
        ci[elem->id].collisionHP);

    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        verticesC += CollisionInfo_Fill(rend, celem, ci, firstTime);

    return verticesC;
}

void ModelObj::CollisionInfo_Free(xElement *elem, CollisionInfo *ci)
{
    xElement_FreeCollisionHierarchyBounds(elem->collisionData.hierarchyP, elem->collisionData.hierarchyC,
        ci[elem->id].collisionHP);

    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Free(celem, ci);
}

void RenderCI(xCollisionHierarchy *ch, xCollisionHierarchyBounds *chb)
{
    if (ch->kidsC)
    {
        for (int i=0; i<ch->kidsC; ++i)
            RenderCI(ch->kidsP + i, chb->kids + i);
    }
    else
    {
        glBegin(GL_QUAD_STRIP);
        {
            glVertex3f(chb->bounding.max.x, chb->bounding.max.y, chb->bounding.max.z);
            glVertex3f(chb->bounding.max.x, chb->bounding.max.y, chb->bounding.min.z);
            glVertex3f(chb->bounding.max.x, chb->bounding.min.y, chb->bounding.max.z);
            glVertex3f(chb->bounding.max.x, chb->bounding.min.y, chb->bounding.min.z);

            glVertex3f(chb->bounding.min.x, chb->bounding.min.y, chb->bounding.max.z);
            glVertex3f(chb->bounding.min.x, chb->bounding.min.y, chb->bounding.min.z);

            glVertex3f(chb->bounding.min.x, chb->bounding.max.y, chb->bounding.max.z);
            glVertex3f(chb->bounding.min.x, chb->bounding.max.y, chb->bounding.min.z);

            glVertex3f(chb->bounding.max.x, chb->bounding.max.y, chb->bounding.max.z);
            glVertex3f(chb->bounding.max.x, chb->bounding.max.y, chb->bounding.min.z);
        }
        glEnd();
    }
}

void ModelObj::CollisionInfo_Render(xElement *elem, CollisionInfo *ci)
{
    if (elem->verticesC)
        for (int i=0; i<elem->collisionData.hierarchyC; ++i)
            RenderCI(elem->collisionData.hierarchyP + i, ci[elem->id].collisionHP + i);

    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Render(celem, ci);
}
