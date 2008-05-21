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

    xVector3       centerOfTheMass;
    xVector3       transVelocity;
    xVector4       rotatVelocity;

    xMatrix        mLocationMatrixPrev;
    xVector3       collisionNorm;
    xVector3       collisionVelo;
    xVector3       collisionCent;
    xVector3       penetrationCorrection;
    float          gravityAccumulator; // slowly increase gravity, to avoid vibrations

    //xVector3       cp, com, cno, cro;
protected:
    bool           forceNotStatic;
    xRenderGL      renderer;
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
    virtual void Invalidate () { renderer.Invalidate(); smap.texId = 0; }
    /********* LIFETIME : END *********/

    xRender                       *GetRenderer()       { renderer.UpdatePointers(); return &renderer; }

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
    void CollisionInfo_Fill   (xRender *rend, xElement *elem, xCollisionHierarchyBoundsRoot *ci, bool firstTime);
    void CollisionInfo_Free   (xElement *elem, xCollisionHierarchyBoundsRoot *ci);
    void CollisionInfo_Render (xElement *elem, xCollisionHierarchyBoundsRoot *ci);
    /********* PHYSICS : END *********/

    /******** SHADOWS : BEGIN ********/
public:
    void       CreateShadowMap(xLight *light)
    {
        xMatrix blocker;
        GetShadowProjectionMatrix(light, blocker, smap.receiverUVMatrix, Config::ShadowMapSize);
        smap.texId = GetRenderer()->CreateShadowMapTexture(Config::ShadowMapSize, blocker);
    }
    xShadowMap GetShadowMap ()                { return smap; }
    void       RenderShadowMap (xShadowMap smap, const xFieldOfView *FOV)
    {
        GetRenderer()->SetLocation(mLocationMatrix);
        GetRenderer()->RenderShadowMap(smap, FOV);
    }

    void       RenderShadowVolume(xLight &light, xFieldOfView *FOV)
    {
        GetRenderer()->SetLocation(mLocationMatrix);
        GetRenderer()->RenderShadowVolume(light, FOV);
    }

protected:
    void GetShadowProjectionMatrix (xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width);
    /********* SHADOWS : END *********/

    void RenderObject(bool transparent, const xFieldOfView *FOV);
};

#endif
