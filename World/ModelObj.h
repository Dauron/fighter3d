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
    ModelObj () : Object3D() {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z)
      : Object3D(x,y,z) {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : Object3D(x,y,z, rotX,rotY,rotZ) {}

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL);
    virtual void Finalize ();

    xRender    * GetRenderer()   {
        renderer.UpdatePointers();
        return &renderer;
    }
    CollisionInfo *GetCollisionInfo();
    xWORD          GetCollisionInfoC() { return collisionInfoC; }

    std::vector<CollisionWithModel> CollidedModels;

    virtual void PreUpdate();
    virtual void Update(float deltaTime);

    void   CollisionInfo_ReFill ();

protected:
    xRenderGL      renderer;

    CollisionInfo *collisionInfo;
    xWORD          collisionInfoC;
    std::vector<xDWORD> idx;

    void RenderObject();

    xDWORD CollisionInfo_Fill   (xRender *rend, xElement *elem, CollisionInfo *ci, bool firstTime);
    void   CollisionInfo_Free   (xElement *elem, CollisionInfo *ci);

    void   CollisionInfo_Render (xElement *elem, CollisionInfo *ci);
};

extern float time1b;
extern float time2b;
extern float time1;
extern float time2;

#endif
