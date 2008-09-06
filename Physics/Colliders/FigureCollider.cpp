#include "FigureCollider.h"


#include "../../Math/Figures/xSegment.h"
#include "../../Math/Figures/xSphere.h"
#include "../../Math/Figures/xCapsule.h"
#include "../../Math/Figures/xBoxO.h"
#include "../../Math/Figures/xCylinder.h"
#include "../../Math/Figures/xTriangle.h"
#include "../../Math/Figures/xMesh.h"

using namespace ::Math::Figures;
using namespace ::Physics::Colliders;

#include "AxisSpans.h"

//#include "SegmentSphere.h"
#include "SegmentCapsule.h"
#include "SegmentBoxO.h"
//#include "SegmentCylinder.h"
//#include "SegmentTriangle.h"
//#include "SegmentMesh.h"

#include "SphereSphere.h"
#include "SphereCapsule.h"
#include "SphereBoxO.h"
//#include "SphereCylinder.h"
#include "SphereTriangle.h"
#include "SphereMesh.h"
#include "CapsuleCapsule.h"
#include "CapsuleBoxO.h"
//#include "CapsuleCylinder.h"
#include "CapsuleTriangle.h"
#include "CapsuleMesh.h"
#include "BoxOBoxO.h"
//#include "BoxOCylinder.h"
#include "BoxOTriangle.h"
#include "BoxOMesh.h"
//#include "CylinderCylinder.h"
//#include "CylinderTriangle.h"
//#include "CylinderMesh.h"
#include "TriangleTriangle.h"
#include "TriangleMesh.h"
#include "MeshMesh.h"

////////////////////////////// FigureCollider :: Test

bool   FigureCollider :: Test (const xIFigure3d *figure1, const xIFigure3d *figure2)
{
    // Sphere
    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Sphere)
        return TestSphereSphere ( *((xSphere*) figure1), *((xSphere*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Capsule)
        return TestSphereCapsule ( *((xSphere*) figure1), *((xCapsule*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::BoxOriented)
        return TestSphereBoxO ( *((xSphere*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Mesh)
        return TestSphereMesh( *((xSphere*) figure1), *((xMesh*) figure2) );

    // Capsule
    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Sphere)
        return TestSphereCapsule ( *((xSphere*) figure2), *((xCapsule*) figure1) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Capsule)
        return TestCapsuleCapsule ( *((xCapsule*) figure1), *((xCapsule*) figure2) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::BoxOriented)
        return TestCapsuleBoxO ( *((xCapsule*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Mesh)
        return TestCapsuleMesh ( *((xCapsule*) figure1), *((xMesh*) figure2) );

    // Box Oriented
    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Sphere)
        return TestSphereBoxO ( *((xSphere*) figure2), *((xBoxO*) figure1) );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Capsule)
        return TestCapsuleBoxO ( *((xCapsule*) figure2), *((xBoxO*) figure1) );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::BoxOriented)
        return TestBoxOBoxO ( *((xBoxO*) figure1), *((xBoxO*) figure2) );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Mesh)
        return TestBoxOMesh ( *((xBoxO*) figure1), *((xMesh*) figure2) );

    // Cylinder
    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Sphere)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Capsule)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::BoxOriented)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Mesh)
        return false;

    // Mesh
    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Sphere)
        return TestSphereMesh( *((xSphere*) figure2), *((xMesh*) figure1) );

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Capsule)
        return TestCapsuleMesh ( *((xCapsule*) figure2), *((xMesh*) figure1) );

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::BoxOriented)
        return TestBoxOMesh ( *((xBoxO*) figure2), *((xMesh*) figure1) );

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Mesh)
        return TestMeshMesh ( *((xMesh*) figure1), *((xMesh*) figure2) );

    return false;
}

////////////////////////////// FigureCollider :: Collide

inline xDWORD CSInvert (xDWORD I_lastCount, CollisionSet &cset)
{
    CollisionSet::CollisionVec::reverse_iterator
        CS_curr = cset.collisions.rbegin(),
        CS_last = cset.collisions.rend();
    for (; CS_curr != CS_last; ++CS_curr) CS_curr->invert();
    return I_lastCount;
}

xDWORD FigureCollider :: Collide (IPhysicalBody *body1,      IPhysicalBody *body2,
                                  const xIFigure3d *figure1, const xIFigure3d *figure2, CollisionSet &cset)
{
    // Sphere
    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Sphere)
        return CollideSphereSphere ( *((xSphere*) figure1), *((xSphere*) figure2), body1, body2, cset );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Capsule)
        return CollideSphereCapsule ( *((xSphere*) figure1), *((xCapsule*) figure2), body1, body2, cset );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::BoxOriented)
        return CollideSphereBoxO ( *((xSphere*) figure1), *((xBoxO*) figure2), body1, body2, cset );

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Sphere && figure2->Type == xIFigure3d::Mesh)
        return CollideSphereMesh ( *((xSphere*) figure1), *((xMesh*) figure2), body1, body2, cset );

    // Capsule
    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Sphere)
        return CSInvert (CollideSphereCapsule ( *((xSphere*) figure2), *((xCapsule*) figure1), body2, body1, cset ), cset);

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Capsule)
        return CollideCapsuleCapsule ( *((xCapsule*) figure1), *((xCapsule*) figure2), body1, body2, cset );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::BoxOriented)
        return CollideCapsuleBoxO ( *((xCapsule*) figure1), *((xBoxO*) figure2), body1, body2, cset );

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Capsule && figure2->Type == xIFigure3d::Mesh)
        return CollideCapsuleMesh ( *((xCapsule*) figure1), *((xMesh*) figure2), body1, body2, cset );

    // Box Oriented
    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Sphere)
        return CSInvert (CollideSphereBoxO ( *((xSphere*) figure2), *((xBoxO*) figure1), body2, body1, cset ), cset);

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Capsule)
        return CSInvert (CollideCapsuleBoxO ( *((xCapsule*) figure2), *((xBoxO*) figure1), body2, body1, cset ), cset);

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::BoxOriented)
        return CollideBoxOBoxO ( *((xBoxO*) figure1), *((xBoxO*) figure2), body1, body2, cset );

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::BoxOriented && figure2->Type == xIFigure3d::Mesh)
        return CollideBoxOMesh ( *((xBoxO*) figure1), *((xMesh*) figure2), body1, body2, cset );

    // Cylinder
    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Sphere)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Capsule)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::BoxOriented)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Cylinder && figure2->Type == xIFigure3d::Mesh)
        return false;

    // Mesh
    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Sphere)
        return CSInvert (CollideSphereMesh ( *((xSphere*) figure2), *((xMesh*) figure1), body2, body1, cset ), cset);

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Capsule)
        return CSInvert (CollideCapsuleMesh ( *((xCapsule*) figure2), *((xMesh*) figure1), body2, body1, cset ), cset);

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::BoxOriented)
        return CSInvert (CollideBoxOMesh ( *((xBoxO*) figure2), *((xMesh*) figure1), body2, body1, cset), cset);

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Cylinder)
        return false;

    if (figure1->Type == xIFigure3d::Mesh && figure2->Type == xIFigure3d::Mesh)
        return CollideMeshMesh ( *((xMesh*) figure1), *((xMesh*) figure2), body1, body2, cset );

    return false;
}
