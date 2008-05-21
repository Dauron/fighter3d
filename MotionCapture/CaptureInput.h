#ifndef __incl_MotionCapture_CaptureInput_h
#define __incl_MotionCapture_CaptureInput_h

#include "../Utils/Singleton.h"
#include "../Models/lib3dx/xSkeleton.h"
#include "../Models/lib3dx/xModel.h"

#define g_CaptureInput CaptureInput::GetSingleton()

class CaptureInput : public Singleton<CaptureInput>
{
private:
    xWORD  boneC;
    xBone *spineP;

public:
    bool Initialize(xBone *spineP)
    {
        this->boneC  = xBoneChildCount(spineP)+1;
        this->spineP = spineP;
        return true;
    }
    void Finalize()
    {}

    xVector4 * GetTransformations()
    {
        xVector4 *trans = new xVector4[boneC];

        // kwaternion trans[0] nie opisuje obrotu, a przesuniêcie ca³ego modelu, pozosta³e to obroty w formacie
        // x = axis.x * sin(alpha/2)
        // y = axis.y * sin(alpha/2)
        // z = axis.z * sin(alpha/2)
        // w = cos(alpha/2)
        // oœ Z wskazuje do góry

        for (int i=0; i<boneC; ++i)
            trans[i].zeroQ(); // no rotation

        return trans;
    }

    CaptureInput() {}
   ~CaptureInput() {}

private:
     // disable copy constructor & assignment operator
    CaptureInput(const CaptureInput&) {}
    CaptureInput& operator=(const CaptureInput&) { return *this; }
};

#endif
