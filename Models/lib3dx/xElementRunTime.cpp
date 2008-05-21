#include "xElementRunTime.h"

xShadowData & xElementInstance :: GetShadowData(xLight &light, bool zPassOnly)
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        if (iter->lightId == light.id)
        {
            if (light.modified || (iter->zPassOnly && !zPassOnly))
                iter->FreeData();
            iter->zPassOnly = zPassOnly;
            return *iter;
        }

    xShadowData newData;
    memset(&newData, 0, sizeof(xShadowData));
    newData.lightId = light.id;
    newData.zPassOnly = zPassOnly;
    gpuShadows.push_back(newData);
    return *gpuShadows.rbegin();
}

void xElementInstance :: Clear()
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        iter->FreeData();
    gpuShadows.clear();

    if (verticesP) { delete[] verticesP; verticesP = NULL; }
    if (normalsP)  { delete[] normalsP;  normalsP  = NULL; }
}

void xElementInstance :: InvalidateVertexData()
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        iter->InvalidateData();

    //if (verticesP) { delete[] verticesP; verticesP = NULL; }
    //if (normalsP)  { delete[] normalsP;  normalsP  = NULL; }
}