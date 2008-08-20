#include "RigidObj.h"
#include "../Models/lib3dx/xUtils.h"
#include "../AppFramework/System.h"
#include "../Math/Figures/xCapsule.h"

void RigidObj :: ApplyDefaults()
{
    PhysicalBody::ApplyDefaults();

    FL_stationary   = true;
    FL_physical     = false;
    FL_customBVH    = true;
    M_mass          = 0.f;
    S_radius        = 0.f;
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

    Type = Model_Rigid;
    UpdateMatrices();
}

void RigidObj :: Initialize (const char *gr_filename, const char *ph_filename)
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

    if (!IsDefaultsApplied()) ApplyDefaults();

    MeshData = NULL;
    if (FL_customBVH)
    {
        if (xModelGr->BVHierarchy)
            xModelGr->BVHierarchy->Clone(BVHierarchy);
        else
        if (xModelPh->BVHierarchy)
            xModelPh->BVHierarchy->Clone(BVHierarchy);
        else
            FL_customBVH = false;
    }
    if (!FL_customBVH)
    {
        xModelPh->CreateBVH(BVHierarchy, MeshData);
        P_center = BVHierarchy.Figure->P_center;
    }
    else
        UpdateCustomBVH();

    smap.texId = 0;
    //renderer.UseVBO = forceNotStatic;

    //this->FL_phantom    = phantom;
    //this->FL_physical   = physicalNotLocked;
    //this->FL_stationary = !physicalNotLocked;

    Initialize();

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
    FreeInstanceData();
    g_ModelMgr.DeleteModel(hModelGraphics);
    g_ModelMgr.DeleteModel(hModelPhysical);

    PhysicalBody::Finalize();

    if (MeshData)
        delete[] MeshData;
}



void RigidObj :: UpdateCustomBVH()
{
    if (!FL_customBVH || !BVHierarchy.Figure) return;

    BVHierarchy.invalidateTransformation();

    if (modelInstanceGr.MX_bones)
        for (int i = 0; i < BVHierarchy.I_items; ++i)
            BVHierarchy.L_items[i].MX_LocalToFig_Set(xMatrix::Transpose( modelInstanceGr.MX_bones[BVHierarchy.L_items[i].ID_Bone] ));
    else
        for (int i = 0; i < BVHierarchy.I_items; ++i)
            BVHierarchy.L_items[i].MX_LocalToFig_Set(xMatrix::Transpose( xMatrix::Identity() ));

    Math::Figures::xBoxA box = BVHierarchy.childBounds(MX_LocalToWorld_Get());
    Math::Figures::xSphere &sphere   = *(Math::Figures::xSphere*) BVHierarchy.Figure;
    Math::Figures::xSphere &sphere_T = *(Math::Figures::xSphere*) BVHierarchy.GetTransformed(MX_LocalToWorld_Get());
    sphere.S_radius = sphere_T.S_radius = (box.P_max - box.P_min).length() * 0.5f;
    sphere_T.P_center = (box.P_max + box.P_min) * 0.5f;
    sphere.P_center = MX_WorldToLocal.preTransformP(sphere_T.P_center);

    P_center = sphere.P_center;
}

void RigidObj :: UpdateGeneratedBVH()
{
    if (FL_customBVH || !BVHierarchy.Figure) return;

    BVHierarchy.invalidateTransformation();

    for (int i = 0; i < modelInstancePh.I_elements; ++i)
    {
        MeshData[i].MX_Bones = modelInstancePh.MX_bones;
        MeshData[i].I_Bones  = modelInstancePh.I_bones;
    }

    for (int i = 0; i < modelInstancePh.I_elements; ++i)
        MeshData[i].Transform(MX_LocalToWorld_Get());

    GetModelPh()->ReFillBVH(BVHierarchy, MeshData, MX_LocalToWorld_Get());

    P_center_Trfm = BVHierarchy.GetTransformed()->P_center;
    P_center      = MX_WorldToLocal.preTransformP(P_center_Trfm);
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
    modelInstancePh.MX_bones    = modelInstanceGr.MX_bones;
    modelInstancePh.QT_bones    = modelInstanceGr.QT_bones;
    modelInstancePh.FL_modified = modelInstanceGr.FL_modified;
    modelInstancePh.I_bones     = modelInstanceGr.I_bones;

    if (false && free)
    {
        xModel_SkinElementInstance(xModelGr, modelInstanceGr);
        modelInstancePh.P_center = modelInstanceGr.P_center = xModel_GetBounds(modelInstanceGr);
        if (hModelGraphics != hModelPhysical)
        {
            xModel_SkinElementInstance(xModelPh, modelInstancePh);
            modelInstancePh.P_center = xModel_GetBounds(modelInstancePh);
        }
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




void RigidObj :: GetShadowProjectionMatrix(xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width)
{
    xVector3 centerOfTheMassG = (xVector4::Create(P_center, 1.f)*MX_LocalToWorld_Get()).vector3;
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

    xMatrix MX_ModelToView = modelInstancePh.MX_LocalToWorld * MX_WorldToView;

    // Find the horizontal and vertical angular spreads to find out FOV and aspect ratio
    float RxMax = 0.f, RyMax = 0.f;
    for (int ie = 0; ie < modelInstanceGr.I_elements; ++ie)
    {
        xMatrix MX_ElementToView = xModelPh->L_kids->ById(ie)->MX_MeshToLocal * MX_WorldToView;
        xVector4 *iter = modelInstancePh.L_elements[ie].L_vertices;
        for (int iv = modelInstancePh.L_elements[ie].I_vertices; iv; --iv, ++iter)
        {
            xVector4 v = MX_ElementToView * *iter;
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
