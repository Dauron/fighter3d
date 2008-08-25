#ifndef __incl_OpenGL_Utils_h
#define __incl_OpenGL_Utils_h

#include "ogl.h"
#include "../../Math/Cameras/Camera.h"
#include "../../Math/Cameras/FieldOfView.h"

void xglPerspective ( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar );
void xglPerspective ( GLdouble fovY, GLdouble aspect, GLdouble zNear );

inline void xglPerspectiveInf ( const Math::Cameras::FieldOfView &FOV )
{
    xglPerspective( FOV.PerspAngle, FOV.Aspect, FOV.FrontClip/*, FOV.BackClip*/ );
}
inline void xglPerspective ( const Math::Cameras::FieldOfView &FOV )
{
    xglPerspective( FOV.PerspAngle, FOV.Aspect, FOV.FrontClip, FOV.BackClip );
}

inline void ViewportSet_GL(const Math::Cameras::Camera &camera)
{
    glViewport(camera.FOV.ViewportLeft,  camera.FOV.ViewportTop,
               camera.FOV.ViewportWidth, camera.FOV.ViewportHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&camera.FOV.MX_Projection_Get().x0);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&camera.MX_WorldToView_Get().x0);
}

#endif
