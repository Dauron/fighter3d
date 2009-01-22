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

void xglPerspective( GLdouble fovY, GLdouble aspect, GLdouble zNear )
{
    GLdouble fW, fH;
    fH = zNear * tan( PI_d * fovY / 360.0 );
    fW = fH * aspect;

    double left = -fW;
    double right = fW;
    double top = fH;
    double bottom = -fH;

    double x = (2.0*zNear) / (right-left);
    double y = (2.0*zNear) / (top-bottom);
    double a = (right+left) / (right-left);
    double b = (top+bottom) / (top-bottom);
    double c = -2.0*zNear;

    double mat[16];
#define M(row,col)  mat[row + col*4]
    M(0,0) = x;    M(0,1) = 0.0;  M(0,2) = a;      M(0,3) = 0.0;
    M(1,0) = 0.0;  M(1,1) = y;    M(1,2) = b;      M(1,3) = 0.0;
    M(2,0) = 0.0;  M(2,1) = 0.0;  M(2,2) = -1.0;   M(2,3) = c;
    M(3,0) = 0.0;  M(3,1) = 0.0;  M(3,2) = -1.0;   M(3,3) = 0.0;
#undef M
    glLoadMatrixd(mat);
}

void LightSet_GL(xLight &light, bool t_Ambient, bool t_Diffuse, bool t_Specular, xBYTE light_id)
{
    float light_off[4] = { 0.f, 0.f, 0.f, 0.f };
    // turn off ambient lighting
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_off);

    xVector4 position; position.init(light.position, light.type == xLight_INFINITE ? 0.f : 1.f);
    glLightfv(GL_LIGHT0+light_id, GL_POSITION, position.xyzw);

    glLightfv(GL_LIGHT0+light_id, GL_AMBIENT, t_Ambient ? light.ambient.col : light_off);
    glLightfv(GL_LIGHT0+light_id, GL_DIFFUSE,  t_Diffuse ? light.diffuse.col : light_off); // direct light
    glLightfv(GL_LIGHT0+light_id, GL_SPECULAR, t_Specular ? light.diffuse.col : light_off); // light on mirrors/metal

    if (light.type != xLight_INFINITE)
    {
        // rozpraszanie siê œwiat³a
        glLightf(GL_LIGHT0+light_id, GL_CONSTANT_ATTENUATION,  light.attenuationConst);
        glLightf(GL_LIGHT0+light_id, GL_LINEAR_ATTENUATION,    light.attenuationLinear);
        glLightf(GL_LIGHT0+light_id, GL_QUADRATIC_ATTENUATION, light.attenuationSquare);

        if (light.type == xLight_SPOT)
        {
            glLightfv(GL_LIGHT0+light_id, GL_SPOT_DIRECTION, light.spotDirection.xyz);
            glLightf(GL_LIGHT0+light_id,  GL_SPOT_CUTOFF,    light.spotCutOff);
            glLightf(GL_LIGHT0+light_id,  GL_SPOT_EXPONENT,  light.spotAttenuation);
        }
        else
            glLightf(GL_LIGHT0+light_id, GL_SPOT_CUTOFF, 180.0f);
    }

    glEnable(GL_LIGHT0+light_id);

    //Shader::SetLightType(light.type, t_Ambient, t_Diffuse, t_Specular);
}
