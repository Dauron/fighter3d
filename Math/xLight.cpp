#include "xLight.h"

xFLOAT staticCalculateIntensity ( xFLOAT distance, const xVector3& attenuation )
{
    return 1.0 / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);
}

xFLOAT staticCalculateRadius(xFLOAT testIncrement, const xVector3& attenuation)
{
    float testRadius = testIncrement;
    static const xFLOAT minIntensity = 10.f / 256.f;
    static const xFLOAT minIntensityI = 25.6f;

    // make sure the parameters are reasonable and that
    // the algorithm will terminate
    if ((attenuation.x < 0.f) ||
        (attenuation.y < 0.f) ||
        (attenuation.z < 0.f) ||
        ((attenuation.x < minIntensityI) && (attenuation.y == 0.f) && (attenuation.z == 0.f)))
        return -1.0;

    while (staticCalculateIntensity(testRadius, attenuation) > minIntensity)
        testRadius += testIncrement;

    return testRadius;
}

void xLight::update()
{
    if (modified)
    {
        if (type == xLight_INFINITE) { radius = -1.f; return; }
        radius = staticCalculateRadius(0.1f, xVector3::Create(attenuationConst, attenuationLinear, attenuationSquare));
        boundingPoints[0].init(position.x - radius, position.y - radius, position.z - radius);
        boundingPoints[1].init(position.x + radius, position.y + radius, position.z + radius);
        boundingPoints[2].init(position.x + radius, position.y - radius, position.z - radius);
        boundingPoints[3].init(position.x - radius, position.y + radius, position.z - radius);
        boundingPoints[4].init(position.x + radius, position.y + radius, position.z - radius);
        boundingPoints[5].init(position.x - radius, position.y - radius, position.z + radius);
        boundingPoints[6].init(position.x + radius, position.y - radius, position.z + radius);
        boundingPoints[7].init(position.x - radius, position.y + radius, position.z + radius);
    }
}