#include "RigidObj.h"
#include "../AppFramework/System.h"
#include "../Physics/RigidBody.h"
#include "../Math/Figures/xCapsule.h"

void RigidObj :: ApplyDefaults()
{
    PhysicalBody::ApplyDefaults();

    M_mass       = 0.f;
    S_radius     = 0.f;
}

void RigidObj :: Initialize ()
{
    PhysicalBody::Initialize();

    if (M_mass == 0.f)
        M_mass = BVHierarchy.Figure->W_Volume_Get();
    if (S_radius == 0.f)
    {
        if (BVHierarchy.Figure->Type != xIFigure3d::Capsule)
            S_radius = 2.f * sqrt(BVHierarchy.Figure->S_Radius_Sqr_Get());
        else
            S_radius = 0.75 * ((Math::Figures::xCapsule*) BVHierarchy.Figure)->S_radius
                     + 0.25 * ((Math::Figures::xCapsule*) BVHierarchy.Figure)->S_top;
    }
    P_center = BVHierarchy.Figure->P_center;

    Type = Model_Rigid;
    UpdateMatrices();
}

void RigidObj :: Initialize (const char *gr_filename, const char *ph_filename, bool physicalNotLocked, bool phantom)
{
    modelInstanceGr.Zero();
    modelInstancePh.Zero();
    modelInstanceGr.MX_LocalToWorld = MX_LocalToWorld_Get();
    modelInstancePh.MX_LocalToWorld = MX_LocalToWorld_Get();

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

    xModelPh->CreateBVH(BVHierarchy);

    if (!IsDefaultsApplied()) ApplyDefaults();

    smap.texId = 0;
    collisionInfo = NULL;
    //renderer.UseVBO = forceNotStatic;

    this->FL_phantom    = phantom;
    this->FL_physical   = physicalNotLocked;
    this->FL_stationary = !physicalNotLocked;

    Initialize();

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
        CollisionInfo_Free(GetModelPh()->L_kids, collisionInfo);
        delete[] collisionInfo;
        collisionInfo = NULL;
    }
    FreeInstanceData();
    DestroyVerletSystem();
    g_ModelMgr.DeleteModel(hModelGraphics);
    g_ModelMgr.DeleteModel(hModelPhysical);

    BVHierarchy.free();
}

void RigidObj :: FreeInstanceData()
{
    modelInstanceGr.Clear();
    if (hModelGraphics == hModelPhysical)
        modelInstancePh.L_elements = NULL;
    modelInstancePh.MX_bones = NULL;
    modelInstancePh.QT_bones = NULL;
    modelInstancePh.FL_modified = NULL;
    modelInstancePh.Clear();
}

void RigidObj :: VerticesChanged(bool free)
{
    if (free) {
        UpdatePointers();
        renderer.FreeGraphics(*xModelGr, modelInstanceGr);
        renderer.FreeGraphics(*xModelPh, modelInstancePh);
        FreeInstanceData();

        modelInstanceGr.I_elements = xModelGr->I_elements;
        modelInstanceGr.L_elements = new xElementInstance[xModelGr->I_elements];
        modelInstanceGr.ZeroElements();
        //memset(modelInstanceGr.L_elements, 0, modelInstanceGr.I_elements * sizeof(xElementInstance));
        if (hModelGraphics != hModelPhysical)
        {
            modelInstancePh.I_elements = xModelPh->I_elements;
            modelInstancePh.L_elements = new xElementInstance[xModelPh->I_elements];
            modelInstancePh.ZeroElements();
            //memset(modelInstancePh.L_elements, 0, modelInstancePh.I_elements * sizeof(xElementInstance));
        }
        else
        {
            modelInstancePh.I_elements = modelInstanceGr.I_elements;
            modelInstancePh.L_elements = modelInstanceGr.L_elements;
        }
    }
    else InvalidateShadowRenderData();

    xBoneCalculateMatrices (xModelGr->Spine, &modelInstanceGr);
    xBoneCalculateQuats    (xModelGr->Spine, &modelInstanceGr);
    modelInstancePh.MX_bones   = modelInstanceGr.MX_bones;
    modelInstancePh.QT_bones   = modelInstanceGr.QT_bones;
    modelInstancePh.FL_modified = modelInstanceGr.FL_modified;
    modelInstancePh.I_bones   = modelInstanceGr.I_bones;

    xModel_SkinElementInstance(xModelGr, modelInstanceGr);
    modelInstancePh.P_center = modelInstanceGr.P_center = xModel_GetBounds(modelInstanceGr);
    if (hModelGraphics != hModelPhysical)
    {
        xModel_SkinElementInstance(xModelPh, modelInstancePh);
        modelInstancePh.P_center = xModel_GetBounds(modelInstancePh);
    }
}

void RigidObj :: CalculateSkeleton()
{
    if (!xModelGr->Spine.I_bones) return;
    if (xModelGr->Spine.I_bones != modelInstanceGr.I_bones)
    {
        modelInstanceGr.ClearSkeleton();
        if (!modelInstanceGr.I_bones) // skeleton was added, so refresh VBO data
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

    verletSystem.P_current[0] = verletSystem.P_previous[0] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(0,0,0) );
    verletSystem.P_current[1] = verletSystem.P_previous[1] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(1,0,0) );
    verletSystem.P_current[2] = verletSystem.P_previous[2] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(0,1,0) );
    verletSystem.P_current[3] = verletSystem.P_previous[3] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(0,0,1) );

    verletSystem.M_weight_Inv[0] = verletSystem.M_weight_Inv[1] =
        verletSystem.M_weight_Inv[2] = verletSystem.M_weight_Inv[3] = 4.f / M_mass;

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
    verletSystem.P_previous[0] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(0,0,0) );
    verletSystem.P_previous[1] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(1,0,0) );
    verletSystem.P_previous[2] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(0,1,0) );
    verletSystem.P_previous[3] = MX_LocalToWorld_Get().preTransformP( modelInstanceGr.P_center + xVector3::Create(0,0,1) );
    //verletSystem.SwapPositions();
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
    collisionInfoC = xModelPh->I_elements;
    collisionInfo  = new xCollisionHierarchyBoundsRoot[collisionInfoC];

    for (xElement *elem = xModelPh->L_kids; elem; elem = elem->Next)
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
        for (xElement *elem = xModelPh->L_kids; elem; elem = elem->Next)
            CollisionInfo_Fill(elem, collisionInfo, false);
        Performance.CollisionDataFillMS += GetTick() - delta;
    }
}

void RigidObj::CollisionInfo_Fill(xElement *elem, xCollisionHierarchyBoundsRoot *ci, bool firstTime)
{
    xCollisionHierarchyBoundsRoot &eci = ci[elem->ID];
    UpdatePointers();

    // Create Hierarchy, if not exists
    if (firstTime)
    {
        if (!elem->collisionData.L_kids)
            elem->collisionData.Fill(xModelPh, elem);
        eci.kids = NULL;
        eci.L_vertices = NULL;
    }

    if (eci.I_vertices != modelInstancePh.L_elements[elem->ID].I_vertices)
    {
        eci.I_vertices = modelInstancePh.L_elements[elem->ID].I_vertices;
        if (eci.L_vertices) delete[] eci.L_vertices;
        eci.L_vertices = new xVector4[eci.I_vertices];
    }

    xVector4 *iterS = modelInstancePh.L_elements[elem->ID].L_vertices, *iterD = eci.L_vertices;
    xMatrix   MX_ElementToWorld = elem->MX_MeshToLocal * MX_LocalToWorld_Get();
    for (int i = eci.I_vertices; i; --i, ++iterS, ++iterD)
        iterD->init(MX_ElementToWorld.preTransformP(iterS->vector3), 1.f);

    // Fill collision bounding boxes
    xElement_CalcCollisionHierarchyBox(eci.L_vertices, &elem->collisionData, &eci);

    // Fill CI for subelements
    for (xElement *celem = elem->L_kids; celem; celem = celem->Next)
        CollisionInfo_Fill(celem, ci, firstTime);
}

void RigidObj::CollisionInfo_Free(xElement *elem, xCollisionHierarchyBoundsRoot *ci)
{
    if (!elem) return;
    xElement_FreeCollisionHierarchyBounds(&elem->collisionData, ci[elem->ID].kids);
    for (xElement *celem = elem->L_kids; celem; celem = celem->Next)
        CollisionInfo_Free(celem, ci);
}

void RenderCI(xCollisionHierarchy *ch, xCollisionHierarchyBounds *chb)
{
    if (ch->I_kids)
    {
        for (int i=0; i<ch->I_kids; ++i)
            RenderCI(ch->L_kids + i, chb->kids + i);
    }
    else
    {
        glBegin(GL_QUAD_STRIP);
        {
            glVertex3f(chb->bounding.P_max.x, chb->bounding.P_max.y, chb->bounding.P_max.z);
            glVertex3f(chb->bounding.P_max.x, chb->bounding.P_max.y, chb->bounding.P_min.z);
            glVertex3f(chb->bounding.P_max.x, chb->bounding.P_min.y, chb->bounding.P_max.z);
            glVertex3f(chb->bounding.P_max.x, chb->bounding.P_min.y, chb->bounding.P_min.z);

            glVertex3f(chb->bounding.P_min.x, chb->bounding.P_min.y, chb->bounding.P_max.z);
            glVertex3f(chb->bounding.P_min.x, chb->bounding.P_min.y, chb->bounding.P_min.z);

            glVertex3f(chb->bounding.P_min.x, chb->bounding.P_max.y, chb->bounding.P_max.z);
            glVertex3f(chb->bounding.P_min.x, chb->bounding.P_max.y, chb->bounding.P_min.z);

            glVertex3f(chb->bounding.P_max.x, chb->bounding.P_max.y, chb->bounding.P_max.z);
            glVertex3f(chb->bounding.P_max.x, chb->bounding.P_max.y, chb->bounding.P_min.z);
        }
        glEnd();
    }
}

void RigidObj::CollisionInfo_Render(xElement *elem, xCollisionHierarchyBoundsRoot *ci)
{
    if (elem->I_vertices)
        for (int i=0; i<elem->collisionData.I_kids; ++i)
            RenderCI(elem->collisionData.L_kids + i, ci[elem->ID].kids + i);
    for (xElement *celem = elem->L_kids; celem; celem = celem->Next)
        CollisionInfo_Render(celem, ci);
}

void RigidObj :: GetShadowProjectionMatrix(xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width)
{
    xVector3 centerOfTheMassG = (xVector4::Create(modelInstancePh.P_center, 1.f)*modelInstancePh.MX_LocalToWorld).vector3;
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
    MX_WorldToView.row3.init(0.f,0.f,0.f,1.f);
    MX_WorldToView.postTranslate(-light->position).transpose();

    // Find the horizontal and vertical angular spreads to find out FOV and aspect ratio
    float RxMax = 0.f, RyMax = 0.f;
    xCollisionHierarchyBoundsRoot* ci = GetCollisionInfo();
    for (int i=collisionInfoC; i; --i, ++ci)
    {
        xVector4 *iter = ci->L_vertices;
        for (int j=ci->I_vertices; j; --j, ++iter)
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
    mtxBlockerToLight.preMultiply(MX_WorldToView).preMultiply(MX_LocalToWorld_Get());

    // Projection matrix for computing UVs on the receiver object
    mtxReceiverUVMatrix.row0.init(XProj*0.5f, 0.f, 0.f, 0.f);
    mtxReceiverUVMatrix.row1.init(0.f, YProj*0.5f, 0.f, 0.f);
    mtxReceiverUVMatrix.row2.init(-0.5f, -0.5f, (ZFar+ZNear)/(ZNear-ZFar), -1.f);
    mtxReceiverUVMatrix.row3.init(0.f, 0.f, 2.f*ZFar*ZNear/(ZNear-ZFar), 0.f);
    mtxReceiverUVMatrix.preMultiply(MX_WorldToView);
}
