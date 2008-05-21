#include "ModelObj.h"
#include "../Physics/RigidBody.h"

void ModelObj :: Initialize (const char *gr_filename, const char *ph_filename, bool physicalNotLocked, bool phantom)
{
    collisionInfo = NULL;
    mLocationMatrixPrev = mLocationMatrix;
    transVelocity.zero();
    rotatVelocity.zeroQ();
    mass               = !locked ? 1.f : 100000000.f;
    resilience         = 0.2f;
    gravityAccumulator = 1.f;
    this->phantom = phantom;
    this->physical = physicalNotLocked;
    this->locked   = !physicalNotLocked;
    smap.texId = 0;

    //renderer.UseVBO = forceNotStatic;

    modelInstanceGr.Zero();
    modelInstancePh.Zero();
    modelInstanceGr.location = mLocationMatrix;
    modelInstancePh.location = mLocationMatrix;

    hModelGraphics = g_ModelMgr.GetModel(gr_filename);
    Model3dx *mdl  = g_ModelMgr.GetModel(hModelGraphics);
    xModelGr = mdl->model;
    if (ph_filename)
    {
        hModelPhysical = g_ModelMgr.GetModel(ph_filename);
        xModelPh = g_ModelMgr.GetModel(hModelPhysical)->model;
    }
    else
    {
        hModelPhysical = hModelGraphics;
        xModelPh = xModelGr;
        mdl->IncReferences();
    }

    VerticesChanged(true);
}

void ModelObj :: Finalize ()
{
    if (smap.texId)
    {
        GLuint tid = smap.texId;
        glDeleteTextures(1, &tid);
        smap.texId = 0;
    }
    renderer.FreeInstanceGraphics(modelInstanceGr);
    renderer.FreeInstanceGraphics(modelInstancePh);
    if (collisionInfo)
    {
        CollisionInfo_Free(GetModelPh()->kidsP, collisionInfo);
        delete[] collisionInfo;
        collisionInfo = NULL;
    }
    FreeInstanceData();
    g_ModelMgr.DeleteModel(hModelGraphics);
    g_ModelMgr.DeleteModel(hModelPhysical);
}

void ModelObj :: FreeInstanceData()
{
    modelInstanceGr.Clear();
    if (hModelGraphics == hModelPhysical)
        modelInstancePh.elementInstanceP = NULL;
    modelInstancePh.bonesM = NULL;
    modelInstancePh.bonesQ = NULL;
    modelInstancePh.bonesMod = NULL;
    modelInstancePh.Clear();
}

void ModelObj :: VerticesChanged(bool free)
{
    if (free) {
        renderer.FreeInstanceGraphics(modelInstanceGr);
        renderer.FreeInstanceGraphics(modelInstancePh);
        FreeInstanceData();

        modelInstanceGr.elementInstanceC = xModelGr->elementC;
        modelInstanceGr.elementInstanceP = new xElementInstance[xModelGr->elementC];
        modelInstanceGr.ZeroElements();
        //memset(modelInstanceGr.elementInstanceP, 0, modelInstanceGr.elementInstanceC * sizeof(xElementInstance));
        if (hModelGraphics != hModelPhysical)
        {
            modelInstancePh.elementInstanceC = xModelPh->elementC;
            modelInstancePh.elementInstanceP = new xElementInstance[xModelPh->elementC];
            modelInstancePh.ZeroElements();
            //memset(modelInstancePh.elementInstanceP, 0, modelInstancePh.elementInstanceC * sizeof(xElementInstance));
        }
        else
        {
            modelInstancePh.elementInstanceC = modelInstanceGr.elementInstanceC;
            modelInstancePh.elementInstanceP = modelInstanceGr.elementInstanceP;
        }
    }
    else InvalidateShadowRenderData();

    xBoneCalculateMatrices (xModelGr->spine, &modelInstanceGr);
    xBoneCalculateQuats    (xModelGr->spine, &modelInstanceGr);
    modelInstancePh.bonesM   = modelInstanceGr.bonesM;
    modelInstancePh.bonesQ   = modelInstanceGr.bonesQ;
    modelInstancePh.bonesMod = modelInstanceGr.bonesMod;
    modelInstancePh.bonesC   = modelInstanceGr.bonesC;

    xModel_SkinElementInstance(xModelGr, modelInstanceGr);
    modelInstancePh.center = modelInstanceGr.center = xModel_GetBounds(modelInstanceGr);
    if (hModelGraphics != hModelPhysical)
    {
        xModel_SkinElementInstance(xModelPh, modelInstancePh);
        modelInstancePh.center = xModel_GetBounds(modelInstancePh);
    }
}

void ModelObj :: CalculateSkeleton()
{
    if (!xModelGr->spine.boneC) return;
    if (xModelGr->spine.boneC != modelInstanceGr.bonesC)
    {
        modelInstanceGr.ClearSkeleton();
        if (!modelInstanceGr.bonesC) // skeleton was added, so refresh VBO data
        {
            VerticesChanged(true);
            return;
        }
    }
    VerticesChanged(false);
    renderer.InvalidateBonePositions(modelInstanceGr);
    renderer.InvalidateBonePositions(modelInstancePh);
}




void ModelObj:: RenderObject(bool transparent, const xFieldOfView &FOV)
{
    UpdatePointers();
    modelInstancePh.location = modelInstanceGr.location = mLocationMatrix;
    renderer.RenderModel(*xModelGr, modelInstanceGr, transparent, FOV);
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
    UpdatePointers();
    collisionInfoC = xModelPh->elementC;
    collisionInfo  = new xCollisionHierarchyBoundsRoot[collisionInfoC];

    for (xElement *elem = xModelPh->kidsP; elem; elem = elem->nextP)
        CollisionInfo_Fill(elem, collisionInfo, true);

    Performance.CollisionDataFillMS += GetTick() - delta;

    return collisionInfo;
}

void ModelObj::CollisionInfo_ReFill()
{
    smap.texId = 0;
    if (collisionInfo)
    {
        float   delta  = GetTick();
        UpdatePointers();
        for (xElement *elem = xModelPh->kidsP; elem; elem = elem->nextP)
            CollisionInfo_Fill(elem, collisionInfo, false);
        Performance.CollisionDataFillMS += GetTick() - delta;
    }
}

void ModelObj::CollisionInfo_Fill(xElement *elem, xCollisionHierarchyBoundsRoot *ci, bool firstTime)
{
    xCollisionHierarchyBoundsRoot &eci = ci[elem->id];
    UpdatePointers();

    // Create Hierarchy, if not exists
    if (firstTime)
    {
        if (!elem->collisionData.kidsP)
            elem->collisionData.Fill(xModelPh, elem);
        eci.kids = NULL;
        eci.verticesP = NULL;
    }

    if (eci.verticesC != modelInstancePh.elementInstanceP[elem->id].verticesC)
    {
        eci.verticesC = modelInstancePh.elementInstanceP[elem->id].verticesC;
        if (eci.verticesP) delete[] eci.verticesP;
        eci.verticesP = new xVector4[eci.verticesC];
    }

    xVector4 *iterS = modelInstancePh.elementInstanceP[elem->id].verticesP, *iterD = eci.verticesP;
    xMatrix   transf = elem->matrix * mLocationMatrix;
    for (int i = eci.verticesC; i; --i, ++iterS, ++iterD)
        iterD->init(transf.preTransformP(iterS->vector3), 1.f);

    // Fill collision bounding boxes
    xElement_CalcCollisionHierarchyBox(eci.verticesP, &elem->collisionData, &eci);

    // Fill CI for subelements
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Fill(celem, ci, firstTime);
}

void ModelObj::CollisionInfo_Free(xElement *elem, xCollisionHierarchyBoundsRoot *ci)
{
    if (!elem) return;
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
    xVector3 centerOfTheMassG = (xVector4::Create(modelInstancePh.center, 1.f)*modelInstancePh.location).vector3;
    xVector3 lFAxis = (light->position - centerOfTheMassG).normalize();
    xVector3 tmp;
    if (fabs(lFAxis.x) <= fabs(lFAxis.y))
        if (fabs(lFAxis.z) <= fabs(lFAxis.x)) tmp.init(0.f,0.f,1.f);
        else                                  tmp.init(1.f,0.f,0.f);
    else
        if (fabs(lFAxis.z) <= fabs(lFAxis.y)) tmp.init(0.f,0.f,1.f);
        else                                  tmp.init(0.f,1.f,0.f);

    xVector3 lRAxis = xVector3::CrossProduct(lFAxis, tmp).normalize();
    xVector3 lUAxis = xVector3::CrossProduct(lFAxis, lRAxis);

    // Build view-transformation matrix
    xMatrix lViewMatrix;
    lViewMatrix.row0.vector3 = lRAxis; lViewMatrix.row0.w = 0.f;
    lViewMatrix.row1.vector3 = lUAxis; lViewMatrix.row1.w = 0.f;
    lViewMatrix.row2.vector3 = lFAxis; lViewMatrix.row2.w = 0.f;
    lViewMatrix.row3.zeroQ();
    lViewMatrix.postTranslate(-light->position).transpose();

    // Find the horizontal and vertical angular spreads to find out FOV and aspect ratio
    float RxMax = 0.f, RyMax = 0.f;
    xCollisionHierarchyBoundsRoot* ci = GetCollisionInfo();
    for (int i=collisionInfoC; i; --i, ++ci)
    {
        xVector4 *iter = ci->verticesP;
        for (int j=ci->verticesC; j; --j, ++iter)
        {
            xVector4 v = lViewMatrix * *iter;
            v.z = 1.f / v.z;
            v.x *= v.z; v.y *= v.z;

            if (fabs(v.x) > RxMax) RxMax = fabs(v.x);
            if (fabs(v.y) > RyMax) RyMax = fabs(v.y);
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
    mtxBlockerToLight.preMultiply(lViewMatrix).preMultiply(mLocationMatrix);

    // Projection matrix for computing UVs on the receiver object
    mtxReceiverUVMatrix.row0.init(XProj*0.5f, 0.f, 0.f, 0.f);
    mtxReceiverUVMatrix.row1.init(0.f, YProj*0.5f, 0.f, 0.f);
    mtxReceiverUVMatrix.row2.init(-0.5f, -0.5f, (ZFar+ZNear)/(ZNear-ZFar), -1.f);
    mtxReceiverUVMatrix.row3.init(0.f, 0.f, 2.f*ZFar*ZNear/(ZNear-ZFar), 0.f);
    mtxReceiverUVMatrix.preMultiply(lViewMatrix);
}
