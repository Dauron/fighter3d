#include "xRender.h"

//xDWORD* xRender :: texture     = NULL;
//xWORD   xRender :: shadowWidth = 0;

void xRender :: Initialize(bool isStatic, HModel hGrModel, HModel hPhModel)
{
    hModelGraphics = hGrModel;
    Model3dx *mdl = g_ModelMgr.GetModel(hGrModel);
    xModelGraphics = mdl->model;
    if (hPhModel != HModel())
    {
        hModelPhysical = hPhModel;
        xModelPhysical = g_ModelMgr.GetModel(hPhModel)->model;
    }
    else
    {
        hModelPhysical = hModelGraphics;
        xModelPhysical = xModelGraphics;
        mdl->IncReferences();
    }
    xModelToRender = xModelGraphics;
    spineP = xModelGraphics->spineP;
    bonesM = NULL;
    bonesQ = NULL;
    bonesC = 0;
    shadowTexId  = 0;
    instanceDataGrP = instanceDataPhP = instanceDataTRP = NULL;
    instanceDataGrC = instanceDataPhC = instanceDataTRC = 0;
    location.identity();

    VerticesChanged();
}
