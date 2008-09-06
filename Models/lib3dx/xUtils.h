#ifndef __incl_xUtils_h
#define __incl_xUtils_h

#include "xModel.h"
#include "../../Math/Figures/xBoxA.h"

using namespace Math::Figures;

void         xModel_SkinElementInstance(const xModel &model, xModelInstance &instance);
xVector3     xModel_GetBounds(const xModel &model, xModelInstance &instance);

#endif
