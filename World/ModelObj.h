#ifndef __incl_ModelObj_h
#define __incl_ModelObj_h

#include "Object3D.h"
#include "../Models/ModelMgr.h"
#include "../Models/lib3dx/xSkeleton.h"
#include "../Models/lib3dx/xRenderGL.h"
#include "../Physics/CollisionInfo.h"
#include <algorithm>
#include "../OpenGL/GLShader.h"
#include "../OpenGL/Textures/TextureMgr.h"

class ModelObj : public Object3D
{
public:
    bool           phantom;    // no collisions
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

public:
    ModelObj () : Object3D(), forceNotStatic(false) {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z)
      : Object3D(x,y,z), forceNotStatic(false) {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : Object3D(x,y,z, rotX,rotY,rotZ), forceNotStatic(false) {}

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL, bool physical = false, bool phantom = true);
    virtual void Finalize ();

    xRender    * GetRenderer()   {
        renderer.UpdatePointers();
        return &renderer;
    }
    xCollisionHierarchyBoundsRoot *GetCollisionInfo();
    xWORD                          GetCollisionInfoC() { return collisionInfoC; }

    std::vector<CollisionWithModel> CollidedModels;

    virtual void PreUpdate();
    virtual void Update(float deltaTime);

    void   CollisionInfo_ReFill ();

    void       CreateShadowMap(xLight *light)
    {
        xMatrix    blocker;
        GetShadowProjectionMatrix(light, blocker, smap.receiverUVMatrix, 256);
        smap.texId = GetRenderer()->CreateShadowMap(256, blocker);
    }
    xShadowMap GetShadowMap()
    {
        return smap;
    }

    void RenderShadow(xShadowMap smap)
    {
        GetRenderer()->RenderShadow(smap, mLocationMatrix);
    }

protected:
    bool           forceNotStatic;
    xRenderGL      renderer;
    xShadowMap smap;

    xCollisionHierarchyBoundsRoot *collisionInfo;
    xWORD                          collisionInfoC;
    std::vector<xDWORD> idx;

    void RenderObject(bool transparent);

    void CollisionInfo_Fill   (xRender *rend, xElement *elem, xCollisionHierarchyBoundsRoot *ci, bool firstTime);
    void CollisionInfo_Free   (xElement *elem, xCollisionHierarchyBoundsRoot *ci);
    void CollisionInfo_Render (xElement *elem, xCollisionHierarchyBoundsRoot *ci);

    void GetShadowProjectionMatrix (xLight* light, xMatrix &mtxBlockerToLight, xMatrix &mtxReceiverUVMatrix, xWORD width);
};

extern float time1b;
extern float time2b;
extern float time1;
extern float time2;

#endif
