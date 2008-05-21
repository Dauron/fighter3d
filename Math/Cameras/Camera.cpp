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
void Camera::LookAtMatrix(xMatrix &viewTransform)
{
    /* Forward = center - eye */
    xVector3 forward = (center - eye).normalize();
    /* Side = forward x up */
    xVector3 &side = xVector3::CrossProduct(forward, up).normalize();
    /* Recompute up as: up = side x forward */
    xVector3 up2 = xVector3::CrossProduct(side, forward).normalize();
    /* Fill matrix */
    viewTransform.x0 = side.x;     viewTransform.x1 = side.y;     viewTransform.x2 = side.z;
    viewTransform.y0 = up2.x;      viewTransform.y1 = up2.y;      viewTransform.y2 = up2.z;
    viewTransform.z0 = -forward.x; viewTransform.z1 = -forward.y; viewTransform.z2 = -forward.z;
    viewTransform.w0 = viewTransform.w1 = viewTransform.w2 = 0.f;
    viewTransform.row3.zeroQ();
    viewTransform.preTranslateT(-eye);
}
