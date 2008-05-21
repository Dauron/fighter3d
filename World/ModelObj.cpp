#include "ModelObj.h"
#include "../Physics/RigidBody.h"

float time1;
float time2;
float time1b = 0.f;
float time2b = 0.f;

void ModelObj:: Initialize (const char *gr_filename, const char *ph_filename, bool physical, bool phantom)
{
    assert(!renderer.IsValid());

    collisionInfo = NULL;
    mLocationMatrixPrev = mLocationMatrix;
    transVelocity.zero();
    rotatVelocity.zeroQ();
    mass       = 1.f;
    resilience = 0.5f;
    gravityAccumulator = 1.f;
    this->phantom = phantom;
    this->physical = physical;

    if (ph_filename)
        renderer.Initialize(!physical && !forceNotStatic, g_ModelMgr.GetModel(gr_filename), g_ModelMgr.GetModel(ph_filename));
    else
        renderer.Initialize(!physical && !forceNotStatic, g_ModelMgr.GetModel(gr_filename));
    xRender *rend = GetRenderer();
    centerOfTheMass = xCenterOfTheModelMass(rend->xModelPhysical, rend->bonesM);
}

void ModelObj:: Finalize ()
{
    if (collisionInfo)
    {
        CollisionInfo_Free(GetRenderer()->xModelPhysical->firstP, collisionInfo);
        delete[] collisionInfo;
        collisionInfo = NULL;
    }
    renderer.Finalize();
}

void ModelObj:: RenderObject()
{
    assert(renderer.IsValid());
    GetRenderer()->RenderModel();
/*
    if (!phantom)
        if (!CollidedModels.empty())
        {
            srand(100);
            //if (idx.empty())
            {
                std::vector<CollisionWithModel>::iterator cmiter;
                for (cmiter = CollidedModels.begin(); cmiter < CollidedModels.end(); ++cmiter)
                {
                    std::vector<Collisions>::iterator iter;
                    std::vector<xDWORD>::iterator found;
                    xElement *prevElem = NULL;
                    for (iter = cmiter->collisions.begin(); iter < cmiter->collisions.end(); ++iter)
                    {
                        if (prevElem != iter->elem1)
                        {
                            if (prevElem)
                                GetRenderer()->RenderFaces(prevElem->id, &idx);
                            prevElem = iter->elem1;
                            idx.clear();
                        }
                        found = std::find(idx.begin(), idx.end(), (xDWORD)iter->face1);
                        if (found == idx.end()) idx.push_back((xDWORD)iter->face1);
                    }
                    if (prevElem)
                        GetRenderer()->RenderFaces(prevElem->id, &idx);
                    idx.clear();
                }
            }
        }
*/
/*
    GetCollisionInfo();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    g_TextureMgr.DisableTextures();
    glColor3f(1.f, 1.f, 0.f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPointSize(5.f);
    glPopMatrix();
    //CollisionInfo_Render(GetRenderer()->xModel->firstP, collisionInfo);
    glBegin(GL_LINES);
    {
        glColor3f(1.f, 1.f, 0.f);
        glVertex3fv(cp.xyz);
        glVertex3fv(com.xyz);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3fv(cp.xyz);
        glVertex3fv(cno.xyz);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3fv(com.xyz);
        glVertex3fv(cro.xyz);
    }
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPushMatrix();
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);*/

}

void ModelObj:: PreUpdate()
{
    RigidBody::CalculateCollisions(this);
}

void ModelObj:: Update(float deltaTime)
{
    RigidBody::CalculateMovement(this, deltaTime);
}

CollisionInfo *ModelObj::GetCollisionInfo()
{
    if (collisionInfo) return collisionInfo;

    float delta = GetTick();

    idx.clear();
    xRender *rend  = GetRenderer();
    collisionInfoC = xElementCount(rend->xModelPhysical);

    collisionInfo = new CollisionInfo[collisionInfoC];

    //centerOfTheMass.x = centerOfTheMass.y = centerOfTheMass.z = 0.f;
    xDWORD   verticesC = 0;

    for (xElement *elem = rend->xModelPhysical->firstP; elem; elem = elem->nextP)
        verticesC += CollisionInfo_Fill(rend, elem, collisionInfo, true);

    //if (verticesC)
    //    centerOfTheMass /= (float)verticesC;

    time1 += GetTick() - delta;

    return collisionInfo;
}

void ModelObj::CollisionInfo_ReFill()
{
    if (collisionInfo)
    {
        float delta = GetTick();

        xRender *rend  = GetRenderer();
        //centerOfTheMass.x = centerOfTheMass.y = centerOfTheMass.z = 0.f;
        xDWORD   verticesC = 0;
        for (xElement *elem = rend->xModelPhysical->firstP; elem; elem = elem->nextP)
            verticesC += CollisionInfo_Fill(rend, elem, collisionInfo, false);
        //if (verticesC)
        //    centerOfTheMass /= (float)verticesC;

        time1 += GetTick() - delta;
    }
}

xDWORD ModelObj::CollisionInfo_Fill(xRender *rend, xElement *elem, CollisionInfo *ci, bool firstTime)
{
    if (firstTime) ci[elem->id].verticesP = NULL;
    xElement_GetSkinnedVertices(elem, rend->bonesM, mLocationMatrix, ci[elem->id].verticesP, false);
    xDWORD verticesC = ci[elem->id].verticesC = elem->verticesC;

    /*centerOfTheMass += */xCenterOfTheMass(ci[elem->id].verticesP, ci[elem->id].verticesC, false);
    ci[elem->id].bounding  = xBoundingBox(ci[elem->id].verticesP, elem->verticesC);

    if (firstTime)
    {
        if (!elem->collisionData.kidsP)
            xElement_GetCollisionHierarchy(rend->xModelPhysical, elem);
        ci[elem->id].collisionHP = NULL;
    }
    xElement_CalcCollisionHierarchyBox(ci[elem->id].verticesP,
        &elem->collisionData, ci[elem->id].collisionHP);

    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        verticesC += CollisionInfo_Fill(rend, celem, ci, firstTime);

    return verticesC;
}

void ModelObj::CollisionInfo_Free(xElement *elem, CollisionInfo *ci)
{
    xElement_FreeCollisionHierarchyBounds(&elem->collisionData, ci[elem->id].collisionHP);

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
        for (int i=0; i<elem->collisionData.kidsC; ++i)
            RenderCI(elem->collisionData.kidsP + i, ci[elem->id].collisionHP + i);

    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Render(celem, ci);
}
