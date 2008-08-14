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

    virtual void ApplyDefaults();
    virtual void Initialize ();
    virtual void Finalize ();

    virtual void Initialize (const char *gr_filename, const char *ph_filename = NULL,
                             bool physicalNotLocked = false, bool phantom = true);
    void AddAnimation(const char *fileName, xDWORD T_start = 0, xDWORD T_end = 0);

    virtual void PreUpdate(float deltaTime);
    virtual void Update(float deltaTime);

    xActionSet actions;

    xQuaternion  *verletQuaternions;
    xFLOAT        verletWeight;
    xFLOAT        verletTime;
    xFLOAT        verletTimeMaxInv;
    xFLOAT        postHit;

    enum EControlType
    {
        Control_AI           = 0,
        Control_CaptureInput = 1,
        Control_NetworkInput = 2
    } ControlType;

    virtual void UpdateVerletSystem();

protected:
    virtual void CreateVerletSystem();
    virtual void DestroyVerletSystem();

};

#endif
