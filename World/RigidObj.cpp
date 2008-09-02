#include "RigidObj.h"
#include "../Models/lib3dx/xUtils.h"
#include "../AppFramework/System.h"
#include "../Math/Figures/xCapsule.h"
#include "ObjectTypes.h"
#include "../Utils/Filesystem.h"

void RigidObj :: ApplyDefaults()
{
    PhysicalBody::ApplyDefaults();

    FL_shadowcaster = false;
    FL_stationary   = true;
    FL_physical     = false;
    FL_customBVH    = true;
    M_mass          = 0.f;
    S_radius        = 0.f;

    Name.clear();
    fastModelFile.clear();
    modelFile.clear();
}

void RigidObj :: Initialize ()
{
    PhysicalBody::Initialize();

    if (M_mass == 0.f)
        M_mass = BVHierarchy.Figure->W_Volume_Get();
    if (S_radius == 0.f)
    {
        if (BVHierarchy.Figure->Type != xIFigure3d::Capsule)
            S_radius = sqrt(BVHierarchy.Figure->S_Radius_Sqr_Get());
        else
            S_radius = 0.75 * ((Math::Figures::xCapsule*) BVHierarchy.Figure)->S_radius
                     + 0.25 * ((Math::Figures::xCapsule*) BVHierarchy.Figure)->S_top;
    }

    Type = FL_stationary ? AI::ObjectType::Structure : AI::ObjectType::Physical;
}

void RigidObj :: Initialize (const char *gr_filename, const char *ph_filename)
{
    UpdateMatrices();

    HModel hModelGr = g_ModelMgr.GetModel(gr_filename);
    ModelGr = new ModelInstance(hModelGr);
    ModelPh = NULL;
    if (ph_filename)
    {
        HModel hModelPh = g_ModelMgr.GetModel(ph_filename);
        if (hModelPh != hModelGr)
            ModelPh = new ModelInstance(hModelPh);
        else
            g_ModelMgr.DeleteModel(hModelPh);
    }
    FL_renderNeedsUpdate      = false;
    FL_renderNeedsUpdateBones = false;

    if (!IsDefaultsApplied()) ApplyDefaults();

    MeshData = NULL;
    if (FL_customBVH)
    {
        if (ModelGr->xModelP->BVHierarchy)
            ModelGr->xModelP->BVHierarchy->Clone(BVHierarchy);
        else
        if (ModelPh && ModelPh->xModelP->BVHierarchy)
            ModelPh->xModelP->BVHierarchy->Clone(BVHierarchy);
        else
            FL_customBVH = false;
    }
    if (!FL_customBVH)
    {
        ModelPh_Get().xModelP->CreateBVH(BVHierarchy, MeshData);
        P_center = BVHierarchy.Figure->P_center;
    }

    VerticesChanged(true, false);

    smap.texId = 0;

    Initialize();
}

void RigidObj :: Finalize ()
{
    if (ModelGr) { delete ModelGr; ModelGr = NULL; }
    if (ModelPh)
    {
        ModelPh->instance.MX_bones = NULL;
        ModelPh->instance.QT_bones = NULL;
        ModelPh->instance.P_bone_roots = NULL;
        ModelPh->instance.P_bone_trans = NULL;
        ModelPh->instance.FL_modified = NULL;
        delete ModelPh;
        ModelPh = NULL;
    }

    PhysicalBody::Finalize();

    if (MeshData)
        delete[] MeshData;
}



void RigidObj :: UpdateCustomBVH()
{
    if (!FL_customBVH || !BVHierarchy.Figure) return;

    BVHierarchy.invalidateTransformation();

    Math::Figures::xSphere &sphere   = *(Math::Figures::xSphere*) BVHierarchy.Figure;
    Math::Figures::xSphere &sphere_T = *(Math::Figures::xSphere*) BVHierarchy.GetTransformed(MX_LocalToWorld_Get());

    if (ModelGr->instance.MX_bones)
    {
        for (int i = 0; i < BVHierarchy.I_items; ++i)
            BVHierarchy.L_items[i].MX_RawToLocal_Set(xMatrix::Transpose( ModelGr->instance.MX_bones[BVHierarchy.L_items[i].ID_Bone] ));
        Math::Figures::xBoxA boxA = BVHierarchy.childBounds(MX_LocalToWorld_Get());
        sphere.S_radius = sphere_T.S_radius = (boxA.P_max - boxA.P_min).length() * 0.5f;
        sphere_T.P_center = (boxA.P_max + boxA.P_min) * 0.5f;
        sphere.P_center = MX_WorldToLocal.preTransformP(sphere_T.P_center);
    }
    else
    {
        for (int i = 0; i < BVHierarchy.I_items; ++i)
            BVHierarchy.L_items[i].MX_RawToLocal_Set( xMatrix::Identity() );
        Math::Figures::xBoxA boxA = BVHierarchy.childBounds();
        sphere.S_radius = sphere_T.S_radius = (boxA.P_max - boxA.P_min).length() * 0.5f;
        sphere.P_center = (boxA.P_max + boxA.P_min) * 0.5f;
        sphere_T.P_center = MX_LocalToWorld_Get().preTransformP(sphere.P_center);
    }

    P_center = sphere.P_center;
    S_radius = sphere.S_radius;
}

void RigidObj :: UpdateGeneratedBVH()
{
    if (FL_customBVH || !BVHierarchy.Figure) return;

    BVHierarchy.invalidateTransformation();

    xModelInstance &minstance = ModelPh_Get().instance;

    for (int i = minstance.I_elements-1; i >= 0 ; --i)
    {
        MeshData[i].MX_Bones = minstance.MX_bones;
        MeshData[i].I_Bones  = minstance.I_bones;
        MeshData[i].Transform(MX_LocalToWorld_Get());
    }

    ModelPh_Get().xModelP->ReFillBVH(BVHierarchy, MeshData, MX_LocalToWorld_Get());

    S_radius      = ((xSphere*)BVHierarchy.GetTransformed())->S_radius;
    P_center_Trfm = BVHierarchy.GetTransformed()->P_center;
    P_center      = MX_WorldToLocal.preTransformP(P_center_Trfm);
}

void RigidObj :: VerticesChanged(bool init, bool free)
{
    if (free)
    {
        FL_renderNeedsUpdate = true;
        ModelGr->instance.FreeVertices();
        if (ModelPh)
        {
            ModelPh->instance.MX_bones = NULL;
            ModelPh->instance.QT_bones = NULL;
            ModelPh->instance.P_bone_roots = NULL;
            ModelPh->instance.P_bone_trans = NULL;
            ModelPh->instance.FL_modified = NULL;
            ModelPh->instance.FreeVertices();
        }
    }
    else
    if (!init)
        InvalidateShadowData();

    xBoneCalculateMatrices (ModelGr->xModelP->Spine, ModelGr->instance);
    xBoneCalculateQuats    (ModelGr->xModelP->Spine, ModelGr->instance);
    if (ModelPh)
    {
        ModelPh->instance.MX_bones     = ModelGr->instance.MX_bones;
        ModelPh->instance.QT_bones     = ModelGr->instance.QT_bones;
        ModelPh->instance.P_bone_roots = ModelGr->instance.P_bone_roots;
        ModelPh->instance.P_bone_trans = ModelGr->instance.P_bone_trans;
        ModelPh->instance.FL_modified  = ModelGr->instance.FL_modified;
        ModelPh->instance.I_bones      = ModelGr->instance.I_bones;
    }

    float delta = GetTick();

    // bounds used for rendering
    if (ModelGr->xModelP->Spine.I_bones) xModel_SkinElementInstance(*ModelGr->xModelP, ModelGr->instance);
    xModel_GetBounds(*ModelGr->xModelP, ModelGr->instance);

    Performance.CollisionDataFillMS += GetTick() - delta;
    delta = GetTick();

    if (FL_customBVH)
        UpdateCustomBVH();
    else
        UpdateGeneratedBVH();

    Performance.CollisionDeterminationMS += GetTick() - delta;
}

void RigidObj :: CalculateSkeleton()
{
    if (!ModelGr->xModelP->Spine.I_bones) return;
    if (ModelGr->xModelP->Spine.I_bones != ModelGr->instance.I_bones)
    {
        if (!ModelGr->instance.I_bones) // skeleton has been added, so refresh VBO data
        {
            VerticesChanged(false, true);
            return;
        }
        else
            ModelGr->instance.ClearSkeleton();
    }
    VerticesChanged(false, false);
    FL_renderNeedsUpdateBones = true;
}


void RigidObj :: LoadLine(char *buffer, std::string &dir)
{
    if (StartsWith(buffer, "name"))
    {
        char file[255];
        sscanf(buffer+4, "%s", file);
        Name = file;
        return;
    }
    if (StartsWith(buffer, "fastm"))
    {
        char file[255];
        sscanf(buffer+5, "%s", file);
        fastModelFile = Filesystem::GetFullPath(dir + "/" + file);
        return;
    }
    if (StartsWith(buffer, "model"))
    {
        char file[255];
        sscanf(buffer+5, "%s", file);
        modelFile = Filesystem::GetFullPath(dir + "/" + file);
        return;
    }
    if (StartsWith(buffer, "customBVH"))
    {
        int customBVH;
        sscanf(buffer+9, "%d", &customBVH);
        FL_customBVH = customBVH;
        return;
    }
    if (StartsWith(buffer, "position"))
    {
        float x,y,z;
        sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
        Translate(x, y, z);
        return;
    }
    if (StartsWith(buffer, "rotation"))
    {
        float x,y,z;
        sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
        Rotate(x, y, z);
        return;
    }
    if (StartsWith(buffer, "velocity"))
    {
        float x,y,z;
        sscanf(buffer+8, "%f\t%f\t%f", &x,&y,&z);
        ApplyAcceleration(xVector3::Create(x,y,z), 1.f);
        return;
    }
    if (StartsWith(buffer, "physical"))
    {
        int b;
        sscanf(buffer+8, "%d", &b);
        FL_physical = b;
        return;
    }
    if (StartsWith(buffer, "locked"))
    {
        int b;
        sscanf(buffer+6, "%d", &b);
        FL_stationary = b;
        return;
    }
    if (StartsWith(buffer, "phantom"))
    {
        int b;
        sscanf(buffer+7, "%d", &b);
        FL_phantom = b;
        return;
    }
    if (StartsWith(buffer, "mass"))
    {
        float mass;
        sscanf(buffer+4, "%f", &mass);
        M_mass = mass;
        return;
    }
    if (StartsWith(buffer, "restitution"))
    {
        float restitution;
        sscanf(buffer+11, "%f", &restitution);
        W_restitution = restitution;
        return;
    }
    if (StartsWith(buffer, "restitution_self"))
    {
        float restitution;
        sscanf(buffer+16, "%f", &restitution);
        W_restitution_self = restitution;
        return;
    }
    if (StartsWith(buffer, "shadows"))
    {
        int b;
        sscanf(buffer+7, "%d", &b);
        FL_shadowcaster = b;
        return;
    }
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

    xModelInstance &instance =  ModelPh_Get().instance;
    xModel         &model    = *ModelPh_Get().xModelP;
    xMatrix MX_LocalToView = MX_LocalToWorld_Get() * MX_WorldToView;

    // Find the horizontal and vertical angular spreads to find out FOV and aspect ratio
    float RxMax = 0.f, RyMax = 0.f;
    for (int ie = 0; ie < instance.I_elements; ++ie)
    {
        xElement &elem = *model.L_kids->ById(ie);
        xMatrix MX_MeshToView = elem.MX_MeshToLocal * MX_LocalToView;

        if (instance.L_elements)
        {
            xPoint4 *iter = instance.L_elements[ie].L_vertices;
            for (int iv = instance.L_elements[ie].I_vertices; iv; --iv, ++iter)
            {
                xPoint4 v = MX_MeshToView * *iter;
                v.z = 1.f / v.z;
                v.x *= v.z; v.y *= v.z;

                if (fabs(v.x) > RxMax) RxMax = fabs(v.x);
                if (fabs(v.y) > RyMax) RyMax = fabs(v.y);
            }
        }
        else
        {
            size_t    stride = elem.GetVertexStride();
            xBYTE    *srcV   = (xBYTE *) elem.L_vertices,
                     *lastV  = (xBYTE *) elem.L_vertices + stride*elem.I_vertices;
            for (; srcV != lastV; srcV += stride)
            {
                xPoint3 v = MX_MeshToView.postTransformP(*(xPoint3 *)srcV);
                v.z = 1.f / v.z;
                v.x *= v.z; v.y *= v.z;
                if (fabs(v.x) > RxMax) RxMax = fabs(v.x);
                if (fabs(v.y) > RyMax) RyMax = fabs(v.y);
            }
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
    mtxBlockerToLight.preMultiply(MX_LocalToView);

    // Projection matrix for computing UVs on the receiver object
    mtxReceiverUVMatrix.row0.init(XProj*0.5f, 0.f, 0.f, 0.f);
    mtxReceiverUVMatrix.row1.init(0.f, YProj*0.5f, 0.f, 0.f);
    mtxReceiverUVMatrix.row2.init(-0.5f, -0.5f, (ZFar+ZNear)/(ZNear-ZFar), -1.f);
    mtxReceiverUVMatrix.row3.init(0.f, 0.f, 2.f*ZFar*ZNear/(ZNear-ZFar), 0.f);
    mtxReceiverUVMatrix.preMultiply(MX_WorldToView);
}
