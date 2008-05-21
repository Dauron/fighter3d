#ifndef __incl_xUtils_h
#define __incl_xUtils_h

#include "xModel.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

struct xSkinnedData
{
    xVector3 *verticesP;
    xVector3 *normalsP;
};

struct xSkinnedDataShd
{
    xVector4 *verticesP;
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
    xVector3                  * verticesP;
    xDWORD                      verticesC;
};

xVector3 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, bool fromRenderData = true);
xVector3 * xElement_GetSkinnedVertices(const xElement *elem, const xMatrix *bones, xMatrix transformation,
                                       xVector3 *&dst, bool fromRenderData = true);
xSkinnedData    xElement_GetSkinnedElement(const xElement *elem, const xMatrix *bones);
xSkinnedDataShd xElement_GetSkinnedElementForShadow(const xElement *elem, const xMatrix *bones);

xVector3   xCenterOfTheMass           (const xElement *elem, const xMatrix *bones);
xVector3   xCenterOfTheModelMass      (const xFile    *file, const xMatrix *bones);
xVector3   xCenterOfTheElement        (const xElement *elem, const xMatrix *bones);
xVector3   xCenterOfTheModel          (const xFile    *file, const xMatrix *bones);

xVector3 xCenterOfTheMass           (const xVector4* vertices, xDWORD count, bool scale = true);
void xFile_GetBounds(const xFile *model, const xMatrix *boneP, xElementInstance *instanceP);

void xElement_GetCollisionHierarchy        (const xFile *file, xElement *elem);
void xElement_FreeCollisionHierarchy       (xCollisionData *pcData);
void xElement_FreeCollisionHierarchyBounds (xCollisionData *pcData, xCollisionHierarchyBounds *hierarchyBP);
void xElement_CalcCollisionHierarchyBox    (const xVector3* vertices,
                                            xCollisionData *pcData, xCollisionHierarchyBounds *pBound);

#endif
