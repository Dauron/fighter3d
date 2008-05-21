#ifndef __incl_xUtils_h
#define __incl_xUtils_h

#include "xModel.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

struct xBox
{
    xVector3 min;
    xVector3 max;

    bool Contains(const xVector3 &vert) const
    {
        return vert >= min && vert <= max;
    }
};

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

xVector4 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, bool fromRenderData = true);
xVector4 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, xMatrix transformation,
                                       xVector4 *&dst, bool fromRenderData = true);
xSkinnedData xElement_GetSkinnedElement(const xElement *elem, const xMatrix *bones);

xVector3   xCenterOfTheMass           (const xElement *elem, const xMatrix *bones);
xVector3   xCenterOfTheModelMass      (const xFile    *file, const xMatrix *bones);
xVector3   xCenterOfTheElement        (const xElement *elem, const xMatrix *bones);
xVector3   xCenterOfTheModel          (const xFile    *file, const xMatrix *bones);

xVector3   xCenterOfTheMass           (const xVector4* vertices, xDWORD count, bool scale = true);
xBox       xBoundingBox               (const xVector4* vertices, xDWORD count);

void xElement_GetCollisionHierarchy        (const xFile *file, xElement *elem);
void xElement_FreeCollisionHierarchy       (xCollisionData *pcData);
void xElement_FreeCollisionHierarchyBounds (xCollisionData *pcData, xCollisionHierarchyBounds *hierarchyBP);
void xElement_CalcCollisionHierarchyBox    (const xVector4* vertices,
                                            xCollisionData *pcData, xCollisionHierarchyBounds *pBound);

#endif
