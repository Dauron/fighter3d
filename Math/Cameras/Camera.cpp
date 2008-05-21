#include "Camera.h"

void Camera::SetCamera (xFLOAT eyex, xFLOAT eyey, xFLOAT eyez,
                        xFLOAT centerx, xFLOAT centery, xFLOAT centerz,
                        xFLOAT upx, xFLOAT upy, xFLOAT upz)
{
    eye.init   (eyex, eyey, eyez);
    center.init(centerx, centery, centerz);
    up.init    (upx, upy, upz);
}
