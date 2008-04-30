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

struct xCollisionHierarchyBounds
{
    xBox           bounding;

    xCollisionHierarchyBounds *kids;
};

xVector4 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, bool fromRenderData = true);
xVector4 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, xMatrix transformation,
                                       xVector4 *&dst, bool fromRenderData = true);

xVector3   xCenterOfTheMass           (const xElement *elem, const xMatrix *bones);
xVector3   xCenterOfTheElement        (const xElement *elem, const xMatrix *bones);
xVector3   xCenterOfTheModel          (const xFile *file, const xMatrix *bones);

xVector3   xCenterOfTheMass           (const xVector4* vertices, xDWORD count, bool scale = true);
xBox       xBoundingBox               (const xVector4* vertices, xDWORD count);

void xElement_GetCollisionHierarchy(const xFile *file, xElement *elem);
void xElement_FreeCollisionHierarchy(xCollisionHierarchy *hierarchyP, xWORD hierarchyC);
void xElement_FreeCollisionHierarchyBounds(xCollisionHierarchy *hierarchyP, xWORD hierarchyC,
                                           xCollisionHierarchyBounds *hierarchyBP);
xBox xElement_CalcCollisionHierarchyBox(const xVector4* vertices,
                                        xCollisionHierarchy *hierarchyP, xWORD hierarchyC,
                                        xCollisionHierarchyBounds *&bounds);

#endif
