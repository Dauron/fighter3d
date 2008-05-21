#ifndef __incl_lib3dx_xLight_h
#define __incl_lib3dx_xLight_h

#include "xModel.h"

typedef enum
{
    xLight_POINT = 1,
    xLight_SPOT,
    xLight_INFINITE,
    xLight_SUN
} xLightType;

struct xLight
{
    char*       name;
    char*       prefix;            // Light prefix

    xLightType  type;
    xVector3    position;
    xColor      color;

    xFLOAT      constAttenuation;
    xFLOAT      linearAttenuation;
    xFLOAT      squareAttenuation;
};

#endif
