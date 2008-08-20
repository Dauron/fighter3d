#ifndef __incl_MotionCapture_NetworkInput_h
#define __incl_MotionCapture_NetworkInput_h

#include "../Utils/Singleton.h"
#include "../Models/lib3dx/xSkeleton.h"

#define g_NetworkInput NetworkInput::GetSingleton()

class NetworkInput : public Singleton<NetworkInput>
{
private:
    const xSkeleton *Spine;

public:
    bool Initialize(const xSkeleton &spine)
    {
        this->Spine = &spine;
        return true;
    }
    void Finalize()
    {}

    xQuaternion * GetTransformations()
    {
        xQuaternion *QT_bones = new xQuaternion[Spine->I_bones];

        // kwaternion QT_bones[0] nie opisuje obrotu, a przesuniêcie ca³ego modelu, pozosta³e to obroty w formacie
        // x = axis.x * sin(alpha/2)
        // y = axis.y * sin(alpha/2)
        // z = axis.z * sin(alpha/2)
        // w = cos(alpha/2)
        // oœ Z wskazuje do góry

        for (int i=0; i < Spine->I_bones; ++i)
            QT_bones[i].zeroQ(); // no rotation

        return QT_bones;
    }

    NetworkInput() {}
   ~NetworkInput() {}
};

#endif
