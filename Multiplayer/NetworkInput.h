#ifndef __incl_MotionCapture_NetworkInput_h
#define __incl_MotionCapture_NetworkInput_h

#include "../Utils/Singleton.h"
#include "../Models/lib3dx/xSkeleton.h"

#define g_NetworkInput NetworkInput::GetSingleton()

class NetworkInput : public Singleton<NetworkInput>
{
private:
    const xSkeleton *spine;

public:
    bool Initialize(const xSkeleton &spine)
    {
        this->spine = &spine;
        return true;
    }
    void Finalize()
    {}

    xVector4 * GetTransformations()
    {
        xVector4 *QT_bones = new xVector4[spine->I_bones];

        // kwaternion QT_bones[0] nie opisuje obrotu, a przesuni�cie ca�ego modelu, pozosta�e to obroty w formacie
        // x = axis.x * sin(alpha/2)
        // y = axis.y * sin(alpha/2)
        // z = axis.z * sin(alpha/2)
        // w = cos(alpha/2)
        // o� Z wskazuje do g�ry

        for (int i=0; i < spine->I_bones; ++i)
            QT_bones[i].zeroQ(); // no rotation

        return QT_bones;
    }

    NetworkInput() {}
   ~NetworkInput() {}

private:
     // disable copy constructor & assignment operator
    NetworkInput(const CaptureInput&) {}
    NetworkInput& operator=(const CaptureInput&) { return *this; }
};

#endif
