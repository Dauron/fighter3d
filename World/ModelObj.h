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
    xVector3       transVelocityPrev;
    xVector4       rotatVelocity;
    xVector4       rotatVelocityPrev;

public:
    ModelObj () : Object3D() {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z)
      : Object3D(x,y,z) {}
    ModelObj (GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : Object3D(x,y,z, rotX,rotY,rotZ) {}

    virtual void Initialize (const char *filename);
    virtual void Finalize ();

    const char * GetName() const { return g_ModelMgr.GetName(modelHandle); }
    Model3dx   * GetModel()      { return g_ModelMgr.GetModel(modelHandle); }
    xRender    * GetRenderer()   {
        renderer.xModel = GetModel()->renderer.xModel;
        return &renderer;
    }
    CollisionInfo *GetCollisionInfo();

    std::vector<CollisionWithModel> CollidedModels;

    virtual void Update(float deltaTime);

    void   CollisionInfo_ReFill ();

protected:
    HModel         modelHandle;
    xRenderGL      renderer;

    CollisionInfo *collisionInfo;
    xWORD          collisionInfoC;
    std::vector<xDWORD> idx;

    //xVector3       cp, com, cno;

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
