#ifndef __incl_Object3D_h
#define __incl_Object3D_h

#include "../AppFramework/System.h"
#include <GL/gl.h>
#include "../Math/xMath.h"
#include "../Math/xFieldOfView.h"

class BaseObj
{
    protected:
        virtual void RenderObject(bool transparent, const xFieldOfView &FOV) = 0;

    public:
        xMatrix MX_ModelToWorld;

        virtual ~BaseObj() {}

        void Translate(xFLOAT x, xFLOAT y, xFLOAT z);
        void Rotate   (xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ);

        virtual void LocationChanged() {}

        void Render(bool transparent, const xFieldOfView &FOV)
        {
            RenderObject(transparent, FOV);
        }

        BaseObj ();
        BaseObj (xFLOAT x, xFLOAT y, xFLOAT z);
        BaseObj (xFLOAT x, xFLOAT y, xFLOAT z,
                  xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ);
};

#endif
