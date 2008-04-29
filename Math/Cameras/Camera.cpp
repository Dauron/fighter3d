#include "Camera.h"

void Camera::SetCamera (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                        xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                        xFLOAT upx, xFLOAT upy, xFLOAT upz)
{
    eye.Init   (eyex, eyey, eyez);
    center.Init(centerx, centery, centerz);
    up.Init    (upx, upy, upz);
}
