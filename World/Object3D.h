#ifndef __incl_Object3D_h
#define __incl_Object3D_h

#include "../AppFramework/System.h"
#include <GL/gl.h>
#include "../Math/xMath.h"
#include "../Math/xFieldOfView.h"

class Object3D
{
    protected:
        virtual void RenderObject(bool transparent, const xFieldOfView &FOV) = 0;

    public:
        xMatrix mLocationMatrix;

        virtual ~Object3D() {}

        void Translate(xFLOAT x, xFLOAT y, xFLOAT z);
        void Rotate   (xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ);

        virtual void LocationChanged() {}

        void Render(bool transparent, const xFieldOfView &FOV)
        {
            RenderObject(transparent, FOV);
        }

        Object3D ();
        Object3D (xFLOAT x, xFLOAT y, xFLOAT z);
        Object3D (xFLOAT x, xFLOAT y, xFLOAT z,
                  xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ);
};

#endif
