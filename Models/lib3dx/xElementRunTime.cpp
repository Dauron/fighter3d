#include "xElementRunTime.h"

xShadowData & xElementInstance :: GetShadowData(xLight &light, xShadowData::xShadowDataLevel zLevel)
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        if (iter->ID_light == light.id)
        {
            if (light.modified || iter->zDataLevel < zLevel)
                iter->FreeData();
            iter->zDataLevel = zLevel;
            return *iter;
        }

    xShadowData newData;
    memset(&newData, 0, sizeof(xShadowData));
    newData.ID_light = light.id;
    newData.zDataLevel = zLevel;
    gpuShadows.push_back(newData);
    return *gpuShadows.rbegin();
}

void xElementInstance :: Zero()
{
    mode = xGPURender::NONE;

    memset (&gpuMain, 0, sizeof(xGPUPointers));
    gpuShadows.clear();

    bBox.P_center.zero();
    bBox.S_top = bBox.S_side = bBox.S_front = 0.f;
    bSphere.P_center.zero();
    bSphere.S_radius = 0.f;

    bBox_T    = NULL;
    bSphere_T = NULL;

    I_vertices = 0;
    L_vertices = NULL;
    L_normals  = NULL;
}

void xElementInstance :: Clear()
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        iter->FreeData();
    gpuShadows.clear();

    if (L_vertices) { delete[] L_vertices; L_vertices = NULL; }
    if (L_normals)  { delete[] L_normals;  L_normals  = NULL; }

    if (bBox_T)    { delete bBox_T;    bBox_T = NULL; }
    if (bSphere_T) { delete bSphere_T; bSphere_T = NULL; }
}

void xElementInstance :: InvalidateVertexData()
{
    xShadowDataVector::iterator iter = gpuShadows.begin(), iterE = gpuShadows.end();
    for (; iter != iterE; ++iter)
        iter->InvalidateData();

    //if (L_vertices) { delete[] L_vertices; L_vertices = NULL; }
    //if (L_normals)  { delete[] L_normals;  L_normals  = NULL; }
}
