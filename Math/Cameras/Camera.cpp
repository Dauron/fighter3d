#include "Camera.h"

void Camera::SetCamera (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                        xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                        xFLOAT upx, xFLOAT upy, xFLOAT upz)
{
    eye.init   (eyex, eyey, eyez);
    center.init(centerx, centery, centerz);
    up.init    (upx, upy, upz);
}

// Based on MESA
void Camera::LookAtMatrix(xMatrix &MX_ModelToView)
{
    /* Forward = center - eye */
    xVector3 forward = (center - eye).normalize();
    /* Side = forward x up */
    xVector3 &side = xVector3::CrossProduct(forward, up).normalize();
    /* Recompute up as: up = side x forward */
    xVector3 up2 = xVector3::CrossProduct(side, forward).normalize();
    /* Fill matrix */
    MX_ModelToView.x0 = side.x;     MX_ModelToView.x1 = side.y;     MX_ModelToView.x2 = side.z;
    MX_ModelToView.y0 = up2.x;      MX_ModelToView.y1 = up2.y;      MX_ModelToView.y2 = up2.z;
    MX_ModelToView.z0 = -forward.x; MX_ModelToView.z1 = -forward.y; MX_ModelToView.z2 = -forward.z;
    MX_ModelToView.w0 = MX_ModelToView.w1 = MX_ModelToView.w2 = 0.f;
    MX_ModelToView.row3.init(0.f,0.f,0.f,1.f);
    MX_ModelToView.preTranslateT(-eye);
}
