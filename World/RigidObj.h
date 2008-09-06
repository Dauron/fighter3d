#ifndef __incl_ModelObj_h
#define __incl_ModelObj_h

#include "../Physics/PhysicalBody.h"
#include "../Math/Figures/xSphere.h"

#include "../Models/ModelMgr.h"
#include "../Physics/Verlet/VerletSolver.h"
#include "../Graphics/Renderer.h"

#include <algorithm>

struct ModelInstance
{
    HModel         hModel;
    xModel        *xModelP;
    xModelInstance instance;

    ModelInstance(HModel hModel)
    {
        this->hModel = hModel;
        xModelP = g_ModelMgr.GetModel(hModel)->model;
        instance.Zero();
        instance.I_elements = xModelP->I_elements;
        instance.L_elements = new xElementInstance[xModelP->I_elements];
        instance.ZeroElements();
    }

    ~ModelInstance()
    { g_ModelMgr.Release(hModel); instance.Clear(); }

    void Update()
    { xModelP = g_ModelMgr.GetModel(hModel)->model; }

    int GetReferences()
    { return g_ModelMgr.GetModel(hModel)->CountReferences(); }
};

class RigidObj : public Physics::PhysicalBody
{
public:
    std::string Name;

    bool FL_shadowcaster;
    bool FL_customBVH;

    bool FL_renderNeedsUpdate;
    bool FL_renderNeedsUpdateBones;

    VConstraintCollisionVector  collisionConstraints;
    Math::Figures::xMeshData   *MeshData;

    xMatrix        MX_WorldToLocal;

protected:
    xShadowMap     smap;

    /******** LIFETIME : BEGIN ********/
public:
    virtual void LoadLine(char *buffer, std::string &dir);

    virtual void ApplyDefaults();
    virtual void Create  ();
    virtual void Destroy ();

    virtual void Create (const char *gr_filename, const char *ph_filename = NULL);
    /********* LIFETIME : END *********/

    /******** PHYSICS : BEGIN ********/
public:
    void UpdateMatrices()
    { xMatrix::Invert(MX_LocalToWorld_Get(), MX_WorldToLocal); }
    void UpdateCustomBVH();
    void UpdateGeneratedBVH();

    virtual void Render() {
        PhysicalBody::Render();
        if (ModelGr) ModelGr->instance.MX_LocalToWorld = MX_LocalToWorld_Get();
        if (ModelPh) ModelPh->instance.MX_LocalToWorld = MX_LocalToWorld_Get();
    }
    virtual void FrameEnd   () {
        if (IsModified()) UpdateMatrices();
        PhysicalBody::FrameEnd();
    }
protected:
    virtual void LocationChanged() {
        PhysicalBody::LocationChanged();
        UpdateMatrices();
        UpdateCustomBVH();
    }
    /********* PHYSICS : END *********/

    /******** RENDERING : BEGIN ********/
public:
    /*
    void CreateShadowMap(xLight *light)
    {
        xMatrix blocker;
        UpdatePointers();
        GetShadowProjectionMatrix(light, blocker, smap.receiverUVMatrix, Config::ShadowMapSize);
        renderer.CreateShadowMapTexture( *xModelPh, modelInstancePh, smap.texId,
                                          Config::ShadowMapSize, blocker);
    }
    */
    xShadowMap &GetShadowMap () { return smap; }
    void InvalidateShadowData()
    {
        if (ModelGr) InvalidateShadowData(ModelGr->instance);
        if (ModelPh) InvalidateShadowData(ModelPh->instance);
    }
private:
    void GetShadowProjectionMatrix (xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width);
    void InvalidateShadowData(xModelInstance &instance)
    {
        if (instance.L_elements)
        {
            xElementInstance *iter = instance.L_elements;
            for (int i = instance.I_elements; i; --i, ++iter)
                iter->InvalidateVertexData();
        }
    }
    /********* RENDERING : END *********/

public:
    std::string fastModelFile, modelFile;
    ModelInstance *ModelGr, *ModelPh;

    ModelInstance &ModelGr_Get() { return *ModelGr; }
    ModelInstance &ModelPh_Get() { return ModelPh ? *ModelPh : *ModelGr; }

    void UpdatePointers()
    {
        if (ModelGr) ModelGr->Update();
        if (ModelPh) ModelPh->Update();
    }
    void SwapModels()
    { if (ModelPh) { ModelInstance *swp = ModelPh; ModelPh = ModelGr; ModelGr = swp; } }

    virtual void VerticesChanged(bool init, bool free);
    virtual void CalculateSkeleton();

    static void CopySpine(const xSkeleton &src, xSkeleton &dst)
    {
        if (src.L_bones != dst.L_bones)
        {
            dst.Clear();
            dst = src.Clone();
        }
    }
};

#endif
