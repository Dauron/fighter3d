#ifndef __incl_Object3D_h
#define __incl_Object3D_h

#include "../App Framework/System.h"
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

        void Render(bool transparent, const xFieldOfView &FOV)
        {
            glPushMatrix();
            glMultMatrixf(&mLocationMatrix.x0);
            RenderObject(transparent, FOV);
            glPopMatrix();
        }

        Object3D ();
        Object3D (xFLOAT x, xFLOAT y, xFLOAT z);
        Object3D (xFLOAT x, xFLOAT y, xFLOAT z,
                  xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ);
};

#endif
