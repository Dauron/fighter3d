#ifndef __incl_Math_xLight_h
#define __incl_Math_xLight_h

#include "Cameras/FieldOfView.h"
#include <vector>

typedef enum
{
    xLight_NONE     = 0,
    xLight_GLOBAL   = 1,
    xLight_INFINITE = 2,
    xLight_POINT    = 3,
    xLight_SPOT     = 4
} xLightType;

struct xLight
{
    xBYTE       id;
    char*       name;
    char*       prefix;            // Light prefix

    bool        turned_on;

    xLightType  type;
    xVector3    position;
    xColor      color;
    xColor      ambient;
    xColor      diffuse;
    xFLOAT      softness;

    xVector3    spotDirection;
    xFLOAT      spotCutOff;
    xFLOAT      spotAttenuation; // spot attenuation 0-no attenuation 128-max

    xFLOAT      attenuationConst;
    xFLOAT      attenuationLinear;
    xFLOAT      attenuationSquare;

    bool        modified;

    xFLOAT      radius;
    xVector3    boundingPoints[8];

    void create() { id = ++counter; modified = true; }
    void update();

    bool isVisible(const Math::Cameras::FieldOfView &FOV) const
    {
        if (type == xLight_INFINITE || radius < 0) return true;
        return FOV.CheckBox(boundingPoints);
    }

    bool elementReceivesLight(const xVector3 &bsCenter, float bsRadius) const;
    bool elementReceivesLight(const Math::Figures::xSphere &sphere) const
    {
        return elementReceivesLight(sphere.P_center, sphere.S_radius);
    }
    bool elementReceivesDiffuseLight(const Math::Cameras::FieldOfView &FOV,
                                     xVector3 boundingPoints[8]) const;

private:
    static xBYTE counter;
};

typedef std::vector<xLight> Vec_xLight;

#endif
