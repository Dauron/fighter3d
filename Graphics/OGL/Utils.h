#ifndef __incl_OpenGL_Utils_h
#define __incl_OpenGL_Utils_h

#include "ogl.h"
#include "../../Math/Cameras/Camera.h"
#include "../../Math/xFieldOfView.h"

void xglPerspective ( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar );
void xglPerspective ( GLdouble fovY, GLdouble aspect, GLdouble zNear );

inline void xglPerspectiveInf ( const xFieldOfView &FOV )
{
    xglPerspective( FOV.Angle, FOV.Aspect, FOV.FrontClip/*, FOV.BackClip*/ );
}
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
