#include "RigidObj.h"
#include "../Physics/RigidBody.h"

void RigidObj :: Initialize (const char *gr_filename, const char *ph_filename, bool physicalNotLocked, bool phantom)
{
    Type = Model_Rigid;
    collisionInfo = NULL;
    MX_ModelToWorld_prev = MX_ModelToWorld;
    xMatrix::Invert(MX_ModelToWorld, MX_WorldToModel);

    mass               = !locked ? 1.f : 100000000.f;
    resilience         = 0.2f;
    this->phantom = phantom;
    this->physical = physicalNotLocked;
    this->locked   = !physicalNotLocked;
    smap.texId = 0;

    //renderer.UseVBO = forceNotStatic;
    
    modelInstanceGr.Zero();
    modelInstancePh.Zero();
    modelInstanceGr.location = MX_ModelToWorld;
    modelInstancePh.location = MX_ModelToWorld;
    
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

    CreateVerletSystem();
    VerticesChanged(true);
}

void RigidObj :: Finalize ()
{
    if (smap.texId)
    {
        GLuint tid = smap.texId;
        glDeleteTextures(1, &tid);
        smap.texId = 0;
    }
    UpdatePointers();
    Model3dx *mdlGr = g_ModelMgr.GetModel(hModelGraphics);
    Model3dx *mdlPh = g_ModelMgr.GetModel(hModelPhysical);
    renderer.FreeGraphics(*xModelGr, modelInstanceGr, mdlGr->m_References == 1);
    renderer.FreeGraphics(*xModelPh, modelInstancePh, mdlPh->m_References == 1);
    if (collisionInfo)
    {
        CollisionInfo_Free(GetModelPh()->kidsP, collisionInfo);
        delete[] collisionInfo;
        collisionInfo = NULL;
    }
    FreeInstanceData();
    DestroyVerletSystem();
    g_ModelMgr.DeleteModel(hModelGraphics);
    g_ModelMgr.DeleteModel(hModelPhysical);
}

void RigidObj :: FreeInstanceData()
{
    modelInstanceGr.Clear();
    if (hModelGraphics == hModelPhysical)
        modelInstancePh.elementInstanceP = NULL;
    modelInstancePh.bonesM = NULL;
    modelInstancePh.bonesQ = NULL;
    modelInstancePh.bonesMod = NULL;
    modelInstancePh.Clear();
}

void RigidObj :: VerticesChanged(bool free)
{
    if (free) {
        UpdatePointers();
        renderer.FreeGraphics(*xModelGr, modelInstanceGr);
        renderer.FreeGraphics(*xModelPh, modelInstancePh);
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

void RigidObj :: CalculateSkeleton()
{
    if (!xModelGr->spine.I_bones) return;
    if (xModelGr->spine.I_bones != modelInstanceGr.bonesC)
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

void RigidObj :: CreateVerletSystem()
{
    verletSystem.Free();
    verletSystem.Init(4);
    verletSystem.C_constraints = new VConstraint*[6];
    verletSystem.I_constraints = 6;
    verletSystem.C_collisions = &collisionConstraints;
    verletSystem.C_lengthConst = NULL;

    VConstraintLengthEql *constr;
    verletSystem.C_constraints[0] = constr = new VConstraintLengthEql();
    constr->particleA = 0;
    constr->particleB = 1;
    constr->restLength = constr->restLengthSqr = 1;
    
    verletSystem.C_constraints[1] = constr = new VConstraintLengthEql();
    constr->particleA = 0;
    constr->particleB = 2;
    constr->restLength = constr->restLengthSqr = 1;

    verletSystem.C_constraints[2] = constr = new VConstraintLengthEql();
    constr->particleA = 0;
    constr->particleB = 3;
    constr->restLength = constr->restLengthSqr = 1;

    xFLOAT sqrt2 = sqrtf(2.f);
    verletSystem.C_constraints[3] = constr = new VConstraintLengthEql();
    constr->particleA = 1;
    constr->particleB = 2;
    constr->restLengthSqr = 2;
    constr->restLength    = sqrt2;

    verletSystem.C_constraints[4] = constr = new VConstraintLengthEql();
    constr->particleA = 1;
    constr->particleB = 3;
    constr->restLengthSqr = 2;
    constr->restLength    = sqrt2;

    verletSystem.C_constraints[5] = constr = new VConstraintLengthEql();
    constr->particleA = 2;
    constr->particleB = 3;
    constr->restLengthSqr = 2;
    constr->restLength    = sqrt2;

    verletSystem.P_current[0] = verletSystem.P_previous[0] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(0,0,0) );
    verletSystem.P_current[1] = verletSystem.P_previous[1] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(1,0,0) );
    verletSystem.P_current[2] = verletSystem.P_previous[2] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(0,1,0) );
    verletSystem.P_current[3] = verletSystem.P_previous[3] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(0,0,1) );
    
    verletSystem.M_weight_Inv[0] = verletSystem.M_weight_Inv[1] =
        verletSystem.M_weight_Inv[2] = verletSystem.M_weight_Inv[3] = 4.f / mass;
    
    verletSystem.A_forces[0].zero();
    verletSystem.A_forces[1].zero();
    verletSystem.A_forces[2].zero();
    verletSystem.A_forces[3].zero();
}

void RigidObj :: DestroyVerletSystem()
{
    if (verletSystem.C_constraints)
        delete[] verletSystem.C_constraints;
    verletSystem.Free();
}

void RigidObj :: UpdateVerletSystem()
{
    verletSystem.P_previous[0] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(0,0,0) );
    verletSystem.P_previous[1] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(1,0,0) );
    verletSystem.P_previous[2] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(0,1,0) );
    verletSystem.P_previous[3] = MX_ModelToWorld.preTransformP( modelInstanceGr.center + xVector3::Create(0,0,1) );
    verletSystem.SwapPositions();
}
    



void RigidObj:: PreUpdate(float deltaTime)
{
    RigidBody::CalculateCollisions(this, deltaTime);
}

void RigidObj:: Update(float deltaTime)
{
    RigidBody::CalculateMovement(this, deltaTime);
}
    
xCollisionHierarchyBoundsRoot *RigidObj::GetCollisionInfo()
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

void RigidObj::CollisionInfo_ReFill()
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

void RigidObj::CollisionInfo_Fill(xElement *elem, xCollisionHierarchyBoundsRoot *ci, bool firstTime)
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
    xMatrix   MX_ElementToWorld = elem->matrix * MX_ModelToWorld;
    for (int i = eci.verticesC; i; --i, ++iterS, ++iterD)
        iterD->init(MX_ElementToWorld.preTransformP(iterS->vector3), 1.f);

    // Fill collision bounding boxes
    xElement_CalcCollisionHierarchyBox(eci.verticesP, &elem->collisionData, &eci);

    // Fill CI for subelements
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Fill(celem, ci, firstTime);
}

void RigidObj::CollisionInfo_Free(xElement *elem, xCollisionHierarchyBoundsRoot *ci)
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

void RigidObj::CollisionInfo_Render(xElement *elem, xCollisionHierarchyBoundsRoot *ci)
{
    if (elem->verticesC)
        for (int i=0; i<elem->collisionData.kidsC; ++i)
            RenderCI(elem->collisionData.kidsP + i, ci[elem->id].kids + i);
    for (xElement *celem = elem->kidsP; celem; celem = celem->nextP)
        CollisionInfo_Render(celem, ci);
}

void RigidObj :: GetShadowProjectionMatrix(xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width)
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
    xMatrix MX_WorldToView;
    MX_WorldToView.row0.vector3 = lRAxis; MX_WorldToView.row0.w = 0.f;
    MX_WorldToView.row1.vector3 = lUAxis; MX_WorldToView.row1.w = 0.f;
    MX_WorldToView.row2.vector3 = lFAxis; MX_WorldToView.row2.w = 0.f;
    MX_WorldToView.row3.zeroQ();
    MX_WorldToView.postTranslate(-light->position).transpose();
    
    // Find the horizontal and vertical angular spreads to find out FOV and aspect ratio
    float RxMax = 0.f, RyMax = 0.f;
    xCollisionHierarchyBoundsRoot* ci = GetCollisionInfo();
    for (int i=collisionInfoC; i; --i, ++ci)
    {
        xVector4 *iter = ci->verticesP;
        for (int j=ci->verticesC; j; --j, ++iter)
        {
            xVector4 v = MX_WorldToView * *iter;
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
    mtxBlockerToLight.preMultiply(MX_WorldToView).preMultiply(MX_ModelToWorld);
    
    // Projection matrix for computing UVs on the receiver object
    mtxReceiverUVMatrix.row0.init(XProj*0.5f, 0.f, 0.f, 0.f);
    mtxReceiverUVMatrix.row1.init(0.f, YProj*0.5f, 0.f, 0.f);
    mtxReceiverUVMatrix.row2.init(-0.5f, -0.5f, (ZFar+ZNear)/(ZNear-ZFar), -1.f);
    mtxReceiverUVMatrix.row3.init(0.f, 0.f, 2.f*ZFar*ZNear/(ZNear-ZFar), 0.f);
    mtxReceiverUVMatrix.preMultiply(MX_WorldToView);
}