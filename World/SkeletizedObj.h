#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "ModelObj.h"
#include "../Models/lib3dx/xAction.h"
#include "../MotionCapture/CaptureInput.h"

class SkeletizedObj : public ModelObj
{
public:

    SkeletizedObj () : ModelObj(), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z)
      : ModelObj(x,y,z), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z,
        GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : ModelObj(x,y,z, rotX,rotY,rotZ), ControlType(Control_AI) {}

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL, bool physical = false, bool phantom = true)
    {
        forceNotStatic = true;
        ModelObj::Initialize(gr_filename, ph_filename);
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
        xVector4 *bones = NULL;

        if (actions.actions.size())
        {
            xDWORD delta = (xDWORD)(deltaTime*1000);

            actions.Update(delta);
            xVector4 *bones = actions.GetTransformations();

            if (actions.progress > 10000) actions.progress = 0;
        }
        if (ControlType == Control_CaptureInput)
        {
            xVector4 *bones2 = g_CaptureInput.GetTransformations();
            if (bones2)
                if (bones)
                {
                    xAnimation::Combine(bones2, bones, renderer->bonesC , bones);
                    delete[] bones2;
                }
                else
                    bones = bones2;
        }

        if (bones)
        {
            xAnimation::SaveToSkeleton(renderer->spineP, bones);
            delete[] bones;

            renderer->CalculateSkeleton();
            CollisionInfo_ReFill();
            centerOfTheMass = xCenterOfTheModelMass(renderer->xModelPhysical, renderer->bonesM);
        }
        else
            xSkeletonReset(renderer->spineP);
    }

    xActionSet actions;

    typedef enum
    {
        Control_AI           = 0,
        Control_CaptureInput = 1,
        Control_NetworkInput = 2
    } xControlType;
    xControlType ControlType;
};

#endif
