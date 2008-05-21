#ifndef __incl_SkeletizedObj_h
#define __incl_SkeletizedObj_h

#include "ModelObj.h"
#include "../Models/lib3dx/xAction.h"
#include "../MotionCapture/CaptureInput.h"
#include "../Multiplayer/NetworkInput.h"

class SkeletizedObj : public ModelObj
{
public:

    SkeletizedObj () : ModelObj(), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z)
      : ModelObj(x,y,z), ControlType(Control_AI) {}
    SkeletizedObj (GLfloat x, GLfloat y, GLfloat z,
        GLfloat rotX, GLfloat rotY, GLfloat rotZ)
      : ModelObj(x,y,z, rotX,rotY,rotZ), ControlType(Control_AI) {}

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL,
                             bool physicalNotLocked = false, bool phantom = true);
    virtual void Finalize ();

    void AddAnimation(const char *fileName, xDWORD startTime = 0, xDWORD endTime = 0);

    virtual void Update(float deltaTime);

    xActionSet actions;

    typedef enum
    {
        Control_AI           = 0,
        Control_CaptureInput = 1,
        Control_NetworkInput = 2
    } EControlType;
    EControlType ControlType;
};

#endif
