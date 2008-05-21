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
    mass       = physical ? 1.f : 100000000.f;
    resilience = 0.2f;
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

void ModelObj:: RenderObject(bool transparent)
{
    assert(renderer.IsValid());
    GetRenderer()->RenderModel(transparent);
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

xCollisionHierarchyBoundsRoot *ModelObj::GetCollisionInfo()
{
    if (collisionInfo) return collisionInfo;

    float delta = GetTick();

    idx.clear();
    xRender *rend  = GetRenderer();
    collisionInfoC = xElementCount(rend->xModelPhysical);
    collisionInfo  = new xCollisionHierarchyBoundsRoot[collisionInfoC];

    for (xElement *elem = rend->xModelPhysical->firstP; elem; elem = elem->nextP)
        CollisionInfo_Fill(rend, elem, collisionInfo, true);

    time1 += GetTick() - delta;

    return collisionInfo;
}

void ModelObj::CollisionInfo_ReFill()
{
    if (collisionInfo)
    {
        float   delta  = GetTick();
        xRender *rend  = GetRenderer();
        for (xElement *elem = rend->xModelPhysical->firstP; elem; elem = elem->nextP)
            CollisionInfo_Fill(rend, elem, collisionInfo, false);
        time1 += GetTick() - delta;
    }
}

void ModelObj::CollisionInfo_Fill(xRender *rend, xElement *elem, xCollisionHierarchyBoundsRoot *ci, bool firstTime)
{
    xCollisionHierarchyBoundsRoot &eci = ci[elem->id];
    if (firstTime) eci.verticesP = NULL;
    xElement_GetSkinnedVertices(elem, rend->bonesM, mLocationMatrix, eci.verticesP, false);
    eci.verticesC = elem->verticesC;

    // Create Hierarchy, if not exists
    if (firstTime)
    {
        if (!elem->collisionData.kidsP)
            xElement_GetCollisionHierarchy(rend->xModelPhysical, elem);
        eci.kids = NULL;
    }
    // Fill collision bounding boxes
    xElement_CalcCollisionHierarchyBox(eci.verticesP, &elem->collisionData, &eci);

    // Fill CI for subelements
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Fill(rend, celem, ci, firstTime);
}

void ModelObj::CollisionInfo_Free(xElement *elem, xCollisionHierarchyBoundsRoot *ci)
{
    xElement_FreeCollisionHierarchyBounds(&elem->collisionData, ci[elem->id].kids);
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

void ModelObj::CollisionInfo_Render(xElement *elem, xCollisionHierarchyBoundsRoot *ci)
{
    if (elem->verticesC)
        for (int i=0; i<elem->collisionData.kidsC; ++i)
            RenderCI(elem->collisionData.kidsP + i, ci[elem->id].kids + i);
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Render(celem, ci);
}

void ModelObj :: GetShadowProjectionMatrix(xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width)
{
    //E3d_MatrixCopy(LModel->LocalToWorldMatrix,LBlockerLocalToWorldMatrix);

    xVector3 centerOfTheMassG = (xVector4::Create(centerOfTheMass, 1.f)*mLocationMatrix).vector3;
    xVector3 lFAxis = (light->position - centerOfTheMassG).normalize();
    xVector3 tmp;
    if (abs(lFAxis.x) <= abs(lFAxis.y))
        if (abs(lFAxis.z) <= abs(lFAxis.x)) tmp.init(0.f,0.f,1.f);
        else                                tmp.init(1.f,0.f,0.f);
    else
        if (abs(lFAxis.z) <= abs(lFAxis.y)) tmp.init(0.f,0.f,1.f);
        else                                tmp.init(0.f,1.f,0.f);

    xVector3 lRAxis = xVector3::CrossProduct(lFAxis, tmp).normalize();
    xVector3 lUAxis = xVector3::CrossProduct(lFAxis, lRAxis);

    // Build view-transformation matrix
    xMatrix lViewMatrix;
    lViewMatrix.row0.vector3 = lRAxis; lViewMatrix.row0.w = 0.f;
    lViewMatrix.row1.vector3 = lUAxis; lViewMatrix.row1.w = 0.f;
    lViewMatrix.row2.vector3 = lFAxis; lViewMatrix.row2.w = 0.f;
    lViewMatrix.row3.zeroQ();
    xMatrix lViewMatrixT = lViewMatrix * xMatrixTranslate(-light->position.x, -light->position.y, -light->position.z);
    lViewMatrixT.transpose();

    // Find the horizontal and vertical angular spreads to find out FOV and aspect ratio
    float RxMax = 0.f, RyMax = 0.f;
    xCollisionHierarchyBoundsRoot* ci = GetCollisionInfo();
    for (int i=collisionInfoC; i; --i, ++ci)
    {
        xVector4 *iter = ci->verticesP;
        for (int j=ci->verticesC; j; --j, ++iter)
        {
            xVector4 v = *iter * lViewMatrixT;
            v.z = 1.f / v.z;
            v.x *= v.z; v.y *= v.z;

            if (abs(v.x) > RxMax) RxMax = abs(v.x);
            if (abs(v.y) > RyMax) RyMax = abs(v.y);
        }
    }
    float ZNear=0.1f, ZFar=1000.f;

    // Set up the perspective-projection matrix
    //
    //    f = cotangent(Field-of-view) = 1.0/LYMax
    //    f/aspect=1.0/LXMax
    //
    //    | f/aspect   0         0.0        0.0   |
    //    |                                       |
    //    |                                       |
    //    |                                       |
    //    |    0       f         0.0        0.0   |
    //    |                                       |
    //    |                                       |
    //    |                 Zfar+Znear            |
    //    |    0       0    ----------     -1.0   |
    //    |                 Znear-Zfar            |
    //    |                      |
    //    |                2*Zfar*Znear           |
    //    |    0       0   ------------     0.0   |
    //    |                 Znear-Zfar            |

    // Leave a pixel or two around the edges of the
    // shadow map for texture clamping
    float XProj=(0.995f-1.f/(float)width)/RxMax;
    float YProj=(0.995f-1.f/(float)width)/RyMax;
    mtxBlockerToLight.row0.init(XProj, 0.f, 0.f, 0.f);
    mtxBlockerToLight.row1.init(0.f, YProj, 0.f, 0.f);
    mtxBlockerToLight.row2.init(0.f, 0.f, (ZFar+ZNear)/(ZNear-ZFar), -1.f);
    mtxBlockerToLight.row3.init(0.f, 0.f, 2.f*ZFar*ZNear/(ZNear-ZFar), 0.f);
    mtxBlockerToLight = mLocationMatrix * lViewMatrixT * mtxBlockerToLight;

    // Projection matrix for computing UVs on the receiver object
    mtxReceiverUVMatrix.row0.init(XProj*0.5f, 0.f, 0.f, 0.f);
    mtxReceiverUVMatrix.row1.init(0.f, YProj*0.5f, 0.f, 0.f);
    mtxReceiverUVMatrix.row2.init(-0.5f, -0.5f, (ZFar+ZNear)/(ZNear-ZFar), -1.f);
    mtxReceiverUVMatrix.row3.init(0.f, 0.f, 2.f*ZFar*ZNear/(ZNear-ZFar), 0.f);
    mtxReceiverUVMatrix = lViewMatrixT * mtxReceiverUVMatrix;
}
