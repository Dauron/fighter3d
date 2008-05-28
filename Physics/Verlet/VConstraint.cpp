#include "VConstraint.h"
#include "VConstraintLengthEql.h"
#include "VConstraintLengthMin.h"
#include "VConstraintLengthMax.h"
#include "VConstraintAngular.h"
#include "VConstraintCollision.h"

VConstraint * VConstraint :: LoadType( FILE *file )
{
    VConstraint_Type type;
    VConstraint *res = NULL;
    fread(&type, sizeof(VConstraint_Type), 1, file);
    switch (type)
    {
        case Constraint_LengthEql:
            res = (new VConstraintLengthEql())->Load(file);
            break;
        case Constraint_LengthMin:
            res = (new VConstraintLengthMin())->Load(file);
            break;
        case Constraint_LengthMax:
            res = (new VConstraintLengthMax())->Load(file);
            break;
        case Constraint_Angular:
            res = (new VConstraintAngular())->Load(file);
            break;
        case Constraint_Plane:
            res = (new VConstraintCollision())->Load(file);
            break;
    }
    return res;
}
