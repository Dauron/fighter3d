#ifndef __incl_ModelObj_h
#define __incl_ModelObj_h

#include "../Physics/PhysicalBody.h"

#include "../Models/ModelMgr.h"
#include "../Physics/CollisionInfo.h"
#include "../Physics/Verlet/VerletSolver.h"
#include "../Graphics/OGL/Render/RendererGL.h"

#include <algorithm>

class RigidObj : public Physics::PhysicalBody
{
public:
    enum ModelType
    {
        Model_Rigid,
        Model_Verlet
    } Type;

    bool           FL_shadowcaster;

    VConstraintCollisionVector collisionConstraints;
    VerletSystem               verletSystem;

    xMatrix        MX_WorldToLocal;
    xMatrix        MX_LocalToWorld_prev;

    RendererGL     renderer;
    xModelInstance modelInstanceGr;
    xModelInstance modelInstancePh;

    //xVector3       cp, com, cno, cro;
protected:
    bool           forceNotStatic;
    xShadowMap     smap;

    xCollisionHierarchyBoundsRoot *collisionInfo;
    xWORD                          collisionInfoC;
    std::vector<xDWORD> idx;

    /******** CONSTRUCTORS : BEGIN ********/
public:
    RigidObj () : FL_shadowcaster(false), forceNotStatic(false) {}
    /********* CONSTRUCTORS : END *********/

    /******** LIFETIME : BEGIN ********/
public:
    virtual void ApplyDefaults();
    virtual void Initialize ();
    virtual void Invalidate () {
        UpdatePointers();
        renderer.InvalidateGraphics(*xModelGr, modelInstanceGr);
        renderer.InvalidateGraphics(*xModelPh,modelInstancePh);
        smap.texId = 0;
    }
    virtual void FrameEnd   () {
        if (IsModified()) UpdateMatrices();
        PhysicalBody::FrameEnd();
    }
    virtual void Finalize   ();

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL, bool physicalNotLocked = false, bool phantom = true);
    /********* LIFETIME : END *********/

    /******** UPDATE : BEGIN ********/
public:
    virtual void PreUpdate(float deltaTime);
    virtual void Update(float deltaTime);
    void UpdateMatrices()
    {
        MX_LocalToWorld_prev = MX_LocalToWorld_Get();
        xMatrix::Invert(MX_LocalToWorld_Get(), MX_WorldToLocal);
    }
    /********* UPDATE : END *********/

    /******** PHYSICS : BEGIN ********/
public:
    xCollisionHierarchyBoundsRoot  *GetCollisionInfo();
    xWORD                           GetCollisionInfoC() { return collisionInfoC; }
    void                            CollisionInfo_ReFill ();
    std::vector<CollisionWithModel> CollidedModels;
protected:
    void CollisionInfo_Fill   (xElement *elem, xCollisionHierarchyBoundsRoot *ci, bool firstTime);
    void CollisionInfo_Free   (xElement *elem, xCollisionHierarchyBoundsRoot *ci);
    void CollisionInfo_Render (xElement *elem, xCollisionHierarchyBoundsRoot *ci);

    virtual void LocationChanged() {
        xMatrix::Invert(MX_LocalToWorld_Get(), MX_WorldToLocal);
        UpdateVerletSystem();
        verletSystem.SwapPositions();
        UpdateVerletSystem();
        memset(verletSystem.A_forces, 0, sizeof(xVector3)*verletSystem.I_particles);
        memset(verletSystem.NW_shift, 0, sizeof(xVector3)*verletSystem.I_particles);
    }

    virtual void CreateVerletSystem();
    virtual void DestroyVerletSystem();
    virtual void UpdateVerletSystem();
    /********* PHYSICS : END *********/

    /******** RENDERING : BEGIN ********/
public:
    void CreateShadowMap(xLight *light)
    {
        xMatrix blocker;
        UpdatePointers();
        GetShadowProjectionMatrix(light, blocker, smap.receiverUVMatrix, Config::ShadowMapSize);
        renderer.CreateShadowMapTexture( *xModelPh, modelInstancePh, smap.texId,
                                          Config::ShadowMapSize, blocker);
    }
    xShadowMap GetShadowMap () { return smap; }
    void RenderShadowMap (xShadowMap smap, const xFieldOfView &FOV)
    {
        UpdatePointers();
        renderer.RenderShadowMap(*xModelGr, modelInstanceGr, smap, FOV);
    }
    void RenderShadowVolume(xLight &light, xFieldOfView &FOV)
    {
        //UpdatePointers();
        renderer.RenderShadowVolume(*xModelGr, modelInstanceGr, light, FOV);
    }
    void RenderDepth(xFieldOfView &FOV, bool transparent)
    {
        UpdatePointers();
        renderer.RenderDepth(*xModelGr, modelInstanceGr, transparent, FOV);
    }
    void RenderAmbient(const xLightVector &lights, xFieldOfView &FOV, bool transparent)
    {
        //UpdatePointers();
        renderer.RenderAmbient(*xModelGr, modelInstanceGr, lights, transparent, FOV);
    }
    void RenderDiffuse(xLight &light, xFieldOfView &FOV, bool transparent)
    {
        //UpdatePointers();
        renderer.RenderDiffuse(*xModelGr, modelInstanceGr, light, transparent, FOV);
    }
    void Render(bool transparent, const xFieldOfView &FOV)
    {
        UpdatePointers();
        renderer.RenderModel(*xModelGr, modelInstanceGr, transparent, FOV);
    }
    void InvalidateShadowRenderData()
    {
        InvalidateShadowRenderData(modelInstanceGr);
        if (hModelGraphics != hModelPhysical)
            InvalidateShadowRenderData(modelInstancePh);
    }
private:
    void GetShadowProjectionMatrix (xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width);
    void InvalidateShadowRenderData(xModelInstance &instance)
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
    xModel *GetModelPh() { UpdatePointers(); return xModelPh; }
    xModel *GetModelGr() { UpdatePointers(); return xModelGr; }

    void CopySpineToPhysical()
    {
        CopySpine(xModelGr->Spine, xModelPh->Spine);
    }
    void CopySpineToGraphics()
    {
        CopySpine(xModelPh->Spine, xModelGr->Spine);
    }

    virtual void VerticesChanged(bool free);
    virtual void CalculateSkeleton();

private:
    HModel     hModelGraphics;
    HModel     hModelPhysical;
    xModel   * xModelGr;
    xModel   * xModelPh;

    void UpdatePointers()
    {
        xModelGr = g_ModelMgr.GetModel(hModelGraphics)->model;
        xModelPh = g_ModelMgr.GetModel(hModelPhysical)->model;
        modelInstancePh.MX_LocalToWorld = modelInstanceGr.MX_LocalToWorld = MX_LocalToWorld_Get();
    }

    void CopySpine(const xSkeleton &src, xSkeleton &dst)
    {
        if (src.L_bones != dst.L_bones)
        {
            dst.Clear();
            dst = src.Clone();
        }
    }

    void FreeInstanceData();
};

#endif
