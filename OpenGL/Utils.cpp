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
