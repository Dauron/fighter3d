#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "ModelObj.h"
#include "../Models/lib3dx/xAction.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Multiplayer/NetworkInput.h"
#include "../Math/xVerletConstraint.h"

class SkeletizedObj : public ModelObj
{
public:

    SkeletizedObj () : ModelObj(), verletQuaternions(NULL), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z)
      : ModelObj(x,y,z), verletQuaternions(NULL), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z,
        GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : ModelObj(x,y,z, rotX,rotY,rotZ), verletQuaternions(NULL), ControlType(Control_AI) {}

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL,
                             bool physicalNotLocked = false, bool phantom = true);
    virtual void Finalize ();

    void AddAnimation(const char *fileName, xDWORD startTime = 0, xDWORD endTime = 0);

    virtual void PreUpdate(float deltaTime);
    virtual void Update(float deltaTime);

    xActionSet actions;

    xVector4 *verletQuaternions;
    xFLOAT        verletWeight;

    enum EControlType
    {
        Control_AI           = 0,
        Control_CaptureInput = 1,
        Control_NetworkInput = 2
    } ControlType;

protected:
    virtual void CreateVerletSystem();
    virtual void DestroyVerletSystem();
    virtual void UpdateVerletSystem();
};

#endif
