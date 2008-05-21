#ifndef __incl_ModelObj_h
#define __incl_ModelObj_h

#include "Object3D.h"
#include "../Math/xLight.h"
#include "../Models/ModelMgr.h"
#include "../Models/lib3dx/xBone.h"
#include "../Models/lib3dx/xRenderGL.h"
#include "../Physics/CollisionInfo.h"
#include <algorithm>
#include "../OpenGL/GLShader.h"
#include "../OpenGL/Textures/TextureMgr.h"

class ModelObj : public Object3D
{
public:
    bool           castsShadows;
    bool           phantom;    // no collisions
    bool           locked;     // may not be moved?
    bool           physical;   // affected by gravity?
    float          mass;       // weight of an object
    float          resilience; // how much energy will the object retain during collisions

    xVector3       transVelocity;
    xVector4       rotatVelocity;

    xMatrix        mLocationMatrixPrev;
    xVector3       collisionNorm;
    xVector3       collisionVelo;
    xVector3       collisionCent;
    xVector3       penetrationCorrection;
    float          gravityAccumulator; // slowly increase gravity, to avoid vibrations

    xRenderGL      renderer;
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
    ModelObj () : Object3D(), castsShadows(false), forceNotStatic(false) {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z) : Object3D(x,y,z), castsShadows(false), forceNotStatic(false) {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ) : Object3D(x,y,z, rotX,rotY,rotZ), castsShadows(false), forceNotStatic(false) {}
    /********* CONSTRUCTORS : END *********/

    /******** LIFETIME : BEGIN ********/
public:
    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL, bool physicalNotLocked = false, bool phantom = true);
    virtual void Finalize   ();
    virtual void Invalidate () {
        renderer.InvalidateInstanceGraphics(modelInstanceGr);
        renderer.InvalidateInstanceGraphics(modelInstancePh);
        smap.texId = 0;
    }
    /********* LIFETIME : END *********/

    /******** UPDATE : BEGIN ********/
public:
    virtual void PreUpdate();
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
    void       RenderShadowVolume(xLight &light, xFieldOfView &FOV)
    {
        UpdatePointers();
        renderer.RenderShadowVolume(*xModelGr, modelInstanceGr, light, FOV);
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
    void RenderObject(bool transparent, const xFieldOfView &FOV);

protected:
    HModel     hModelGraphics;
    HModel     hModelPhysical;
    xModel   * xModelGr;
    xModel   * xModelPh;

    void UpdatePointers()
    {
        xModelGr = g_ModelMgr.GetModel(hModelGraphics)->model;
        xModelPh = g_ModelMgr.GetModel(hModelPhysical)->model;
        modelInstancePh.location = modelInstanceGr.location = mLocationMatrix;
    }

    void CopySpine(const xSkeleton &src, xSkeleton &dst)
    {
        if (src.boneP != dst.boneP)
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
