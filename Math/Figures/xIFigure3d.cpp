#include "xIFigure3d.h"
#include "xSphere.h"
#include "xCapsule.h"
#include "xBoxO.h"

using namespace Math::Figures;

void        xIFigure3d :: Save( FILE *file )
{
    fwrite(&Type, sizeof(Type), 1, file);
    fwrite(&P_center, sizeof(P_center), 1, file);
    saveInstance(file);
}

xIFigure3d *xIFigure3d :: Load( FILE *file )
{
    xFigure3dType type;
    xIFigure3d   *res;
    xPoint3       p_center;

    fread(&type,     sizeof(type),     1, file);
    fread(&p_center, sizeof(p_center), 1, file);

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

    return res;
}
