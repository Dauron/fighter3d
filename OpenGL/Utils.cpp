#include "../AppFramework/System.h"
#include <GL/gl.h>

#include "Utils.h"

/*
    Based on NEHE
    Replaces gluPerspective. Sets the frustum to perspective mode.
    fovY    - Field of vision in degrees in the y direction
    aspect    - Aspect ratio of the viewport
    zNear    - The near clipping distance
    zFar    - The far clipping distance
*/
void xglPerspective( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    //    Half of the size of the x and y clipping planes.
    GLdouble fW, fH;
    //    Calculate the distance from 0 of the y clipping plane. Basically trig to calculate
    //    position of clipper at zNear.
    //    Note:    tan( double ) uses radians but OpenGL works in degrees so we convert
    //        degrees to radians by dividing by 360 then multiplying by pi.
    // fH = tan( (fovY / 2) / 180 * pi ) * zNear; // Same as:
    fH = zNear * tan( PI_d * fovY / 360.0 );
    //    Calculate the distance from 0 of the x clipping plane based on the aspect ratio.
    fW = fH * aspect;
    //    Finally call glFrustum, this is all gluPerspective does anyway!
    //    This is why we calculate half the distance between the clipping planes - glFrustum
    //    takes an offset from zero for each clipping planes distance. (Saves 2 divides)
    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}

// Based on MESA
void xglLookAt(const xVector3 &eye, const xVector3 &center, const xVector3 &up)
{
    xMatrix m;
    /* Forward = center - eye */
    xVector3 forward = (center - eye).normalize();
    /* Side = forward x up */
    xVector3 &side = xVector3::CrossProduct(forward, up).normalize();
    /* Recompute up as: up = side x forward */
    xVector3 up2 = xVector3::CrossProduct(side, forward).normalize();
    /* Fill matrix */
    m.x0 = side.x;     m.x1 = side.y;     m.x2 = side.z;
    m.y0 = up2.x;      m.y1 = up2.y;      m.y2 = up2.z;
    m.z0 = -forward.x; m.z1 = -forward.y; m.z2 = -forward.z;
    m.x3 = m.y3 = m.z3 = m.w0 = m.w1 = m.w2 = 0.f;
    m.w3 = 1.f;
    glLoadMatrixf(&m.x0);
    //glMultMatrixf(&m.x0);
    glTranslatef(-eye.x, -eye.y, -eye.z);
}

void Camera_Aim_GL(Camera &camera)
{
    //glLoadIdentity();
    xglLookAt(camera.eye, camera.center, camera.up);
}
