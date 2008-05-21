#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "ModelObj.h"
#include "../Models/lib3dx/xAction.h"

class SkeletizedObj : public ModelObj
{
public:
    SkeletizedObj () : ModelObj() {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z)
      : ModelObj(x,y,z) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z,
         GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : ModelObj(x,y,z, rotX,rotY,rotZ) {}

    virtual void Initialize (const char *filename)
    {
        ModelObj::Initialize(filename);
        resilience = 0.2f;
    }

    virtual void Finalize ()
    {
        ModelObj::Finalize();
        if (actions.actions.size())
        {
            std::vector<xAction>::iterator iterF = actions.actions.begin(), iterE = actions.actions.end();
            for (; iterF != iterE; ++iterF)
                g_AnimationMgr.DeleteAnimation(iterF->hAnimation);
            actions.actions.clear();
        }
    }

    void AddAnimation(const char *fileName, xDWORD startTime = 0, xDWORD endTime = 0)
    {
        actions.actions.resize(actions.actions.size()+1);
        actions.actions.rbegin()->hAnimation = g_AnimationMgr.GetAnimation(fileName);
        actions.actions.rbegin()->startTime = startTime;
        actions.actions.rbegin()->endTime = endTime;
    }

    virtual void Update(float deltaTime)
    {
        ModelObj::Update(deltaTime);
        CollidedModels.clear();

        xRender *renderer = GetRenderer();

        if (actions.actions.size())
        {
            xDWORD delta = (xDWORD)(deltaTime*1000);

            actions.Update(delta);
            xVector4 *bones = actions.GetTransformations();

            if (bones)
            {
                xAnimation::SaveToSkeleton(renderer->spineP, bones);
                delete[] bones;

                //CollisionInfo_ReFill();
            }
            renderer->CalculateSkeleton();
            CollisionInfo_ReFill();
            centerOfTheMass = xCenterOfTheModelMass(renderer->xModelPhysical, renderer->bonesM);

            if (actions.progress > 10000) actions.progress = 0;
        }
        else
            xSkeletonReset(renderer->spineP);
    }

    xActionSet actions;
};

#endif
