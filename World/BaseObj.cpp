#include "BaseObj.h"

BaseObj::BaseObj ()
{
    MX_ModelToWorld.identity();
}

BaseObj::BaseObj (xFLOAT x, xFLOAT y, xFLOAT z)
{
    MX_ModelToWorld.identity();
    Translate(x, y, z);
}

BaseObj::BaseObj (xFLOAT x, xFLOAT y, xFLOAT z,
                    xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ)
{
    MX_ModelToWorld.identity();
    Rotate(rotX, rotY, rotZ);
    Translate(x, y, z);
}

void BaseObj::Translate(xFLOAT x, xFLOAT y, xFLOAT z)
{
    xMatrix matrT;
    matrT.identity();
    matrT.x3 = x;
    matrT.y3 = y;
    matrT.z3 = z;
    MX_ModelToWorld *= matrT;
    LocationChanged();
}

void BaseObj::Rotate(xFLOAT rotX, xFLOAT rotY, xFLOAT rotZ)
{
    if (rotZ)
    {
        float rad = (float)DegToRad(rotZ);
        float sinz = sinf (rad);
        float cosz = cosf (rad);

        xMatrix matrZ;
        matrZ.identity();
        matrZ.y1 = matrZ.x0 = cosz;
        matrZ.x1 = sinz;
        matrZ.y0 = -sinz;

        MX_ModelToWorld *= matrZ;
    }

    if (rotY)
    {
        float rad = (float)DegToRad(rotY);
        float siny = sinf (rad);
        float cosy = cosf (rad);

        xMatrix matrY;
        matrY.identity();
        matrY.x0 = matrY.z2 = cosy;
        matrY.z0 = siny;
        matrY.x2 = -siny;

        MX_ModelToWorld *= matrY;
    }

    if (rotX)
    {
        float rad = (float)DegToRad(rotX);
        float sinx = sinf (rad);
        float cosx = cosf (rad);

        xMatrix matrX;
        matrX.identity();
        matrX.y1 = matrX.z2 = cosx;
        matrX.y2 = sinx;
        matrX.z1 = -sinx;

        MX_ModelToWorld *= matrX;
    }
    LocationChanged();
}
