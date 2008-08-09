#ifndef __incl_ModelObj_h
#define __incl_ModelObj_h

#include "BaseObj.h"
#include "../Models/ModelMgr.h"
#include "../Physics/CollisionInfo.h"
#include "../Physics/Verlet/VerletSolver.h"
#include "../Graphics/OGL/Render/RendererGL.h"

#include <algorithm>

struct xForce
{
    xVector3 point;
    xVector3 accel;
};

class RigidObj : public BaseObj
{
public:
    enum ModelType
    {
        Model_Rigid,
        Model_Verlet
    } Type;

    bool           castsShadows;
    bool           phantom;    // no collisions
    bool           locked;     // may not be moved?
    bool           physical;   // affected by gravity?
    float          mass;       // weight of an object
    float          resilience; // how much energy will the object retain during collisions

    VConstraintCollisionVector collisionConstraints;
    VerletSystem              verletSystem;

    xMatrix        MX_WorldToModel;
    xMatrix        MX_ModelToWorld_prev;

    RendererGL      renderer;
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
    RigidObj () : BaseObj(), castsShadows(false), forceNotStatic(false) {}
    RigidObj (GLfloat x, GLfloat y, GLfloat z) : BaseObj(x,y,z), castsShadows(false), forceNotStatic(false) {}
    RigidObj (GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ) : BaseObj(x,y,z, rotX,rotY,rotZ), castsShadows(false), forceNotStatic(false) {}
    /********* CONSTRUCTORS : END *********/

    /******** LIFETIME : BEGIN ********/
public:
    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL, bool physicalNotLocked = false, bool phantom = true);
    virtual void Finalize   ();
    virtual void Invalidate () {
        UpdatePointers();
        renderer.InvalidateGraphics(*xModelGr, modelInstanceGr);
        renderer.InvalidateGraphics(*xModelPh,modelInstancePh);
        smap.texId = 0;
    }
    /********* LIFETIME : END *********/

    /******** UPDATE : BEGIN ********/
public:
    virtual void PreUpdate(float deltaTime);
    virtual void Update(float deltaTime);
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
        xMatrix::Invert(MX_ModelToWorld, MX_WorldToModel);
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

    /******** SHADOWS : BEGIN ********/
public:
    void       CreateShadowMap(xLight *light)
    {
        xMatrix blocker;
        UpdatePointers();
        GetShadowProjectionMatrix(light, blocker, smap.receiverUVMatrix, Config::ShadowMapSize);
        renderer.CreateShadowMapTexture( *xModelPh, modelInstancePh, smap.texId,
                                          Config::ShadowMapSize, blocker);
    }
    xShadowMap GetShadowMap ()                { return smap; }
    void       RenderShadowMap (xShadowMap smap, const xFieldOfView &FOV)
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
    void InvalidateShadowRenderData()
    {
        InvalidateShadowRenderData(modelInstanceGr);
        if (hModelGraphics != hModelPhysical)
            InvalidateShadowRenderData(modelInstancePh);
    }
protected:
    void GetShadowProjectionMatrix (xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width);
    /********* SHADOWS : END *********/

public:
    xModel *GetModelPh() { UpdatePointers(); return xModelPh; }
    xModel *GetModelGr() { UpdatePointers(); return xModelGr; }

    void CopySpineToPhysical()
    {
        CopySpine(xModelGr->spine, xModelPh->spine);
    }
    void CopySpineToGraphics()
    {
        CopySpine(xModelPh->spine, xModelGr->spine);
    }

    virtual void VerticesChanged(bool free);
    virtual void CalculateSkeleton();

protected:
    void RenderObject(bool transparent, const xFieldOfView &FOV)
    {
        UpdatePointers();
        renderer.RenderModel(*xModelGr, modelInstanceGr, transparent, FOV);
    }

protected:
    HModel     hModelGraphics;
    HModel     hModelPhysical;
    xModel   * xModelGr;
    xModel   * xModelPh;

    void UpdatePointers()
    {
        xModelGr = g_ModelMgr.GetModel(hModelGraphics)->model;
        xModelPh = g_ModelMgr.GetModel(hModelPhysical)->model;
        modelInstancePh.location = modelInstanceGr.location = MX_ModelToWorld;
    }

    void CopySpine(const xSkeleton &src, xSkeleton &dst)
    {
        if (src.L_bones != dst.L_bones)
        {
            dst.Clear();
            dst = src.Clone();
        }
    }

    void InvalidateShadowRenderData(xModelInstance &instance)
    {
        if (instance.elementInstanceP)
        {
            xElementInstance *iter = instance.elementInstanceP;
            for (int i = instance.elementInstanceC; i; --i, ++iter)
                iter->InvalidateVertexData();
        }
    }

    void FreeInstanceData();
};

#endif
