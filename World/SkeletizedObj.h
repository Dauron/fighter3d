#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "RigidObj.h"
#include "../Models/lib3dx/xAction.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Multiplayer/NetworkInput.h"

class SkeletizedObj : public RigidObj
{
public:

    SkeletizedObj () : RigidObj(), verletQuaternions(NULL), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z)
      : RigidObj(x,y,z), verletQuaternions(NULL), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z,
        GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : RigidObj(x,y,z, rotX,rotY,rotZ), verletQuaternions(NULL), ControlType(Control_AI) {}

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL,
                             bool physicalNotLocked = false, bool phantom = true);
    virtual void Finalize ();

    void AddAnimation(const char *fileName, xDWORD startTime = 0, xDWORD endTime = 0);

    virtual void PreUpdate(float deltaTime);
    virtual void Update(float deltaTime);

    xActionSet actions;

    xVector4 *verletQuaternions;
    xFLOAT        verletWeight;
    xFLOAT        verletTime;
    xFLOAT        verletTimeMaxInv;

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
