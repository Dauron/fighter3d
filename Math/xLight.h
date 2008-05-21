#ifndef __incl_lib3dx_xLight_h
#define __incl_lib3dx_xLight_h

#include "xFieldOfView.h"

typedef enum
{
    xLight_POINT    = 1,
    xLight_SPOT     = 2,
    xLight_INFINITE = 3
} xLightType;

struct xLight
{
    char*       name;
    char*       prefix;            // Light prefix

    bool        turned_on;

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

    xFLOAT      radius;
    xVector3    boundingPoints[8];

    void update();

    bool isVisible(xFieldOfView *FOV) const
    {
        if (type == xLight_INFINITE || radius < 0) return true;
        return FOV->CheckBox(boundingPoints);
    }

    bool elementReceivesLight(const xVector3 &bsCenter, float bsRadius) const;
};

#endif
