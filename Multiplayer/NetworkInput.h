#ifndef __incl_MotionCapture_NetworkInput_h
#define __incl_MotionCapture_NetworkInput_h

#include "../Utils/Singleton.h"
#include "../Models/lib3dx/xBone.h"

#define g_NetworkInput NetworkInput::GetSingleton()

class NetworkInput : public Singleton<NetworkInput>
{
private:
    xWORD  boneC;
    xBone *spineP;

public:
    bool Initialize(xBone *spineP)
    {
        this->spineP = spineP;
        this->boneC  = (spineP) ? spineP->CountAllKids()+1 : 0;
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

    NetworkInput() {}
   ~NetworkInput() {}

private:
     // disable copy constructor & assignment operator
    NetworkInput(const CaptureInput&) {}
    NetworkInput& operator=(const CaptureInput&) { return *this; }
};

#endif
