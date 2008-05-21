#ifndef __incl_xUtils_h
#define __incl_xUtils_h

#include "xModel.h"

struct xSkinnedData
{
    xVector3 *verticesP;
    xVector3 *normalsP;
};

struct xCollisionHierarchyBounds
{
    xBox           bounding;
    xVector3       center;
    xFLOAT         radius;
    bool           sorted;

    xCollisionHierarchyBounds *kids;
};

struct xCollisionHierarchyBoundsRoot : public xCollisionHierarchyBounds
{
    xVector4                  * verticesP;
    xDWORD                      verticesC;
};

xSkinnedData xElement_GetSkinnedElement(const xElement *elem, const xMatrix *bones);
void         xModel_SkinElementInstance(const xModel *model, xModelInstance &instance);
xVector3     xModel_GetBounds(xModelInstance &instance);

void xElement_FreeCollisionHierarchyBounds (xCollisionData *pcData, xCollisionHierarchyBounds *hierarchyBP);
void xElement_CalcCollisionHierarchyBox    (const xVector4* vertices,
                                            xCollisionData *pcData, xCollisionHierarchyBounds *pBound);

#endif
