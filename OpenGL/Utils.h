#ifndef __incl_OpenGL_Utils_h
#define __incl_OpenGL_Utils_h

#include "../Math/Cameras/Camera.h"
#include "../Math/xFieldOfView.h"

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

inline void xglPerspective ( const xFieldOfView &FOV )
{
    xglPerspective( FOV.Angle, FOV.Aspect, FOV.FrontClip, FOV.BackClip );
}

inline void Camera_Aim_GL(Camera &camera)
{
    xMatrix m;
    camera.LookAtMatrix(m);
    glLoadMatrixf(&m.x0);
}

#endif
