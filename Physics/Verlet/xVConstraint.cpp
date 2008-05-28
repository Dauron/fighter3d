#include "xVConstraint.h"
#include "xVConstraintLengthEql.h"
#include "xVConstraintLengthMin.h"
#include "xVConstraintLengthMax.h"
#include "xVConstraintAngular.h"
#include "xVConstraintCollision.h"

xVConstraint * xVConstraint :: LoadType( FILE *file )
{
    xVConstraint_Type type;
    xVConstraint *res = NULL;
    fread(&type, sizeof(xVConstraint_Type), 1, file);
    switch (type)
    {
        case Constraint_LengthEql:
            res = (new xVConstraintLengthEql())->Load(file);
            break;
        case Constraint_LengthMin:
            res = (new xVConstraintLengthMin())->Load(file);
            break;
        case Constraint_LengthMax:
            res = (new xVConstraintLengthMax())->Load(file);
            break;
        case Constraint_Angular:
            res = (new xVConstraintAngular())->Load(file);
            break;
        case Constraint_Plane:
            res = (new xVConstraintCollision())->Load(file);
            break;
    }
    return res;
}
