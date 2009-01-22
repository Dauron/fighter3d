#include "xIFigure3d.h"
#include "xSphere.h"
#include "xCapsule.h"
#include "xBoxO.h"

using namespace Math::Figures;

void        xIFigure3d :: Save( FILE *file )
{
    SAFE_begin
    {
        SAFE_fread(Type,     1, file);
        SAFE_fread(P_center, 1, file);
        saveInstance(file);
        SAFE_return;
    }
    SAFE_catch;
        LOG(1, "Error writing xIFigure3d");
}

xIFigure3d *xIFigure3d :: Load( FILE *file )
{
    SAFE_begin
    {
        xFigure3dType type;
        xIFigure3d   *res;
        xPoint3       p_center;

        SAFE_fread(type,     1, file);
        SAFE_fread(p_center, 1, file);

        switch (type)
        {
            case Sphere:
                res = new xSphere();
                break;
            case Capsule:
                res = new xCapsule();
                break;
            case BoxOriented:
                res = new xBoxO();
                break;
            default:
                return NULL;
        }

        res->Type     = type;
        res->P_center = p_center;
        res->loadInstance(file);
        SAFE_return res;
    }
    SAFE_catch;
        LOG(1, "Error reading xIFigure3d");
        return NULL;
}
