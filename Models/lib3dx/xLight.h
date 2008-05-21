#ifndef __incl_lib3dx_xLight_h
#define __incl_lib3dx_xLight_h

#include "xModel.h"

typedef enum
{
    xLight_POINT    = 1,
    xLight_SPOT     = 2,
    xLight_INFINITE = 3,
    xLight_SUN      = 4
} xLightType;

struct xLight
{
    char*       name;
    char*       prefix;            // Light prefix

    xLightType  type;
    xVector3    position;
    xColor      color;

    xVector3    spotDirection;
    xFLOAT      spotCutOff;
    xFLOAT      spotAttenuation; // spot attenuation 0-no attenuation 128-max

    xFLOAT      attenuationConst;
    xFLOAT      attenuationLinear;
    xFLOAT      attenuationSquare;

    bool        modified;
};

#endif
