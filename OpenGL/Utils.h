#ifndef __incl_OpenGL_Utils_h
#define __incl_OpenGL_Utils_h

#include "../Math/Cameras/Camera.h"

class GLUtils
{
public:
    static void Rectangle(float x1, float y1, float x2, float y2)
    {
        glBegin(GL_QUADS);
            glVertex2f(x1, y1);
            glVertex2f(x2, y1);
            glVertex2f(x2, y2);
            glVertex2f(x1, y2);
        glEnd();
    }
};

void xglPerspective ( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar );
void xglLookAt      ( const xVector3 &eye, const xVector3 &center, const xVector3 &up );

void Camera_Aim_GL(Camera &camera);

#endif
