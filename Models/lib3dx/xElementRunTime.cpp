#include "xElementRunTime.h"

xShadowData & xElementInstance :: GetShadowData(xLight &light, xShadowData::xShadowDataLevel zLevel)
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        if (iter->lightId == light.id)
        {
            if (light.modified || iter->zDataLevel < zLevel)
                iter->FreeData();
            iter->zDataLevel = zLevel;
            return *iter;
        }

    xShadowData newData;
    memset(&newData, 0, sizeof(xShadowData));
    newData.lightId = light.id;
    newData.zDataLevel = zLevel;
    gpuShadows.push_back(newData);
    return *gpuShadows.rbegin();
}

void xElementInstance :: Zero()
{
    mode = xGPURender::NONE;

    memset (&gpuMain, 0, sizeof(xGPUPointers));
    gpuShadows.clear();

    memset (&bbBox, 0, sizeof(xBox));
    bsCenter.zero();
    bsRadius = 0;

    verticesC = 0;
    verticesP = NULL;
    normalsP  = NULL;
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
