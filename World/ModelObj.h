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

    virtual void Initialize (const char *filename)
    {
        assert(modelHandle.IsNull());
        modelHandle = g_ModelMgr.GetModel(filename);
        renderer.Initialize(NULL);
        collisionInfo = NULL;
        centerOfTheMass.x = centerOfTheMass.y = centerOfTheMass.z = 0.f;
        transVelocity.x = transVelocity.y = transVelocity.z = 0.f;
        rotatVelocity.x = rotatVelocity.y = rotatVelocity.z = 0.f; rotatVelocity.w = 1.f;
        transVelocityPrev = transVelocity;
        rotatVelocityPrev = rotatVelocity;
        mass       = 1.f;
        resilience = 0.5f;
        physical   = false;
        phantom    = true;
    }

    virtual void Finalize ()
    {
        if (collisionInfo)
        {
            CollisionInfo_Free(GetRenderer()->xModel->firstP, collisionInfo);
            delete[] collisionInfo;
            collisionInfo = NULL;
        }
        if (!modelHandle.IsNull())
        {
            assert(ModelMgr::GetSingletonPtr());
            //if (ModelMgr::GetSingletonPtr()) // not needed when the World is used
            g_ModelMgr.DeleteModel(modelHandle);
            modelHandle = HModel();
        }
        renderer.xModel = NULL; // do not free the model and its render data, it may be used by other ModelObj, Model3dx will free it, when needed
        renderer.Finalize();
    }

    const char * GetName() const { return g_ModelMgr.GetName(modelHandle); }
    Model3dx   * GetModel()      { return g_ModelMgr.GetModel(modelHandle); }
    xRender    * GetRenderer()   {
        renderer.xModel = GetModel()->renderer.xModel;
        return &renderer;
    }
    CollisionInfo *GetCollisionInfo();

    std::vector<CollisionWithModel> CollidedModels;

    virtual void Update(float deltaTime);

protected:
    HModel         modelHandle;
    xRenderGL      renderer;

    CollisionInfo *collisionInfo;
    xWORD          collisionInfoC;
    std::vector<xDWORD> idx;

    //xVector3       cp, com, cno;

    void RenderObject()
    {
        transVelocityPrev = transVelocity;
        rotatVelocityPrev = rotatVelocity;

        assert(!modelHandle.IsNull());
        GetRenderer()->RenderModel();
        if (!phantom)
            if (!CollidedModels.empty())
            {
                srand(100);
                //if (idx.empty())
                {
                    std::vector<CollisionWithModel>::iterator cmiter;
                    for (cmiter = CollidedModels.begin(); cmiter < CollidedModels.end(); ++cmiter)
                    {
                        std::vector<Collisions>::iterator iter;
                        std::vector<xDWORD>::iterator found;
                        xElement *prevElem = NULL;
                        for (iter = cmiter->collisions.begin(); iter < cmiter->collisions.end(); ++iter)
                        {
                            if (prevElem != iter->elem1)
                            {
                                if (prevElem)
                                    GetRenderer()->RenderFaces(prevElem->id, &idx);
                                prevElem = iter->elem1;
                                idx.clear();
                            }
                            found = std::find(idx.begin(), idx.end(), (xDWORD)iter->face1);
                            if (found == idx.end()) idx.push_back((xDWORD)iter->face1);
                        }
                        if (prevElem)
                            GetRenderer()->RenderFaces(prevElem->id, &idx);
                        idx.clear();
                    }
                }
            }
/*
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_COLOR_MATERIAL);
        g_TextureMgr.DisableTextures();
        glPointSize(5.f);
        glPopMatrix();
        glBegin(GL_LINES);
        {
            glColor3f(1.f, 1.f, 0.f);
            glVertex3fv(cp.xyz);
            glVertex3fv(com.xyz);
            glColor3f(1.f, 0.f, 0.f);
            glVertex3fv(cp.xyz);
            glVertex3fv(cno.xyz);
        }
        glEnd();
        glPushMatrix();
        glDisable(GL_COLOR_MATERIAL);
        glEnable(GL_DEPTH_TEST);
*/
    }

    void   CollisionInfo_ReFill ();
    xDWORD CollisionInfo_Fill   (xRender *rend, xElement *elem, CollisionInfo *ci, bool firstTime);
    void   CollisionInfo_Free   (xElement *elem, CollisionInfo *ci);

    void   CollisionInfo_Render (xElement *elem, CollisionInfo *ci);
};

#endif
